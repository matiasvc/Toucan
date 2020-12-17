# Toucan library target setup

set(
		Toucan_source
		Toucan.cpp
		Toucan_internals.cpp
		Toucan_render.cpp
		util/tick_number.cpp
		
		extern/glad/src/glad.c
		extern/glad/include/glad/glad.h
		extern/glad/include/KHR/khrplatform.h
		
		extern/imgui/imgui.cpp
		extern/imgui/imgui_draw.cpp
		extern/imgui/imgui_tables.cpp
		extern/imgui/imgui_widgets.cpp
		extern/imgui/backends/imgui_impl_opengl3.cpp
		extern/imgui/backends/imgui_impl_glfw.cpp
		extern/imgui/imgui_demo.cpp
		
		gl/geometry.cpp
		gl/shader.cpp
)

add_library(Toucan STATIC ${Toucan_source})
add_library(Toucan::Toucan ALIAS Toucan)


set_target_properties(
		Toucan PROPERTIES
		CXX_STANDARD 17
		CXX_EXTENSIONS ON
		CXX_STANDARD_REQUIRED ON
		POSITION_INDEPENDENT_CODE ON
)

target_compile_definitions(
		Toucan PRIVATE
		IMGUI_IMPL_OPENGL_LOADER_GLAD
)

target_include_directories(
		Toucan
			PUBLIC
				$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
				$<INSTALL_INTERFACE:include>
			PRIVATE
				${CMAKE_CURRENT_SOURCE_DIR}
				${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/include
				${CMAKE_CURRENT_SOURCE_DIR}/extern/renderdoc/include
				${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui
				${CMAKE_CURRENT_SOURCE_DIR}/extern/imgui/backends
)

target_compile_options(
		Toucan PRIVATE
			-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual
			-Wpedantic -Wnull-dereference -Wdouble-promotion -Wformat=2 #-Werror
			#-Wold-style-cast -Wconversion -Wsign-conversion
)

set_target_properties(
		Toucan PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
		LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# glfw
find_package(glfw3 3 REQUIRED)

# pthread
find_package(Threads REQUIRED)

# Link librares with Toucan
target_link_libraries(
		Toucan PUBLIC
		glfw
		Threads::Threads
		${CMAKE_DL_LIBS}
)


include(GNUInstallDirs)
install(
		TARGETS Toucan
		EXPORT ToucanTargets
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		INCLUDES DESTINATION ${LIBLEGACY_INCLUDE_DIRS}
)

install(
		DIRECTORY ${CMAKE_SOURCE_DIR}/include/Toucan
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
		EXPORT ToucanTargets
		FILE Toucan-targets.cmake
		NAMESPACE Toucan::
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Toucan
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
		${CMAKE_SOURCE_DIR}/cmake/ToucanConfig.cmake.in
		${CMAKE_BINARY_DIR}/cmake/ToucanConfig.cmake
		INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Toucan
)

write_basic_package_version_file(
		${CMAKE_BINARY_DIR}/cmake/ToucanConfigVersion.cmake
		VERSION ${PACKAGE_VERSION}
		COMPATIBILITY AnyNewerVersion
)

install(
		FILES
			${CMAKE_BINARY_DIR}/cmake/ToucanConfig.cmake
			${CMAKE_BINARY_DIR}/cmake/ToucanConfigVersion.cmake
		DESTINATION
			${CMAKE_INSTALL_LIBDIR}/cmake/Toucan
)

