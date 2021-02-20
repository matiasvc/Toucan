add_subdirectory(Catch2)

set(
		Toucan_test_source
		tests.cpp
		LinAlg_test.cpp
)

add_executable(Toucan_test ${Toucan_test_source})

target_include_directories(
		Toucan_test PRIVATE
		../src ../include)

target_compile_features(
		Toucan_test PRIVATE
		cxx_std_17
)

target_compile_options(
		Toucan_test PRIVATE
		-Wall -Wextra -Wpedantic -Werror
)

include(CTest)

find_package(Eigen3 REQUIRED NO_MODULE)
target_link_libraries(
		Toucan_test PRIVATE
		Eigen3::Eigen
		Catch2::Catch2
)
