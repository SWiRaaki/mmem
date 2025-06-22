#include "mmem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Simple struct to test allocations
typedef struct {
    int x, y;
    float value;
} TestStruct;

#define POOL_CAPACITY 64
#define ARENA_SIZE (1024 * sizeof(TestStruct))

int main(void) {
	printf( "Zero-Policy: %d\n", MMEM_ZERO_POLICY );
	printf( "Alignment-Policy: %d\n", MMEM_ALIGNMENT_POLICY );
	printf( "sizeof MemoryPool: %zu\n", sizeof( MemoryPool ) );
	printf( "padding: %lu\n", MMEM_POOL_PADDING );
	printf( "sizeof ComplexPool: %zu\n", sizeof( ComplexPool ) );
	printf( "padding: %lu\n", MMEM_CPOOL_PADDING );
	printf( "sizeof MemoryArena: %zu\n", sizeof( MemoryArena ) );
	printf( "padding: %lu\n", MMEM_ARENA_PADDING );
	printf( "sizeof ComplexArena: %zu\n", sizeof( ComplexArena ) );
	printf( "padding: %lu\n", MMEM_CARENA_PADDING );

    // --- Pool Test ---
    MemoryPool pool = PoolCreate(sizeof(TestStruct), POOL_CAPACITY);

    TestStruct *pool_ptrs[POOL_CAPACITY];
    for (size_t i = 0; i < POOL_CAPACITY; ++i) {
        pool_ptrs[i] = PoolAllocate(&pool);
        assert(pool_ptrs[i] != NULL);
        pool_ptrs[i]->x = (int)i;
        pool_ptrs[i]->y = (int)(i * 2);
        pool_ptrs[i]->value = (float)i / 2.0f;
    }

    // Check reuse logic
    for (size_t i = 0; i < POOL_CAPACITY; ++i) {
        PoolRelease(&pool, pool_ptrs[i]);
    }

    assert(PoolSlotsAvailable(&pool) == POOL_CAPACITY);
    PoolDestroy(&pool);

    // --- Arena Test ---
    MemoryArena arena = ArenaCreate(ARENA_SIZE);

    TestStruct *a1 = ArenaAllocate(&arena, sizeof(TestStruct));
    TestStruct *a2 = ArenaAllocate(&arena, sizeof(TestStruct));
    TestStruct *a3 = ArenaAllocate(&arena, sizeof(TestStruct));

    assert(a1 && a2 && a3);

    a1->x = 1;
    a2->x = 2;
    a3->x = 3;

    ArenaReset(&arena);

    TestStruct *a4 = ArenaAllocate(&arena, sizeof(TestStruct));
    assert(a4 != NULL);

    ArenaDestroy(&arena);

    // --- Success ---
    printf("[mmem test] All tests passed.\n");
    return EXIT_SUCCESS;
}
