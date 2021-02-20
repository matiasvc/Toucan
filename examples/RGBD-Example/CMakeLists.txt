add_executable(
		rgbd-example
		main.cpp
		DataLoader.cpp
)

set_target_properties(
		rgbd-example PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS ON
		CXX_STANDARD_REQUIRED ON
		POSITION_INDEPENDENT_CODE ON
)

add_library(stb STATIC stb/stb_image.c)
target_include_directories(rgbd-example PRIVATE stb)

target_link_libraries(
		rgbd-example
		PRIVATE Toucan::Toucan stb stdc++fs
)

file(COPY dataset DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
