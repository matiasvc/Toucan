add_executable(
		image2d-example
		main.cpp
)

set_target_properties(
		image2d-example PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS ON
		CXX_STANDARD_REQUIRED ON
		POSITION_INDEPENDENT_CODE ON
)

# pthread
find_package(Threads REQUIRED)

target_link_libraries(
		image2d-example PRIVATE
		Toucan::Toucan
		Threads::Threads
)
