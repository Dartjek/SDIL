cmake_minimum_required(VERSION 3.16)
project(SDIL)

include(CTest)

set(CMAKE_CXX_STANDARD 17)

add_library(SDIL source/SDIL.cpp includes/SDIL.hpp includes/SDIL_internal.hpp)

message(${CMAKE_CXX_COMPILER_ID})
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	target_compile_options(SDIL PRIVATE /W4 /GR-)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "CLANG")
	target_compile_options(SDIL PRIVATE -Wall -fno-rtti)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	target_compile_options(SDIL PRIVATE -Wall -fno-rtti)
endif()

target_include_directories(SDIL INTERFACE includes)
target_include_directories(SDIL PRIVATE includes)

if (BUILD_TESTING)
	add_subdirectory(tests)
endif()