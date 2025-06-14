#include "mmem.h"

#include <string.h>

#define MMEM_ZERO_POLICY_ONALLOCATE 0
#define MMEM_ZERO_POLICY_ONRELEASE 1
#define MMEM_ZERO_POLICY_MANUAL 2

#if !defined( MMEM_ZERO_POLICY ) || ( MMEM_ZERO_POLICY != 0 && MMEM_ZERO_POLICY != 1 && MMEM_ZERO_POLICY != 2 )
#define MMEM_ZERO_POLICY MMEM_ZERO_POLICY_ONRELEASE
#endif

typedef unsigned int bitslot_t;
#define SLOT_BITS ((bitslot_t)sizeof( bitslot_t ) * 8)

inline bitslot_t bitmask( unsigned int bit ) {
	return 1 << ( bit % SLOT_BITS );
}

inline bitslot_t bitslot( unsigned int bit ) {
	return bit / SLOT_BITS;
}

inline void bitset( bitslot_t * set, unsigned int bit ) {
	set[bitslot( bit )] |= bitmask( bit );
}

inline void bitclear( bitslot_t * set, unsigned int bit ) {
	set[bitslot( bit )] &= ~bitmask( bit );
}

inline int bittest( bitslot_t * set, unsigned int bit ) {
	return (int)( set[bitslot( bit )] & bitmask( bit ) );
}

inline bitslot_t bitslots( unsigned int bits ) {
	return (bits + SLOT_BITS - 1) / SLOT_BITS;
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

MemoryPool PoolCreate( size_t const p_element_size, size_t const p_capacity ) {
	return (MemoryPool) {
		.Raw = calloc( p_capacity, p_element_size ),
		.List = calloc( bitslots( (bitslot_t)p_capacity ), sizeof( bitslot_t ) ),
		.ElementSize = p_element_size,
		.Capacity = p_capacity
	};
}

void PoolDestroy( MemoryPool * p_pool ) {
	free( p_pool->Raw );
	free( p_pool->List );
	memset( p_pool, 0x00, sizeof( MemoryPool ) );
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
