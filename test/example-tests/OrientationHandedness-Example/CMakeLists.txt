cmake_minimum_required(VERSION 3.10)
project(orientation-handedness-example)

add_executable(
		orientation-handedness-example
		main.cpp
)

set_target_properties(
		orientation-handedness-example PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
)

# This if check is used to work better with a local copy of Toucan.
# In a regular project the 'if' can be omitted, as the 'find_package' command should always be called.
if (NOT Toucan_FOUND)
	find_package(Toucan REQUIRED)
endif (NOT Toucan_FOUND)

target_link_libraries(
		orientation-handedness-example
		PRIVATE Toucan::Toucan
)
