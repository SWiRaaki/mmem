#include "mmem.h"
#include <bits/types/clock_t.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#define STORAGE_SIZE 1000000
#define ITERATIONS   10000

// Simple struct to test allocations
typedef struct {
    int x, y;
    float value;
} TestStruct;

typedef enum {
	BENCH_MALLOC,
	BENCH_CALLOC,
	BENCH_POOL_SINGLE,
	BENCH_POOL_BATCHED,
	BENCH_ARENA,
	BENCH_TOTAL
} Benchmark;

typedef void (*SimpleFunction)( TestStruct ** storage );

void AllocMalloc( TestStruct ** );
void AllocCalloc( TestStruct ** );
void AllocPool( TestStruct ** );
void AllocArena( TestStruct ** );

void ReleaseMallocCalloc( TestStruct ** );
void ReleasePoolSingle( TestStruct ** );
void ReleasePoolBatched( TestStruct ** );
void ReleaseArena( TestStruct ** );

MemoryPool pool;
MemoryArena arena;

char const * const bfile[BENCH_TOTAL] = {
	"malloc.bench.csv",
	"calloc.bench.csv",
	"pool_single.bench.csv",
	"pool_batched.bench.csv",
	"arena.bench.csv"
};

char const * const bstring[BENCH_TOTAL] = {
	"malloc",
	"calloc",
	"pool_single",
	"pool_batched",
	"arena"
};

char const * const blog[BENCH_TOTAL] = {
	" malloc         :",
	" calloc         :",
	" pool (single)  :",
	" pool (batched) :",
	" arena          :"
};

SimpleFunction const balloc[BENCH_TOTAL] = {
	AllocMalloc,
	AllocCalloc,
	AllocPool,
	AllocPool,
	AllocArena
};

SimpleFunction const bfree[BENCH_TOTAL] = {
	ReleaseMallocCalloc,
	ReleaseMallocCalloc,
	ReleasePoolSingle,
	ReleasePoolBatched,
	ReleaseArena
};

double bttotal[BENCH_TOTAL][ITERATIONS] = {0};
double btalloc[BENCH_TOTAL][ITERATIONS] = {0};
double btfree[BENCH_TOTAL][ITERATIONS] = {0};
double btmin[BENCH_TOTAL] = {0};
double btmax[BENCH_TOTAL] = {0};
double btmean[BENCH_TOTAL] = {0};
double btsdev[BENCH_TOTAL] = {0};

static inline void PrintLine( char const * msg, char const * prefix, va_list args ) {
	time_t t = time(NULL);
	struct tm * now = localtime( &t );
	char now_str[64];
	strftime( now_str, sizeof( now_str ), "%F %T", now );
	printf( "[%s]", now_str );
	
	if ( prefix )
		printf( "[%s]", prefix );
	
	printf( ":" );
	if ( msg )
		vprintf( msg, args );

	printf( "\n" );
}

static inline void Info( char const * msg, ... ) {
	va_list args;
	va_start( args, msg );
	PrintLine( msg, "LOG", args );
	va_end( args );
}

static inline void Debug( char const * msg, ... ) {
	va_list args;
	va_start( args, msg );
	PrintLine( msg, "DBG", args );
	va_end( args );
}

static inline void Warning( char const * msg, ... ) {
	va_list args;
	va_start( args, msg );
	PrintLine( msg, "WRN", args );
	va_end( args );
}

static inline void Error( char const * msg, ... ) {
	va_list args;
	va_start( args, msg );
	PrintLine( msg, "ERR", args );
	va_end( args );
}

static inline double Sum( double * numbers, int count ) {
	double ret = 0;
	for ( int i = 0; i < count; ++i )
		ret += numbers[i];

	return ret;
}

static inline double Timespan( clock_t begin, clock_t end ) {
	return (double)(end - begin) / CLOCKS_PER_SEC;
}

static inline double Measure( Benchmark benchmark, unsigned int seed, TestStruct ** params, int iteration ) {
	Info( "Preparing benchmark %s, iteration %d", bstring[benchmark], iteration );
	srand( seed );

	clock_t alloc_begin = 0;
	clock_t free_begin = 0;

	clock_t alloc_end = 0;
	clock_t free_end = 0;

	// Info( "Starting allocation.." );
	alloc_begin = clock();
	balloc[benchmark]( params );
	alloc_end = clock();
	// Info( "Allocation took %lf seconds", Timespan( alloc_begin, alloc_end ) );

	// Info( "Starting deallocation.." );
	free_begin = clock();
	bfree[benchmark]( params );
	free_end = clock();
	// Info( "Deallocation took %lf seconds", Timespan( free_begin, free_end ) );

	btalloc[benchmark][iteration] = Timespan( alloc_begin, alloc_end );
	btfree[benchmark][iteration] = Timespan( free_begin, free_end );
	bttotal[benchmark][iteration] = btalloc[benchmark][iteration] + btfree[benchmark][iteration];
	if ( bttotal[benchmark][iteration] < btmin[benchmark] )
		btmin[benchmark] = bttotal[benchmark][iteration];

	if ( bttotal[benchmark][iteration] > btmax[benchmark] )
		btmax[benchmark] = bttotal[benchmark][iteration];

	return bttotal[benchmark][iteration];
}

