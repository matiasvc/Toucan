add_executable(
		primitives3d-example
		main.cpp
)

set_target_properties(
		primitives3d-example PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS ON
		CXX_STANDARD_REQUIRED ON
		POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(
		primitives3d-example
		PRIVATE Toucan::Toucan
)
