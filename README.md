# mmem
Compact C allocator library for memory pools and memory arenas

Every struct in this library is not hidden and all members accessible.
But direct access should still be only done if you know excactly what you do.
Why? Because i want to give you the freedom to extend functionality as needed!

## Getting Started
clone the repository with:
```sh
git clone https://github.com/SWiRaaki/mmem
```
You can either copy inc/mmem.h and src/mmem.c into your project or build the whole project with the CMAKE provided and
link against the static library produced by cmake. for that you can also just copy the inc/mmem.h into your project.

## Key Features
- Memory pools for fast fixed-size object allocation and deallocation
- Memory arenas for bulk allocation and management of dynamic sized objects
- Policies to manipulate library behaviour with controlled and expected outcomes
- No dependencies, fully C99 compatible
- Small and focused codebase, since internals are accessible and extendable

## Philosophy
==mmem== encourages a mindset shift from usual c memory approaches:
**Think in terms of memory lifetimes and groups, not individual allocations**
- mmem is not a library for safety mechanism, it is a tool structured for responsibility
- Arenas help simplify temporary memory usage and reduce fragmentations
- More predictable performance and cleaner resource management

## Compiler Flags
Memory Pools have behavior policies defined.

### Zero Policy
The library is designed to be performant and with a known zero-state, but preferences may differ.
| Flag | Internal Name        | Description                                                                           |
|------|----------------------|---------------------------------------------------------------------------------------|
|     0|ZERO_POLICY_ONALLOCATE|The memory is not zero-initialized, but will set the objects data to 0x00 on allocation|
|     1| ZERO_POLICY_ONRELEASE|The memory is zero-initialized and every object released will be set to 0x00 immediatly|
|     2|    ZERO_POLICY_MANUAL|The memory is not zero-initialized and will not alter memory whatsoever                |
|   ANY|                   -/-|The memory will defaul to ZERO_POLICY_ONRELEASE                                        |

### Alignment Policy
The library is designed to internally align with cache to reduce cache misses. The object allocations are not aligned
| Flag | Internal Name         | Description                                                                          |
|------|-----------------------|--------------------------------------------------------------------------------------|
|     0|  ALIGNMENT_POLICY_NONE|No Alignment will be performed                                                        |
|     1|ALIGNMENT_POLICY_STRUCT|The MemoryPool and MemoryArena structs get padding bytes to be cache aligned          |
|     2| ALIGNMENT_POLICY_STACK|Any internal stack allocation of multiple objects will be cache aligned               |
|     4|  ALIGNMENT_POLICY_HEAP|Any internal heap allocation of multiple objects will be cache aligned                |
|     7|   ALIGNMENT_POLICY_ALL|All allignment features are enabled                                                   |


## Memory Pool
This library adds functionality to use memory pools.

### Definition
Memory Pools in this library refer to preallocated memory chunks managed by a pool handler struct.
These pools have dynamic initial size but can not grow in capacity after initialisation.
Every pool is also designed to be specialized for only one specified type.

### Usage
If you plan to dynamically allocate objects of the same type that may vary in count but share a lifetime,
create a new pool state and only allocate from there for those objects. After reaching the end of their liftime,
destroy the pool and go on in your code.

#### Example
```c 
#include <stdlib.h>
#include <stdio.h>

#include "mmem.h"

typedef struct {
	size_t	ID;
	char *	Name;
	float	X;
	float	Y;
	float	XDelta;
	float	YDelta;
	void *	DATA;
} Entity;

#define SOME_CONDITION 1

int main( void ) {
	MemoryPool pool = PoolCreate( sizeof( Entity ), 64 );

	while ( SOME_CONDITION ) {
		Entity * entity = NULL;
		printf( "Creating an entity..\n" );
		entity = PoolAllocate( &pool );
		entity->ID = PoolSlotsAvailable( &pool );
		entity->Name = "Unknown Entity";

		if ( PoolSlotsInUse( &pool ) > 10) {
			if ( rand() % 5 == 0 ) {
				printf( "Entity released!\n" );
				PoolRelease( &pool, entity );
			}
		}
		if ( entity ) {
			printf( "Entity: [%zu] %s\n", entity->ID, entity->Name ? entity->Name : "<NULL>" );
			printf( "Location: %f|%f, moving %f|%f\n", entity->X, entity->Y, entity->XDelta, entity->YDelta );
		} else {
			printf( "Entity: NULL\n" );
		}
		printf( "Pool: [USED=%zu] [AVAILABLE=%zu]\n", PoolSlotsInUse( &pool ), PoolSlotsAvailable( &pool ) );
		
		if ( PoolSlotsAvailable( &pool ) == 0 ) {
			break;
		}
	}

	printf( "Filled pool, did stuff, now releasing it..\n" );
	PoolDestroy( &pool );
	return 0;
}
```

## Memory Arena
This library adds functionality to use memory arenas.

### Definition
Memory Arenas in this library refer to preallocated memory chunks managed by a arena handler struct.
These arenas have dynamic initial size but can not grow in capacity after initialisation.
Every Arena is designed to make fast allocations for various types of objects that share the same lifetime.
Objects allocated from an arena can not be deallocated unless the complete arena is deallocated.
Arenas are also always faster than pools and no deallocation of single objects is expected, using arenas is preferred.

### Usage
```c
#include <stdio.h>
#include "mmem.h"

typedef struct {
    float x, y;
} Vec2;

int main( void ) {
    MemoryArena arena = ArenaCreate( sizeof(Vec2), 1024 );

    for ( int i = 0; i < 10; ++i );
        Vec2* v = ArenaAllocate( &arena, sizeof( Vec2 ) );
        printf( "Vector pre set: [x=%f|y=%f]\n", v->x, v->y );
        v->x = 1.f * i;
        v->y = 2.f * i;
        printf( "Vector post set: [x=%f|y=%f]\n", v->x, v->y );
    }

    ArenaDestroy( &arena );

    return 0;
}
```
## License
This project is licensed under the GNU GPL v3.0. You are free to use, modify and redistribute it under the same license.

### Special explicit clause
You are also free to use, modify and redistribute this in closed source projects as long as the usage is explicitly named.
That can mean being named in the credits of a game, special thanks sections or other means to publizice the usage.

## Planned Features
- Debug build support: memory poisoning, and assertions
- Reference Counting for analytics like valgrind does for malloc/calloc/free
- Benchmarks and performance comparisons with malloc and calloc

### Benchmarks
This is a special point in the planned features since having an automated test for this will help optimize the library.
For that i may need a specialized application written with this library, where it can be switched on and off.
