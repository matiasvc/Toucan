add_executable(
		inputs-example
		main.cpp
)

set_target_properties(
		inputs-example PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS ON
		CXX_STANDARD_REQUIRED ON
		POSITION_INDEPENDENT_CODE ON
)
target_link_libraries(
		inputs-example
		PRIVATE Toucan::Toucan
)
