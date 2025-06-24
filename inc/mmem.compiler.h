#ifndef MMEM_COMPILER_H
#define MMEM_COMPILER_H

#if defined( _MSC_VER )
    #define MMEM_ALIGNED( alignment ) __declspec( align( alignment ) )
#elif defined( __GNUC__ ) || defined( __clang__ )
    #define MMEM_ALIGNED( alignment ) __attribute__(( aligned( alignment ) ))
#else
    #define MMEM_ALIGNED( alignment ) /* no-op */
#endif

#endif // MMEM_COMPILER_H
