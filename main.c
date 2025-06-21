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
	return 0;

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
