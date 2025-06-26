#include "mmem.h"

#include <string.h>

typedef unsigned int bitslot_t;
#define SLOT_BITS ((bitslot_t)sizeof( bitslot_t ) * 8)

static inline bitslot_t bitmask( unsigned int bit ) {
	return 1 << ( bit % SLOT_BITS );
}

static inline bitslot_t bitslot( unsigned int bit ) {
	return bit / SLOT_BITS;
}

static inline void bitset( bitslot_t * set, unsigned int bit ) {
	set[bitslot( bit )] |= bitmask( bit );
}

static inline void bitclear( bitslot_t * set, unsigned int bit ) {
	set[bitslot( bit )] &= ~bitmask( bit );
}

static inline int bittest( bitslot_t * set, unsigned int bit ) {
	return (int)( set[bitslot( bit )] & bitmask( bit ) );
}

static inline bitslot_t bitslots( unsigned int bits ) {
	return (bits + SLOT_BITS - 1) / SLOT_BITS;
}

static inline size_t Align( size_t unaligned ) {
#if MMEM_ALIGNMENT_POLCY & MMEM_ALIGNMENT_POLICY_HEAP
	return unaligned + (MMEM_ALIGNMENT_CACHELINE - unaligned % MMEM_ALIGNMENT_CACHELINE);
#else
	return unaligned;
#endif
}

static inline void ZeroOnAllocate( void * element, size_t const size ) {
#if MMEM_ZERO_POLICY == MMEM_ZERO_POLICY_ONALLOCATE
	memset( element, 0x00, size );
#else
	(void)(element); (void)(size);
	return;
#endif
}

static inline void ZeroOnRelease( void * element, size_t const size ) {
#if MMEM_ZERO_POLICY == MMEM_ZERO_POLICY_ONRELEASE
	memset( element, 0x00, size );
#else
	(void)(element); (void)(size);
	return;
#endif
}

static inline void * Allocate( size_t const capacity, size_t const size ) {
#if MMEM_ZERO_POLICY != MMEM_ZERO_POLICY_ONRELEASE
	return malloc( Align( capacity ) * size );
#else
	return calloc( Align( capacity ), size );
#endif
}

MemoryPool PoolCreate( size_t const p_element_size, size_t const p_capacity ) {
	return PoolCreateEx( p_element_size, p_capacity, Allocate, free );
}

MemoryPool PoolCreateEx( size_t const p_element_size, size_t const p_capacity, AllocateFct const p_allocate_fct, ReleaseFct const p_release_fct ) {
	return (MemoryPool) {
		.Used = 0,
		.Cursor = 0,
		.List = calloc( Align( bitslots( (bitslot_t)p_capacity ) ), sizeof( bitslot_t ) ),
		.Raw = p_allocate_fct ? p_allocate_fct( p_capacity, p_element_size ) : Allocate( p_element_size, p_capacity ),
		.ElementSize = p_element_size,
		.Capacity = Align( p_capacity ),
		.Release = p_release_fct ? p_release_fct : free
	};
}

void PoolDestroy( MemoryPool * p_pool ) {
	p_pool->Release( p_pool->Raw );
	free( p_pool->List );

#if MMEM_ZERO_POLICY == MMEM_ZERO_POLICY_ONRELEASE
	memset( p_pool, 0x00, sizeof( MemoryPool ) );
#endif
}

void * PoolAllocate( MemoryPool * p_pool ) {
	size_t cap = p_pool->Capacity;
	bitslot_t * set = p_pool->List;

	for ( size_t i = 0; i < cap; ++i ) {
		unsigned int index = (unsigned int)((p_pool->Cursor + i) % cap);
		if ( !bittest( set, index ) ) {
			bitset( set, index );
			p_pool->Used++;
			p_pool->Cursor = (index + 1) % cap;
			void * ptr = (char *)p_pool->Raw + index * p_pool->ElementSize;
			ZeroOnAllocate( ptr, p_pool->ElementSize );
			return ptr;
		}
	}

	return NULL;
}

void PoolRelease( MemoryPool * p_pool, void * p_element ) {
	// Check for alignment. Misaligned pointers cannot be valid objects of this pool
	bitslot_t offset = (bitslot_t)((char *)p_element - (char *)p_pool->Raw);
	if ( offset % p_pool->ElementSize != 0 ) {
		return;
	}

	// Check for bounds. Out of bounds pointers cannot be valid objects of this pool
	bitslot_t index = (bitslot_t)(offset / p_pool->ElementSize);
	if ( index >= p_pool->Capacity ) {
		return;
	}

	// Check if the object is actually a managed object and not a unallocated/released slot.
	if ( bittest( p_pool->List, index ) ) {
		bitclear( p_pool->List, index );
		ZeroOnRelease( p_element, p_pool->ElementSize );
		p_pool->Used--;
		if ( index < p_pool->Cursor ) {
			p_pool->Cursor = index;
		}
	}
}

void PoolReset( MemoryPool * p_pool ) {
	p_pool->Used = 0;
	p_pool->Cursor = 0;

#if MMEM_ZERO_POLICY == MMEM_ZERO_POLICY_ONRELEASE
	memset( p_pool->Raw, 0x00, p_pool->Capacity );
#endif

	memset( p_pool->List, 0x00, bitslots( (bitslot_t)p_pool->Capacity ) );
}

MemoryArena ArenaCreate( const size_t p_capacity ) {
	return ArenaCreateEx( p_capacity, Allocate, free );
}

MemoryArena ArenaCreateEx( const size_t p_capacity, AllocateFct const p_allocate_fct, ReleaseFct const p_release_fct ) {
	return (MemoryArena) {
		.Used = 0,
		.Raw = p_allocate_fct ? p_allocate_fct( p_capacity, 1 ) : Allocate( p_capacity, 1 ),
		.Capacity = Align( p_capacity ),
		.Release = p_release_fct ? p_release_fct : free
	};
}

void ArenaDestroy( MemoryArena * p_arena ) {
	p_arena->Release( p_arena->Raw );

#if MMEM_ZERO_POLICY == MMEM_ZERO_POLICY_ONRELEASE
	memset( p_arena, 0x00, sizeof( MemoryArena ) );
#endif
}

void * ArenaAllocate( MemoryArena * p_arena, size_t p_size ) {
	if ( p_arena->Used + p_size > p_arena->Capacity ) {
		return NULL;
	}

	void * ptr = (char *)p_arena->Raw + p_arena->Used;
	ZeroOnAllocate( ptr, p_size );

	p_arena->Used += p_size;
	return ptr;
}

void ArenaReset( MemoryArena * p_arena ) {
	p_arena->Used = 0;

#if MMEM_ZERO_POLICY == MMEM_ZERO_POLICY_ONRELEASE
	memset( p_arena->Raw, 0x00, p_arena->Capacity );
#endif
}
