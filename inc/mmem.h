#ifndef MMEM_H
#define MMEM_H

#include <stdlib.h>
#include <stdbool.h>

#include "mmem.compiler.h"

#define MMEM_VERSION_MAJOR 0
#define MMEM_VERSION_MINOR 1
#define MMEM_VERSION_PATCH 0
#define MMEM_VERSION_STRING "0.1.0-beta"


/// @brief Factor to calculate kilobytes to bytes
#define MMEM_KB_FACTOR (1024LU)
/// @brief Factor to calculate megabytes to bytes
#define MMEM_MB_FACTOR (1024LU * 1024LU)
/// @brief Factor to calculate gigabytes to bytes
#define MMEM_GB_FACTOR (1024LU * 1024LU * 1024LU)
/// @brief Factor to calculate terabytes to bytes
#define MMEM_TB_FACTOR (1024LU * 1024LU * 1024LU * 1024LU)

#ifndef MMEM_ALIGNMENT_CACHELINE
/// @brief Size of the cache line alignment (bytes in one cache bucket)
#define MMEM_ALIGNMENT_CACHELINE 64
#endif
#ifndef MMEM_ALIGNMENT_CACHEL1
/// @brief Size of the expected L1 cache in bytes
#define MMEM_ALIGNMENT_CACHEL1 (16 * MMEM_KB_FACTOR);
#endif

#define MMEM_ZERO_POLICY_ONALLOCATE 0
#define MMEM_ZERO_POLICY_ONRELEASE  1
#define MMEM_ZERO_POLICY_MANUAL     2

#if !defined( MMEM_ZERO_POLICY ) || ( MMEM_ZERO_POLICY != 0 && MMEM_ZERO_POLICY != 1 && MMEM_ZERO_POLICY != 2 )
#define MMEM_ZERO_POLICY MMEM_ZERO_POLICY_ONRELEASE
#endif

#define MMEM_ALIGNMENT_POLICY_NONE   0
#define MMEM_ALIGNMENT_POLICY_STRUCT 1
#define MMEM_ALIGNMENT_POLICY_STACK  2
#define MMEM_ALIGNMENT_POLICY_HEAP   4
#define MMEM_ALIGNMENT_POLICY_ALL    7

#if !defined( MMEM_ALIGNMENT_POLICY )
#define MMEM_ALIGNMENT_POLICY MMEM_ALIGNMENT_POLICY_ALL
#endif

#if MMEM_ALIGNMENT_POLICY & MMEM_ALIGNMENT_POLICY_STRUCT
#define MMEM_STRUCT typedef struct MMEM_ALIGNED( MMEM_ALIGNMENT_CACHELINE )
#else
#define MMEM_STRUCT typedef struct
#endif

typedef void * (*AllocateFct)( size_t const element_size, size_t const capacity );
typedef void (*ReleaseFct)( void * memory );

/**
 * @brief Memory Pool state structure.
 * @details Refrain from accessing members directly unless you know what you do!
 */
MMEM_STRUCT {
	/// @brief Number of slots used in this pool
	size_t Used;
	/// @brief Active cursor for searching available slots
	size_t Cursor;
	/// @brief Anonymous List of slot states( USED/UNUSED )
	void * List;
	/// @brief Raw chunk of memory owned by this pool
	void * Raw;
	/// @brief Size of the element type managed by this pool in bytes
	size_t ElementSize;
	/// @brief Maximum number of elements managable by this pool
	size_t Capacity;
	/// @brief Pointer to a function that will be used to deallocate the memory block
	void (*Release)( void * chunk );
} MemoryPool;

MMEM_STRUCT {
	/// @brief Number of bytes used in this arena
	size_t Used;
	/// @brief Raw chunk of memory owned by this pool
	void * Raw;
	/// @brief Maximum number of bytes managable by this arena
	size_t Capacity;
	/// @brief Pointer to a function that will be used to deallocate the memory block
	void (*Release)( void * chunk );
} MemoryArena;

/**
 * @brief Calculates kilobytes to bytes
 * @param kilobytes	kilobytes to convert
 * @return size_t	bytes needed to represent given kilobytes
 */
static inline size_t KBytesToBytes( size_t kilobytes ) {
	return kilobytes * MMEM_KB_FACTOR;
}

/**
 * @brief Calculates kilobytes to bytes
 * @param kilobytes	kilobytes to convert
 * @return size_t	bytes needed to represent given kilobytes
 */
static inline size_t KBytesToBytesF( double kilobytes ) {
	return (size_t)(kilobytes * MMEM_KB_FACTOR);
}

/**
 * @brief Calculates megabytes to bytes
 * @param megabytes	megabytes to convert
 * @return size_t	bytes needed to represent given megabytes
 */
static inline size_t MBytesToBytes( size_t megabytes ) {
	return megabytes * MMEM_MB_FACTOR;
}

/**
 * @brief Calculates megabytes to bytes
 * @param megabytes	megabytes to convert
 * @return size_t	bytes needed to represent given megabytes
 */
static inline size_t MBytesToBytesF( double megabytes ) {
	return (size_t)(megabytes * MMEM_MB_FACTOR);
}

