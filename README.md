# mmem
Compact C allocator library for memory pools and memory arenas

Every struct in this library is not hidden and all members accessible.
But direct access should still be only done if you know excactly what you do.
Why? Because i want to give you the freedom to extend functionality as needed!

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

### Compiler Flags
Memory Pools have behavior policies defined. Currently, there is only the ZeroPolicy, via the compiler flag MMEM_ZERO_POLICY.
It is using a policy since it is designed to be performant and with a known zero-state, but preferences may differ.
| Flag | Internal Name        | Description                                                                         |
|------|----------------------|-------------------------------------------------------------------------------------|
|     0|ZERO_POLICY_ONALLOCATE|The pool is not zero-initialized, but will set the objects data to 0x00 on allocation|
|     1| ZERO_POLICY_ONRELEASE|The pool is zero-initialized and every object released will be set to 0x00 immediatly|
|     2|    ZERO_POLICY_MANUAL|The pool is not zero-initialized and will not alter memory whatsoever                |
|   ANY|                   -/-|The pool will defaul to ZERO_POLICY_ONRELEASE                                        |

## Memory Arena
This library adds functionality to use memory arenas.

### Definition
Memory Arenas in this library refer to preallocated memory chunks managed by a arena handler struct.
These arenas have dynamic initial size but can not grow in capacity after initialisation.
Every Arena is designed to make fast allocations for various types of objects that share the same lifetime.
Objects allocated from an arena can not be deallocated unless the complete arena is deallocated.

### Usage
TO BE DEFINED
