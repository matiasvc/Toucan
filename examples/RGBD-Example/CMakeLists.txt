cmake_minimum_required(VERSION 3.10)
project(rgbd-example)

add_executable(
		rgbd-example
		main.cpp
		DataLoader.cpp DataLoader.h
		stb_image.h
)

set_target_properties(
		rgbd-example PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
)

# This if check is used to work better with a local copy of Toucan.
# In a regular project the 'if' can be omitted, as the 'find_package' command should always be called.
if (NOT Toucan_FOUND)
	find_package(Toucan REQUIRED)
endif (NOT Toucan_FOUND)

target_link_libraries(
		rgbd-example
		PRIVATE Toucan::Toucan stdc++fs
)

file(COPY dataset DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
