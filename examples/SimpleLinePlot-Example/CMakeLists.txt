add_executable(
		simple-line-plot-example
		main.cpp
)

set_target_properties(
		simple-line-plot-example PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS ON
		CXX_STANDARD_REQUIRED ON
		POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(
		simple-line-plot-example
		PRIVATE Toucan::Toucan
)
