add_executable(
		orientation-handedness-example
		main.cpp
)

set_target_properties(
		orientation-handedness-example PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS ON
		CXX_STANDARD_REQUIRED ON
		POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(
		orientation-handedness-example
		PRIVATE Toucan::Toucan
)
