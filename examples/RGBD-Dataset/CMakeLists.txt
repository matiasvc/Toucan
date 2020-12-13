add_executable(
		rgbd-dataset-example
		main.cpp
		DataLoader.cpp
)

set_target_properties(
		rgbd-dataset-example PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS ON
		CXX_STANDARD_REQUIRED ON
		POSITION_INDEPENDENT_CODE ON
)

add_library(stb STATIC stb/stb_image.c)
target_include_directories(rgbd-dataset-example PRIVATE stb)

find_package(Eigen3 REQUIRED NO_MODULE)

find_package(Sophus REQUIRED)
target_include_directories(rgbd-dataset-example PRIVATE ${Sophus_INCLUDE_DIRS})

target_link_libraries(
		rgbd-dataset-example
		PRIVATE Toucan::Toucan stb stdc++fs Eigen3::Eigen
)

file(COPY dataset DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