void AllocMalloc( TestStruct ** storage ) {
	for ( int i = 0; i < STORAGE_SIZE; ++i ) {
		storage[i] = malloc( sizeof( TestStruct ) );
		storage[i]->x = rand();
		storage[i]->y = rand();
		storage[i]->value = (float)rand() / (float)RAND_MAX;
	}
}

void AllocCalloc( TestStruct ** storage ) {
	for ( int i = 0; i < STORAGE_SIZE; ++i ) {
		storage[i] = calloc( 1, sizeof( TestStruct ) );
		storage[i]->x = rand();
		storage[i]->y = rand();
		storage[i]->value = (float)rand() / (float)RAND_MAX;
	}
}

void AllocPool( TestStruct ** storage ) {
	pool = PoolCreate( sizeof( TestStruct ), STORAGE_SIZE );
	for ( int i = 0; i < STORAGE_SIZE; ++i ) {
		storage[i] = PoolAllocate( &pool );
		storage[i]->x = rand();
		storage[i]->y = rand();
		storage[i]->value = (float)rand() / (float)RAND_MAX;
	}
}

void AllocArena( TestStruct ** storage ) {
	arena = ArenaCreate( sizeof( TestStruct ) * STORAGE_SIZE );
	for ( int i = 0; i < STORAGE_SIZE; ++i ) {
		storage[i] = ArenaAllocate( &arena, sizeof( TestStruct ) );
		storage[i]->x = rand();
		storage[i]->y = rand();
		storage[i]->value = (float)rand() / (float)RAND_MAX;
	}
}

void ReleaseMallocCalloc( TestStruct ** storage ) {
	for ( int i = 0; i < STORAGE_SIZE; ++i ) {
		free( storage[i] );
	}
}

void ReleasePoolSingle( TestStruct ** storage ) {
	for ( int i = 0; i < STORAGE_SIZE; ++i ) {
		PoolRelease( &pool, storage[i] );
	}
	PoolDestroy( &pool );
}

void ReleasePoolBatched( TestStruct ** storage ) {
	(void)(storage);
	PoolDestroy( &pool );
}

void ReleaseArena( TestStruct ** storage ) {
	(void)(storage);
	ArenaDestroy( &arena );
}

int main(void) {
	for ( Benchmark bench = 0; bench < BENCH_TOTAL; ++bench ) {
		btmin[bench] = 10000000;
	}

	TestStruct ** storage = calloc( STORAGE_SIZE, sizeof( TestStruct * ) );

	for ( int i = 0; i < ITERATIONS; ++i ) {
		unsigned int seed = (unsigned int)clock();

		for ( Benchmark bench = 0; bench < BENCH_TOTAL; ++bench ) {
			// Info( "%s %lf seconds", blog[bench], Measure( bench, seed, storage, i ) );
			Measure( bench, seed, storage, i );
		}
	}

	for ( Benchmark bench = 0; bench < BENCH_TOTAL; ++bench ) {
		btmean[bench] = Sum( bttotal[bench], ITERATIONS ) / ITERATIONS;
	}

	for ( Benchmark bench = 0; bench < BENCH_TOTAL; ++bench ) {
		double ssq_diff = 0;

		for ( int i = 0; i < ITERATIONS; ++i ) {
			double diff = bttotal[bench][i] - btmean[bench];
			ssq_diff += diff * diff;
		}
		btsdev[bench] = sqrt( ssq_diff / ITERATIONS );
	}

	Info( "Test finished. saving results.." );
	FILE * f = fopen( "benchmarks.csv", "w+" );
	fprintf( f, "Benchmark;Allocation;Free;Total;Lowest;Highest;Mean;Standard Deviation" );

	for ( Benchmark bench = 0; bench < BENCH_TOTAL; ++bench ) {
		FILE * fbench = fopen( bfile[bench], "w+" );
		fprintf( fbench, "Iteration;Allocation;Free;Total" );
		for ( int i = 0; i < ITERATIONS; ++i ) {
			fprintf( fbench, "\n%d;%lf;%lf;%lf", i + 1, btalloc[bench][i], btfree[bench][i], bttotal[bench][i] );
		}
		fclose( fbench );

		fprintf( f,
			"\n%s;%lf;%lf;%lf;%lf;%lf;%lf;%lf",
			bstring[bench],
			Sum( btalloc[bench], ITERATIONS ),
			Sum( btfree[bench], ITERATIONS ),
			Sum( bttotal[bench], ITERATIONS),
			btmin[bench],
			btmax[bench],
			btmean[bench],
			btsdev[bench]
		);
	}

	fclose( f );

    return EXIT_SUCCESS;
}