/**
 * @brief Calculates gigabytes to bytes
 * @param gigabytes gigabytes to convert
 * @return size_t	bytes needed to represent given gigabytes
 */
static inline size_t GBytesToBytes( size_t gigabytes ) {
	return gigabytes * MMEM_GB_FACTOR;
}

/**
 * @brief Calculates gigabytes to bytes
 * @param gigabytes gigabytes to convert
 * @return size_t	bytes needed to represent given gigabytes
 */
static inline size_t GBytesToBytesF( double gigabytes ) {
	return (size_t)(gigabytes * MMEM_GB_FACTOR);
}

/**
 * @brief Calculates terabytes to bytes
 * @param terabytes terabytes to convert
 * @return size_t	bytes needed to represent given terabytes
 */
static inline size_t TBytesToBytes( size_t terabytes ) {
	return terabytes * MMEM_TB_FACTOR;
}

/**
 * @brief Calculates terabytes to bytes
 * @param terabytes terabytes to convert
 * @return size_t	bytes needed to represent given terabytes
 */
static inline size_t TBytesToBytesF( double terabytes ) {
	return (size_t)(terabytes * MMEM_TB_FACTOR);
}

/**
 * @brief Creates a pool for elements of given size
 * @param element_size	Size of the object types in bytes
 * @param capacity		Maximum number of objects managable
 * @return MemoryPool	Clean state of the pool
 */
MemoryPool PoolCreate( size_t const element_size, size_t const capacity );

/**
 * @brief Creates a pool for elements of given size
 * @param element_size	Size of the object types in bytes
 * @param capacity		Maximum number of objects managable
 * @param allocate_fct	Pointer to the function to preallocate the pool memory
 * @param release_fct	Pointer to the function to release the pool memory
 * @return MemoryPool	Clean state of the pool
 */
MemoryPool PoolCreateEx( size_t const element_size, size_t const capacity, AllocateFct const allocate_fct, ReleaseFct const release_fct );

/**
 * @brief Releases all allocated resources of the pool to the operating system and invalidates the state
 * @param pool	Memory pool to release and invalidate
 */
void PoolDestroy( MemoryPool * pool );

/**
 * @brief Allocates an object owned by the pool
 * @param pool		Memory pool to own and manage the object
 * @return void *	Pointer to the object allocated
 */
void * PoolAllocate( MemoryPool * pool );

/**
 * @brief Releases the resources of an object if it is managed by the pool
 * @param pool		Owner of the object
 * @param element	Pointer to the element managed by the pool
 */
void PoolRelease( MemoryPool * pool, void * element );

/**
 * @brief Releases all resources of the pool without releasing the resources to the os or invalidate it
 * @param pool	Memory pool to reset
 */
void PoolReset( MemoryPool * pool );

/**
 * @brief Receive the number of slots used by the specified pool
 * @param pool		Memory Pool to check
 * @return size_t	Number of allocated object slots
 */
static inline size_t PoolSlotsInUse( MemoryPool * pool ) {
	return pool->Used;
}

/**
 * @brief Receive the number of slots available in the specified pool
 * @param pool		Memory Pool to check
 * @return size_t	Number of unallocated object slots
 */
static inline size_t PoolSlotsAvailable( MemoryPool * pool ) {
	return pool->Capacity - pool->Used;
}

/**
 * @brief Creates an arena of given number of bytes
 * @param capacity		Number of bytes managed by this arena
 * @return MemoryArena	Clean state of the arena
 */
MemoryArena ArenaCreate( size_t const capacity );

/**
 * @brief Creates an arena of given number of bytes
 * @param capacity		Number of bytes managed by this arena
 * @param allocate_fct	Pointer to the function to preallocate the arena memory
 * @param release_fct	Pointer to the function to release the arena memory
 * @return MemoryArena	Clean state of the arena
 */
MemoryArena ArenaCreateEx( size_t const capacity, AllocateFct const allocate_fct, ReleaseFct const release_fct );

/**
 * @brief Releases all allocated resources of the arena to the operating system and invalidates the state
 * @param arena	Memory arena to release and invalidate
 */
void ArenaDestroy( MemoryArena * arena );

/**
 * @brief Allocates an object owned by the arena
 * @param arena		Memory pool to own and manage the object
 * @param size		Number of bytes needed for the object
 * @return void *	Pointer to the object allocated
 */
void * ArenaAllocate( MemoryArena * arena, size_t size );

/**
 * @brief Releases all resources of the arena without releasing the resources to the os or invalidate it
 * @param arena	Memory arena to reset
 */
void ArenaReset( MemoryArena * arena );

/**
 * @brief Receive the number of bytes used by the specified arena
 * @param arena		Memory arena to check
 * @return size_t	Number of allocated bytes
 */
static inline size_t ArenaBytesInUse( MemoryArena * arena ) {
	return arena->Used;
}

/**
 * @brief Receive the number of bytes available in the specified arena
 * @param arena		Memory arena to check
 * @return size_t	Number of available bytes
 */
static inline size_t ArenaBytesAvailable( MemoryArena * arena ) {
	return arena->Capacity - arena->Used;
}

#endif // MMEM_H
