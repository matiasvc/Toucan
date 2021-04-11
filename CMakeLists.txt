cmake_minimum_required(VERSION 3.16)
project(Toucan
		LANGUAGES C CXX
		VERSION 0.1
		DESCRIPTION "A C++ visualization library for computer vision and robotics.")

# Global CMake settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(src)

option(BUILD_EXAMPLES "Build examples." OFF)
if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

option(BUILD_TESTS "Build tests." OFF)
if(BUILD_TESTS)
	add_subdirectory(test)
endif()
