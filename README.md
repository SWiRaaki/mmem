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
TO BE DEFINED

## Memory Arena
This library adds functionality to use memory arenas.

### Definition
Memory Arenas in this library refer to preallocated memory chunks managed by a arena handler struct.
These arenas have dynamic initial size but can not grow in capacity after initialisation.
Every Arena is designed to make fast allocations for various types of objects that share the same lifetime.
Objects allocated from an arena can not be deallocated unless the complete arena is deallocated.

### Usage
TO BE DEFINED
