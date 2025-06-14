#ifndef MMEM_H
#define MMEM_H

#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Memory Pool state structure.
 * @details Refrain from accessing members directly unless you know what you do!
 */
typedef struct {
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
	/// @brief Padding bytes
	size_t __PADDING[2];
} MemoryPool;

/**
 * @brief Creates a pool for elements of given size
 * @param element_size	Size of the object types in bytes
 * @param capacity		Maximum number of objects managable
 * @return MemoryPool	Clean state of the pool
 */
MemoryPool PoolCreate( size_t const element_size, size_t const capacity );

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
 * @param pool	Memory pool to PoolReset
 */
void PoolReset( MemoryPool * pool );

/**
 * @brief Receive the number of slots used by the specified pool
 * @param pool		Memory Pool to check
 * @return size_t	Number of allocated object slots
 */
inline size_t PoolSlotsInUse( MemoryPool * pool ) {
	return pool->Used;
}

/**
 * @brief Receive the number of slots available in the specified pool
 * @param pool		Memory Pool to check
 * @return size_t	Number of unallocated object slots
 */
inline size_t PoolSlotsAvailable( MemoryPool * pool ) {
	return pool->Capacity - pool->Used;
}

#endif // MMEM_H
