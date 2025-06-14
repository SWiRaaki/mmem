if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

cmake_minimum_required( VERSION 3.10 )

project( mmem C )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Release )
endif()
set( CMAKE_C_FLAGS "" )
set( CMAKE_C_FLAGS_DEBUG "-DDEBUG" )
set( CMAKE_C_FLAGS_RELEASE "-DRELEASE" )
# set( CMAKE_VERBOSE_MAKEFILE ON )

set( SRCFILES
	"src/mmem.c"
)
add_library(
	mmem_static STATIC
	${SRCFILES}
)
add_executable(
	mmem_test
	"main.c"
)
if( WIN32 )
	# message( STATUS "Build for Windows starts.." )
	add_definitions( -DPLATFORM_WINDOWS )
	set( CMAKE_C_FLAGS ${CMAKE_C_FLAGS} " /clr:nostdlib /GS- /GR- /EHsc /Wall /WX /permissive /wd4820 /wd4668 /wd5039 /wd4514 /wd5045" )
	set( OS_LIBS "" )
elseif( UNIX )
	# message( STATUS "Build for Linux starts.." )
	add_definitions( -DPLATFORM_LINUX )
	set( CMAKE_C_FLAGS ${CMAKE_C_FLAGS} " -nodefaultlibs -fno-exceptions -pedantic -Wall -Wextra -Wconversion -Wreturn-type -Werror=all -Werror=extra -Werror=conversion -Werror=return-type -Wno-unused-function" )
	set( CMAKE_C_FLAGS_DEBUG "-g2" )
	set( CMAKE_C_FLAGS_RELEASE "-O3" )
	set( OS_LIBS "c" )
else()
	set( OS_LIBS "" )
endif()
target_compile_definitions(
	mmem_static
	PRIVATE MMEM_BUILD_STATIC
)
target_compile_definitions(
	mmem_test
	PRIVATE MMEM_BUILD_UTEST
)
target_link_libraries(
	mmem_test
	mmem_static
	${OS_LIBS}
)
target_include_directories(
	mmem_static
	PRIVATE inc/
	PRIVATE tpl/
)
target_include_directories(
	mmem_test
	PRIVATE inc/
	PRIVATE tpl/
)
set_property(
	TARGET mmem_static
	PROPERTY C_STANDARD 99
)
set_property(
	TARGET mmem_static
	PROPERTY OUTPUT_NAME mmem
)
set_property(
	TARGET mmem_test
	PROPERTY C_STANDARD 99
)
set_property(
	TARGET mmem_test
	PROPERTY OUTPUT_NAME mmem
)
include_directories(
	inc/
	tpl/
)
