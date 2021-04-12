#include <Toucan/Toucan.h>

#include <atomic>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Toucan/DataTypes.h>

#include "internal.h"
#include "render.h"
#include "Utils.h"
#include "util/tick_number.h"
#include "gl/projection.h"
#include "gl/error.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <renderdoc/renderdoc.h>
#include <dlfcn.h>

Toucan::ToucanContext* toucan_context_ptr = nullptr;

// Forward declarations
void render_loop(Toucan::ToucanSettings);
static void glfw_error_callback(int error, const char* description);
static void glfw_window_close_callback(GLFWwindow* window);

// Validation macros
#define validate_initialized(function_name) \
if (toucan_context_ptr == nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called before Toucan was initialized. Did you forget to call 'Toucan::Initialize'?"); }

#define validate_active_figure2d(function_name) \
if (toucan_context_ptr->current_figure_2d == nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called without an active Figure2D. Did you forget to call 'Toucan::BeginFigure2D'?"); }

#define validate_inactive_figure2d(function_name) \
if (toucan_context_ptr->current_figure_2d != nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called while another Figure2D was active. Did you forget to call 'Toucan::EndFigure2D'?"); }

#define validate_active_figure3d(function_name) \
if (toucan_context_ptr->current_figure_3d == nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called without an active Figure2D. Did you forget to call 'Toucan::BeginFigure2D'?"); }

#define validate_inactive_figure3d(function_name) \
if (toucan_context_ptr->current_figure_3d != nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called while another Figure2D was active. Did you forget to call 'Toucan::EndFigure2D'?"); }

#define validate_active_input_window(function_name) \
if (toucan_context_ptr->current_input_window == nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called without an active Figure2D. Did you forget to call 'Toucan::BeginInputWindow'?"); }

#define validate_inactive_input_window(function_name) \
if (toucan_context_ptr->current_input_window != nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called while another InputWindow was active. Did you forget to call 'Toucan::EndInputWindow'?"); }

Toucan::Element2D& get_or_create_element_2d(Toucan::Figure2D& figure, const std::string& name, int draw_layer, Toucan::ElementType2D type) {
	// Does the Element2D object with that name already exist?
	Toucan::Element2D* current_element_ptr = nullptr;
	for (auto& element : figure.elements) {
		if (element.name == name) {
			current_element_ptr = &element;
			break;
		}
	}
	
	// If it does not exist, we must create a new one at the correct location according to its draw layer.
	if (current_element_ptr == nullptr) {
		auto element_iterator = figure.elements.begin();
		
		for (; element_iterator != figure.elements.end(); element_iterator++) {
			if (draw_layer < element_iterator->draw_layer) {
				break;
			}
		}
		
		Toucan::Element2D new_element_2d(name, type, draw_layer);
		
		auto insertion_iterator = figure.elements.insert(element_iterator, std::move(new_element_2d));
		current_element_ptr = &(*insertion_iterator);
	}
	
	return *current_element_ptr;
}


Toucan::Element3D& get_or_create_element_3d(Toucan::Figure3D& figure, const std::string& name, Toucan::ElementType3D type) {
	// Does the Element3D object with that name already exist?
	Toucan::Element3D* current_element_ptr = nullptr;
	for (auto& element : figure.elements) {
		if (element.name == name) {
			current_element_ptr = &element;
			break;
		}
	}
	
	// If it does not exist, we must create a new one.
	if (current_element_ptr == nullptr) {
		
		Toucan::Element3D new_element_3d(name, type);
		
		auto& inserted_element = figure.elements.emplace_back(new_element_3d);
		current_element_ptr = &inserted_element;
	}
	
	return *current_element_ptr;
}

Toucan::ElementInput& get_or_create_element_input(Toucan::InputWindow& input_window, const std::string& name, Toucan::ElementInputType type) {
	// Does the Element3D object with that name already exist?
	Toucan::ElementInput* current_element_ptr = nullptr;
	for (auto& element : input_window.elements) {
		if (element.name == name) {
			current_element_ptr = &element;
			break;
		}
	}
	
	// If it does not exist, we must create a new one.
	if (current_element_ptr == nullptr) {
		
		Toucan::ElementInput new_element_3d = {};
		new_element_3d.name = name;
		new_element_3d.type = type;
		
		auto& inserted_element = input_window.elements.emplace_back(new_element_3d);
		current_element_ptr = &inserted_element;
	}
	
	return *current_element_ptr;
}

void Toucan::Initialize(Toucan::ToucanSettings settings) {
	if (toucan_context_ptr != nullptr) { throw std::runtime_error("Toucan error! 'Toucan::Initialize' was called when Toucan already was initialized. Did you call 'Toucan::Initialize' multiple times?"); }
	toucan_context_ptr = new ToucanContext;
	toucan_context_ptr->render_thread = std::thread(render_loop, settings);
	
	std::unique_lock lock(toucan_context_ptr->initialized_mutex);
	toucan_context_ptr->initialized_cv.wait(lock);
}

void Toucan::Destroy() {
	validate_initialized(Destroy)
	toucan_context_ptr->should_render = false;
	toucan_context_ptr->render_thread.join();
	delete toucan_context_ptr;
}

bool Toucan::IsWindowOpen() {
	validate_initialized(IsWindowOpen)
	return toucan_context_ptr->window_open;
}

void Toucan::SleepUntilWindowClosed() {
	validate_initialized(SleepUntilWindowClosed)
	
	if(not toucan_context_ptr->window_open) {
		return;
	}
	
	std::unique_lock lock(toucan_context_ptr->window_close_mutex);
	toucan_context_ptr->window_close_cv.wait(lock);
}


Toucan::Figure2DSettingsBuilder Toucan::BeginFigure2D(const std::string& name) {
	validate_initialized(BeginFigure2D)
	auto& toucan_context = *toucan_context_ptr;
	validate_inactive_figure2d(BeginFigure2D)
	
	Toucan::Figure2D* figure_2d_ptr = nullptr;
	for (auto& figure_2d : toucan_context.figures_2d) { // Does the Figure2D already exists?
		if (figure_2d.name == name) {
			figure_2d_ptr = &figure_2d;
			break;
		}
	}
	
	if (figure_2d_ptr == nullptr) { // Do we need to create a new Figure2D?
		auto& figure_2d = toucan_context.figures_2d.emplace_back();
		figure_2d.name = name;
		figure_2d_ptr = &figure_2d;
	}
	
	figure_2d_ptr->mutex.lock();
	
	assert(figure_2d_ptr->pose_stack.empty());
	figure_2d_ptr->pose_stack.emplace_back();
	
	toucan_context.current_figure_2d = figure_2d_ptr;
	return Toucan::Figure2DSettingsBuilder(&figure_2d_ptr->settings);
}

void Toucan::EndFigure2D() {
	validate_initialized(EndFigure2D)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_figure2d(EndFigure2D)
	
	auto& figure_2d = *toucan_context.current_figure_2d;
	figure_2d.pose_stack.clear();
	
	toucan_context.current_figure_2d->mutex.unlock();
	toucan_context.current_figure_2d = nullptr;
}

void Toucan::PushPose2D(const Toucan::RigidTransform2Df& pose) {
	validate_initialized(PushPose2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(PushPose2D)
	auto& current_figure = *context.current_figure_2d;
	
	const auto& parent_pose = current_figure.pose_stack.back();
	current_figure.pose_stack.emplace_back(parent_pose * pose);
}

void Toucan::PopPose2D() {
	validate_initialized(PopPose2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(PopPose2D)
	auto& current_figure = *context.current_figure_2d;
	
	if (current_figure.pose_stack.size() <= 1) { throw std::runtime_error("Toucan error! 'Toucan::PopPose2D' was called without a matching call to `Toucan::PushPose2D`."); }
	
	current_figure.pose_stack.pop_back();
}

void Toucan::ClearPose2D() {
	validate_initialized(PopPose2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(PopPose2D)
	auto& current_figure = *context.current_figure_2d;
	
	if (current_figure.pose_stack.size() <= 1) { throw std::runtime_error("Toucan error! 'Toucan::ClearPose2D' was called without any matching call to `Toucan::PushPose2D`."); }
	
	current_figure.pose_stack.clear();
	current_figure.pose_stack.emplace_back(); // Add identity pose back
}

Toucan::ShowLinePlot2DSettingsBuilder Toucan::ShowLinePlot2D(const std::string& name, const Toucan::Buffer<Toucan::Vector2f>& line_buffer, int draw_layer) {
	validate_initialized(PopPose2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(PopPose2D)
	auto& current_figure = *context.current_figure_2d;
	
	auto& current_element = get_or_create_element_2d(current_figure, name, draw_layer, Toucan::ElementType2D::LinePlot2D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	const size_t data_buffer_size = sizeof(Toucan::Vector2f)*line_buffer.number_of_elements;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, line_buffer.data_ptr, data_buffer_size);
	
	current_element.line_plot_2d_metadata.number_of_points = line_buffer.number_of_elements;
	return Toucan::ShowLinePlot2DSettingsBuilder(&current_element.line_plot_2d_metadata.settings);
}

Toucan::ShowPoints2DSettingsBuilder Toucan::ShowPoints2D(const std::string& name, const Buffer <Point2D>& points_buffer, int draw_layer) {
	validate_initialized(ShowPoints2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(ShowPoints2D)
	auto& current_figure = *context.current_figure_2d;
	
	auto& current_element = get_or_create_element_2d(current_figure, name, draw_layer, Toucan::ElementType2D::Point2D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	const size_t data_buffer_size = sizeof(Toucan::Point2D)*points_buffer.number_of_elements;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, points_buffer.data_ptr, data_buffer_size);
	
	current_element.point_2d_metadata.number_of_points = points_buffer.number_of_elements;
	return Toucan::ShowPoints2DSettingsBuilder(&current_element.point_2d_metadata.settings);
}


Toucan::ShowImage2DSettingsBuilder Toucan::ShowImage2D(const std::string& name, const Image2D& image, int draw_layer) {
	validate_initialized(ShowImage2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(ShowImage2D)
	auto& current_figure = *context.current_figure_2d;
	
	assert(image.width > 0 and image.height > 0 and image.image_buffer_ptr != nullptr);
	
	auto& current_element = get_or_create_element_2d(current_figure, name, draw_layer, Toucan::ElementType2D::Image2D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	size_t bytes_per_pixel = get_bytes_per_pixel(image.format);
	
	const size_t data_buffer_size = bytes_per_pixel*image.width*image.height;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, image.image_buffer_ptr, data_buffer_size);
	
	// TODO(Matias): Move quad data to context struct to have one global quad.
	current_element.image_2d_metadata.width = image.width;
	current_element.image_2d_metadata.height = image.height;
	current_element.image_2d_metadata.format = image.format;
	return Toucan::ShowImage2DSettingsBuilder(&current_element.image_2d_metadata.settings);
}

Toucan::Figure3DSettingsBuilder Toucan::BeginFigure3D(const std::string& name) {
	validate_initialized(BeginFigure3D)
	auto& toucan_context = * toucan_context_ptr;
	validate_inactive_figure3d(BeginFigure3D)
	
	Toucan::Figure3D* figure_3d_ptr = nullptr;
	for (auto& figure_3d : toucan_context.figures_3d) { // Does the Figure2D already exists?
		if (figure_3d.name == name) {
			figure_3d_ptr = &figure_3d;
			break;
		}
	}
	
	if (figure_3d_ptr == nullptr) { // Do we need to create a new Figure2D?
		auto& figure_3d = toucan_context.figures_3d.emplace_back();
		
		// Add default 3D grid
		Toucan::Element3D grid_element("Grid", ElementType3D::Grid3D);
		grid_element.data_buffer_ptr = reinterpret_cast<void*>(1); // Use this to signify new data
		grid_element.grid_3d_metadata.spacing = 1.0;
		grid_element.grid_3d_metadata.lines = 20;
		
		figure_3d.elements.emplace_back(grid_element);
		
		figure_3d.name = name;
		figure_3d_ptr = &figure_3d;
	}
	
	figure_3d_ptr->mutex.lock();
	
	assert(figure_3d_ptr->pose_stack.empty()); // TODO(Matias): Error message
	figure_3d_ptr->pose_stack.emplace_back();
	
	toucan_context.current_figure_3d = figure_3d_ptr;
	return Toucan::Figure3DSettingsBuilder(&figure_3d_ptr->settings);
}

void Toucan::EndFigure3D() {
	validate_initialized(EndFigure3D)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_figure3d(EndFigure3D)
	auto& figure_3d = *toucan_context.current_figure_3d;
	
	figure_3d.pose_stack.clear();
	
	toucan_context.current_figure_3d->mutex.unlock();
	toucan_context.current_figure_3d = nullptr;
}

void Toucan::PushPose3D(const Toucan::RigidTransform3Df& pose) {
	validate_initialized(PushPose3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(PushPose3D)
	auto& current_figure = *context.current_figure_3d;
	
	const auto& parent_pose = current_figure.pose_stack.back();
	current_figure.pose_stack.emplace_back(parent_pose * pose);
}

void Toucan::PopPose3D() {
	validate_initialized(PopPose3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(PopPose3D)
	auto& current_figure = *context.current_figure_3d;
	
	if (current_figure.pose_stack.size() <= 1) { throw std::runtime_error("Toucan error! 'Toucan::PopPose3D' was called without a matching call to `Toucan::PushPose3D`."); }
	
	current_figure.pose_stack.pop_back();
}

void Toucan::ClearPose3D() {
	validate_initialized(PopPose3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(PopPose3D)
	auto& current_figure = *context.current_figure_3d;
	
	if (current_figure.pose_stack.size() <= 1) { throw std::runtime_error("Toucan error! 'Toucan::ClearPose3D' was called without any matching call to `Toucan::PushPose3D`."); }
	
	current_figure.pose_stack.clear();
	current_figure.pose_stack.emplace_back(); // Add identity pose back
}

Toucan::ShowAxis3DSettingsBuilder Toucan::ShowAxis3D(const std::string& name) {
	validate_initialized(ShowAxis3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(ShowAxis3D)
	auto& current_figure = *context.current_figure_3d;
	
	auto& current_element = get_or_create_element_3d(current_figure, name, Toucan::ElementType3D::Axis3D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	current_element.data_buffer_ptr = reinterpret_cast<void*>(1);
	return Toucan::ShowAxis3DSettingsBuilder(&current_element.axis_3d_metadata.settings);
}

Toucan::ShowPoints3DSettingsBuilder Toucan::ShowPoints3D(const std::string& name, const Toucan::Buffer<Toucan::Point3D>& points_buffer) {
	validate_initialized(ShowPoints3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(ShowPoints3D)
	auto& current_figure = *context.current_figure_3d;
	
	auto& current_element = get_or_create_element_3d(current_figure, name, Toucan::ElementType3D::Point3D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	const size_t data_buffer_size = sizeof(Toucan::Point3D)*points_buffer.number_of_elements;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, points_buffer.data_ptr, data_buffer_size);
	
	current_element.point_3d_metadata.number_of_points = points_buffer.number_of_elements;
	return Toucan::ShowPoints3DSettingsBuilder(&current_element.point_3d_metadata.settings);
}


Toucan::ShowLines3DSettingsBuilder Toucan::ShowLines3D(const std::string& name, const Toucan::Buffer<Toucan::LineVertex3D>& lines_buffer) {
	validate_initialized(ShowLines3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(ShowLines3D)
	auto& current_figure = *context.current_figure_3d;
	
	auto& current_element = get_or_create_element_3d(current_figure, name, Toucan::ElementType3D::Line3D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	const size_t data_buffer_size = sizeof(Toucan::LineVertex3D)*lines_buffer.number_of_elements;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, lines_buffer.data_ptr, data_buffer_size);
	
	current_element.line_3d_metadata.number_of_line_vertices = lines_buffer.number_of_elements;
	return Toucan::ShowLines3DSettingsBuilder(&current_element.line_3d_metadata.settings);
}

Toucan::ShowPrimitives3DSettingsBuilder Toucan::ShowPrimitives3D(const std::string& name, const Toucan::Buffer<Toucan::Primitive3D>& primitives_buffer) {
	validate_initialized(ShowPrimitives3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(ShowPrimitives3D)
	auto& current_figure = *context.current_figure_3d;
	
	auto& current_element = get_or_create_element_3d(current_figure, name, Toucan::ElementType3D::Primitive3D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	const size_t data_buffer_size = sizeof(Toucan::Primitive3D)*primitives_buffer.number_of_elements;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, primitives_buffer.data_ptr, data_buffer_size);
	
	current_element.primitive_3d_metadata.number_of_primitives = primitives_buffer.number_of_elements;
	return Toucan::ShowPrimitives3DSettingsBuilder(&current_element.primitive_3d_metadata.settings);
}

Toucan::InputSettingsBuilder Toucan::BeginInputWindow(const std::string& name) {
	validate_initialized(BeginInputWindow)
	auto& toucan_context = * toucan_context_ptr;
	validate_inactive_input_window(BeginFigure3D)
	
	Toucan::InputWindow* input_window_ptr = nullptr;
	for (auto& input_window : toucan_context.input_windows) { // Does the InputWindow already exists
		if (input_window.name == name) {
			input_window_ptr = &input_window;
			break;
		}
	}
	
	if (input_window_ptr == nullptr) { // Do we need to create a new InputWindow
		auto& input_window = toucan_context.input_windows.emplace_back();
		input_window.name = name;
		input_window_ptr = &input_window;
	}
	
	input_window_ptr->mutex.lock();
	
	toucan_context.current_input_window = input_window_ptr;
	return Toucan::InputSettingsBuilder(&input_window_ptr->settings);
}

void Toucan::EndInputWindow() {
	validate_initialized(BeginInputWindow)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(BeginInputWindow)
	
	toucan_context.current_input_window->mutex.unlock();
	toucan_context.current_input_window = nullptr;
}

Toucan::ShowButtonsSettingsBuilder Toucan::ShowButton(const std::string& name) {
	validate_initialized(ShowButton)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowButton)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::BUTTON);
	
	if (current_element.show_button_metadata.number_of_click_events > 0) {
		current_element.show_button_metadata.number_of_click_events--;
		return Toucan::ShowButtonsSettingsBuilder(true, &current_element.show_button_metadata.settings);
	} else {
		return Toucan::ShowButtonsSettingsBuilder(false, &current_element.show_button_metadata.settings);
	}
}

Toucan::ShowCheckboxSettingsBuilder Toucan::ShowCheckbox(const std::string& name, bool& value) {
	validate_initialized(ShowSliderFloat)
	auto &toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto &current_input_window = *toucan_context.current_input_window;
	
	auto &current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::CHECKBOX);
	
	if (current_element.show_checkbox_metadata.value_changed) {
		value = current_element.show_checkbox_metadata.value;
		current_element.show_checkbox_metadata.value_changed = false;
		return Toucan::ShowCheckboxSettingsBuilder(true, &current_element.show_checkbox_metadata.settings);
	} else {
		current_element.show_checkbox_metadata.value = value;
		return Toucan::ShowCheckboxSettingsBuilder(false, &current_element.show_checkbox_metadata.settings);
	}
}

Toucan::ShowSliderFloatSettingsBuilder Toucan::ShowSliderFloat(const std::string& name, float& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_FLOAT);
	
	if (current_element.show_slider_float_metadata.value_changed) {
		value = current_element.show_slider_float_metadata.value;
		current_element.show_slider_float_metadata.value_changed = false;
		return Toucan::ShowSliderFloatSettingsBuilder(true, &current_element.show_slider_float_metadata.settings);
	} else {
		current_element.show_slider_float_metadata.value = value;
		return Toucan::ShowSliderFloatSettingsBuilder(false, &current_element.show_slider_float_metadata.settings);
	}
}

Toucan::ShowSliderFloatSettingsBuilder Toucan::ShowSliderFloat2(const std::string& name, Vector2f& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_FLOAT2);
	
	if (current_element.show_slider_float2_metadata.value_changed) {
		value = current_element.show_slider_float2_metadata.value;
		current_element.show_slider_float2_metadata.value_changed = false;
		return Toucan::ShowSliderFloatSettingsBuilder(true, &current_element.show_slider_float2_metadata.settings);
	} else {
		current_element.show_slider_float2_metadata.value = value;
		return Toucan::ShowSliderFloatSettingsBuilder(false, &current_element.show_slider_float2_metadata.settings);
	}
}

Toucan::ShowSliderFloatSettingsBuilder Toucan::ShowSliderFloat3(const std::string& name, Vector3f& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_FLOAT3);
	
	if (current_element.show_slider_float3_metadata.value_changed) {
		value = current_element.show_slider_float3_metadata.value;
		current_element.show_slider_float3_metadata.value_changed = false;
		return Toucan::ShowSliderFloatSettingsBuilder(true, &current_element.show_slider_float3_metadata.settings);
	} else {
		current_element.show_slider_float3_metadata.value = value;
		return Toucan::ShowSliderFloatSettingsBuilder(false, &current_element.show_slider_float3_metadata.settings);
	}
}

Toucan::ShowSliderFloatSettingsBuilder Toucan::ShowSliderFloat4(const std::string& name, Vector4f& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_FLOAT4);
	
	if (current_element.show_slider_float4_metadata.value_changed) {
		value = current_element.show_slider_float4_metadata.value;
		current_element.show_slider_float4_metadata.value_changed = false;
		return Toucan::ShowSliderFloatSettingsBuilder(true, &current_element.show_slider_float4_metadata.settings);
	} else {
		current_element.show_slider_float4_metadata.value = value;
		return Toucan::ShowSliderFloatSettingsBuilder(false, &current_element.show_slider_float4_metadata.settings);
	}
}

Toucan::ShowSliderIntSettingsBuilder Toucan::ShowSliderInt(const std::string& name, int& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_INT);
	
	if (current_element.show_slider_int_metadata.value_changed) {
		value = current_element.show_slider_int_metadata.value;
		current_element.show_slider_int_metadata.value_changed = false;
		return Toucan::ShowSliderIntSettingsBuilder(true, &current_element.show_slider_int_metadata.settings);
	} else {
		current_element.show_slider_int_metadata.value = value;
		return Toucan::ShowSliderIntSettingsBuilder(false, &current_element.show_slider_int_metadata.settings);
	}
}

Toucan::ShowSliderIntSettingsBuilder Toucan::ShowSliderInt2(const std::string& name, Vector2i& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_INT2);
	
	if (current_element.show_slider_int2_metadata.value_changed) {
		value = current_element.show_slider_int2_metadata.value;
		current_element.show_slider_int2_metadata.value_changed = false;
		return Toucan::ShowSliderIntSettingsBuilder(true, &current_element.show_slider_int2_metadata.settings);
	} else {
		current_element.show_slider_int2_metadata.value = value;
		return Toucan::ShowSliderIntSettingsBuilder(false, &current_element.show_slider_int2_metadata.settings);
	}
}

Toucan::ShowSliderIntSettingsBuilder Toucan::ShowSliderInt3(const std::string& name, Vector3i& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_INT3);
	
	if (current_element.show_slider_int3_metadata.value_changed) {
		value = current_element.show_slider_int3_metadata.value;
		current_element.show_slider_int3_metadata.value_changed = false;
		return Toucan::ShowSliderIntSettingsBuilder(true, &current_element.show_slider_int3_metadata.settings);
	} else {
		current_element.show_slider_int3_metadata.value = value;
		return Toucan::ShowSliderIntSettingsBuilder(false, &current_element.show_slider_int3_metadata.settings);
	}
}

Toucan::ShowSliderIntSettingsBuilder Toucan::ShowSliderInt4(const std::string& name, Vector4i& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_INT4);
	
	if (current_element.show_slider_int4_metadata.value_changed) {
		value = current_element.show_slider_int4_metadata.value;
		current_element.show_slider_int4_metadata.value_changed = false;
		return Toucan::ShowSliderIntSettingsBuilder(true, &current_element.show_slider_int4_metadata.settings);
	} else {
		current_element.show_slider_int4_metadata.value = value;
		return Toucan::ShowSliderIntSettingsBuilder(false, &current_element.show_slider_int4_metadata.settings);
	}
}

Toucan::ShowColorPickerSettingsBuilder Toucan::ShowColorPicker(const std::string& name, Color& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::COLOR_PICKER);
	
	if (current_element.show_color_picker_metadata.value_changed) {
		value = current_element.show_color_picker_metadata.value;
		current_element.show_color_picker_metadata.value_changed = false;
		return Toucan::ShowColorPickerSettingsBuilder(true, &current_element.show_color_picker_metadata.settings);
	} else {
		current_element.show_color_picker_metadata.value = value;
		return Toucan::ShowColorPickerSettingsBuilder(false, &current_element.show_color_picker_metadata.settings);
	}
}

void render_loop(Toucan::ToucanSettings settings) {
	
	glfwSetErrorCallback(glfw_error_callback);
	
	if (glfwInit() != GLFW_TRUE) {
		glfwTerminate();
		throw std::runtime_error("Toucan error! Unable to initialize GLFW.\n");
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, settings.resizeable ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_FLOATING, settings.floating ? GLFW_TRUE : GLFW_FALSE);
	
	toucan_context_ptr->window_ptr = glfwCreateWindow(settings.width, settings.height, "Toucan", nullptr, nullptr);
	
	if (toucan_context_ptr->window_ptr == nullptr) {
		glfwTerminate();
		throw std::runtime_error("Toucan error! Unable to create GLFW Window.\n");
	}
	
	glfwSetWindowCloseCallback(toucan_context_ptr->window_ptr, glfw_window_close_callback);
	glfwMakeContextCurrent(toucan_context_ptr->window_ptr);
	
	glfwSwapInterval(1);
	
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		glfwTerminate();
		throw std::runtime_error("Toucan error! Unable to load OpenGL.\n");
	}
	
#if !NDEBUG
	glad_set_post_callback(post_call_callback);
#endif
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(toucan_context_ptr->window_ptr, true);
	constexpr auto imgui_glsl_version = "#version 330 core";
	ImGui_ImplOpenGL3_Init(imgui_glsl_version);
	
	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	
	// Load renderdoc
#if !NDEBUG
	if (void* mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD )) {
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
		RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_1, (void**)&toucan_context_ptr->rdoc_api);
	}
#endif
	
	toucan_context_ptr->initialized_cv.notify_all();
	
	auto& imgui_style = ImGui::GetStyle();
	imgui_style.WindowMinSize = ImVec2(200.0f, 200.0f);
	
	while (toucan_context_ptr->should_render and not glfwWindowShouldClose(toucan_context_ptr->window_ptr)) {
		const auto frame_start = std::chrono::steady_clock::now();
		
		glfwPollEvents();
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		glClearColor(.2f, .2f, .2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // NOLINT
		
		ImGui::DockSpaceOverViewport();
		
		Toucan::draw_figure_2d_list(toucan_context_ptr->figures_2d, *toucan_context_ptr);
		
		// Draw all Figure3Ds
		for (auto& figure_3d : toucan_context_ptr->figures_3d) {
			//ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 200.0f), ImVec2(-1.0f, -1.0f));
			if (ImGui::Begin(figure_3d.name.c_str())) {
				std::unique_lock lock(figure_3d.mutex);
				
				ImGuiWindow* window = ImGui::GetCurrentWindow();
				if (window->SkipItems) {
					continue;
				}
				
				auto* window_draw_list = ImGui::GetWindowDrawList();
				//auto& style = ImGui::GetStyle();
				
				// ***** Compute rects *****
				const ImVec2 window_local_content_min = ImGui::GetWindowContentRegionMin();
				const ImVec2 window_local_content_max = ImGui::GetWindowContentRegionMax();
				const ImVec2 window_global_pos = ImGui::GetWindowPos();
				const ImRect window_local_content_rect(window_local_content_min, window_local_content_max);
				
				const ImVec2 window_global_content_min(window_local_content_min.x + window_global_pos.x, window_local_content_min.y + window_global_pos.y);
				const ImVec2 window_global_content_max(window_local_content_max.x + window_global_pos.x, window_local_content_max.y + window_global_pos.y);
				const ImRect window_global_context_rect(window_global_content_min, window_global_content_max);
				
				const Toucan::Vector2i figure_draw_size(
						static_cast<int>(std::lround(window_global_content_max.x - window_global_content_min.x)),
						static_cast<int>(std::lround(window_global_content_max.y - window_global_content_min.y))
				);
				
				// ***** Input *****
				
				// capture scroll with child region
				ImGui::BeginChild(figure_3d.name.c_str());
				
				bool view_was_changed = false;
				
				bool plot_hovered, plot_held;
				const ImGuiID plot_id = window->GetID("plot");
				ImGui::ButtonBehavior(window_global_context_rect, plot_id, &plot_hovered, &plot_held, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
				
				
				if (plot_held) {
					if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
						figure_3d.camera.orbit(-0.005f*Toucan::Vector2f(io.MouseDelta.x, io.MouseDelta.y));
						view_was_changed = true;
					}
					if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
						figure_3d.camera.move(0.001f*Toucan::Vector2f(io.MouseDelta.x, io.MouseDelta.y));
						view_was_changed = true;
					}
				}
				
				if (io.MouseWheel != 0.0f and plot_hovered) {
					figure_3d.camera.change_distance(-0.25f*io.MouseWheel);
					view_was_changed = true;
				}
				
				// ***** Drawing *****
				const bool framebuffer_was_updated = Toucan::update_framebuffer_3d(figure_3d, figure_draw_size);
				const bool elements_has_new_data = std::any_of(
						figure_3d.elements.cbegin(), figure_3d.elements.cend(),
						[](const Toucan::Element3D& element) { return element.data_buffer_ptr != nullptr; }
				);
				
				if (view_was_changed or framebuffer_was_updated or elements_has_new_data) {
					if(toucan_context_ptr->rdoc_api) toucan_context_ptr->rdoc_api->StartFrameCapture(nullptr, nullptr);
					glBindFramebuffer(GL_FRAMEBUFFER, figure_3d.framebuffer);
					glViewport(0, 0, figure_3d.framebuffer_size.x(), figure_3d.framebuffer_size.y());
					
					glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
					glEnable(GL_DEPTH_TEST);
					glDepthFunc(GL_LEQUAL);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // NOLINT
					
					const Toucan::Matrix4f world_to_camera_matrix = figure_3d.camera.get_pose().inverse().transformation_matrix();
					const Toucan::Matrix4f orientation_and_handedness_matrix = create_3d_orientation_and_handedness_matrix(figure_3d.settings.orientation, figure_3d.settings.handedness);
					
					const auto near_clip = figure_3d.settings.near_clip;
					const auto far_clip = figure_3d.settings.far_clip;
					const Toucan::Matrix4f projection_matrix = create_3d_projection_matrix<float>(near_clip, far_clip, 1024, figure_draw_size);
					
					for (auto& element : figure_3d.elements) {
						const auto model_to_world_matrix = element.pose.transformation_matrix();
						Toucan::draw_element_3d(element, model_to_world_matrix, orientation_and_handedness_matrix, world_to_camera_matrix, projection_matrix, toucan_context_ptr);
					}
					
					if (figure_3d.settings.gizmo_enabled) {
						Toucan::draw_axis_gizmo_3d(figure_3d.camera.get_orbit_pose(100.0f).inverse(), figure_draw_size, orientation_and_handedness_matrix, toucan_context_ptr);
					}
					
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					if(toucan_context_ptr->rdoc_api) toucan_context_ptr->rdoc_api->EndFrameCapture(nullptr, nullptr);
				}
				
				window_draw_list->AddImage(reinterpret_cast<void*>(figure_3d.framebuffer_color_texture), window_global_content_min, window_global_content_max);
				
				ImGui::EndChild();
			}
			ImGui::End();
		}
		
		// Draw all InputWindows
		for (auto& input_window : toucan_context_ptr->input_windows) {
			if (ImGui::Begin(input_window.name.c_str())) {
				std::unique_lock lock(input_window.mutex);
				
				for (auto& input_element : input_window.elements) {
					switch (input_element.type) {
						case Toucan::ElementInputType::BUTTON : {
							if (ImGui::Button(input_element.name.c_str())) {
								input_element.show_button_metadata.number_of_click_events++;
							}
						} break;
						case Toucan::ElementInputType::CHECKBOX : {
							bool* value_ptr = &input_element.show_checkbox_metadata.value;
							
							if (ImGui::Checkbox(input_element.name.c_str(), value_ptr)) {
								input_element.show_checkbox_metadata.value_changed = true;
							}
						} break;
						case Toucan::ElementInputType::SLIDER_FLOAT : {
							float* value_ptr = &input_element.show_slider_float_metadata.value;
							const float min_value = input_element.show_slider_float_metadata.settings.min_value;
							const float max_value = input_element.show_slider_float_metadata.settings.max_value;
							
							if (ImGui::SliderFloat(input_element.name.c_str(), value_ptr, min_value, max_value)) {
								input_element.show_slider_float_metadata.value_changed = true;
							}
						} break;
						case Toucan::ElementInputType::SLIDER_FLOAT2 : {
							float* value_ptr = input_element.show_slider_float2_metadata.value.data();
							const float min_value = input_element.show_slider_float2_metadata.settings.min_value;
							const float max_value = input_element.show_slider_float2_metadata.settings.max_value;
							
							if (ImGui::SliderFloat2(input_element.name.c_str(), value_ptr, min_value, max_value)) {
								input_element.show_slider_float2_metadata.value_changed = true;
							}
						} break;
						case Toucan::ElementInputType::SLIDER_FLOAT3 : {
							float* value_ptr = input_element.show_slider_float3_metadata.value.data();
							const float min_value = input_element.show_slider_float3_metadata.settings.min_value;
							const float max_value = input_element.show_slider_float3_metadata.settings.max_value;
							
							if (ImGui::SliderFloat3(input_element.name.c_str(), value_ptr, min_value, max_value)) {
								input_element.show_slider_float3_metadata.value_changed = true;
							}
						} break;
						case Toucan::ElementInputType::SLIDER_FLOAT4 : {
							float* value_ptr = input_element.show_slider_float4_metadata.value.data();
							const float min_value = input_element.show_slider_float4_metadata.settings.min_value;
							const float max_value = input_element.show_slider_float4_metadata.settings.max_value;
							
							if (ImGui::SliderFloat4(input_element.name.c_str(), value_ptr, min_value, max_value)) {
								input_element.show_slider_float4_metadata.value_changed = true;
							}
						} break;
						case Toucan::ElementInputType::SLIDER_INT : {
							int* value_ptr = &input_element.show_slider_int_metadata.value;
							const int min_value = input_element.show_slider_int_metadata.settings.min_value;
							const int max_value = input_element.show_slider_int_metadata.settings.max_value;
							
							if (ImGui::SliderInt(input_element.name.c_str(), value_ptr, min_value, max_value)) {
								input_element.show_slider_int_metadata.value_changed = true;
							}
						} break;
						case Toucan::ElementInputType::SLIDER_INT2 : {
							int* value_ptr = input_element.show_slider_int2_metadata.value.data();
							const int min_value = input_element.show_slider_int2_metadata.settings.min_value;
							const int max_value = input_element.show_slider_int2_metadata.settings.max_value;
							
							if (ImGui::SliderInt2(input_element.name.c_str(), value_ptr, min_value, max_value)) {
								input_element.show_slider_int2_metadata.value_changed = true;
							}
						} break;
						case Toucan::ElementInputType::SLIDER_INT3 : {
							int* value_ptr = input_element.show_slider_int3_metadata.value.data();
							const int min_value = input_element.show_slider_int3_metadata.settings.min_value;
							const int max_value = input_element.show_slider_int3_metadata.settings.max_value;
							
							if (ImGui::SliderInt3(input_element.name.c_str(), value_ptr, min_value, max_value)) {
								input_element.show_slider_int3_metadata.value_changed = true;
							}
						} break;
						case Toucan::ElementInputType::SLIDER_INT4 : {
							int* value_ptr = input_element.show_slider_int4_metadata.value.data();
							const int min_value = input_element.show_slider_int4_metadata.settings.min_value;
							const int max_value = input_element.show_slider_int4_metadata.settings.max_value;
							
							if (ImGui::SliderInt4(input_element.name.c_str(), value_ptr, min_value, max_value)) {
								input_element.show_slider_int4_metadata.value_changed = true;
							}
						} break;
						case Toucan::ElementInputType::COLOR_PICKER : {
							float* value_ptr = &input_element.show_color_picker_metadata.value.r;
							
							if (ImGui::ColorEdit3(input_element.name.c_str(), value_ptr)) {
								input_element.show_color_picker_metadata.value_changed = true;
							}
						} break;
					}
				}
			}
			ImGui::End();
		}
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(toucan_context_ptr->window_ptr);
		
		const auto frame_end = std::chrono::steady_clock::now();
		const auto current_frame_duration = frame_end - frame_start;
		
		using namespace std::chrono_literals;
		const auto min_frame_duration = 1000ms / settings.max_frames_per_second;
		
		if (current_frame_duration < min_frame_duration) {
			std::this_thread::sleep_for(min_frame_duration - current_frame_duration);
		}
	}
	
	ImGui::DestroyContext();
	glfwTerminate();
	
	toucan_context_ptr->window_open = false;
	toucan_context_ptr->window_close_cv.notify_all();
}

static void glfw_error_callback(int error, const char* description) {
	std::ostringstream ss;
	ss << "GLFW error! (" << error << "): " << description << '\n';
	throw std::runtime_error(ss.str());
}

static void glfw_window_close_callback(GLFWwindow* window) {
	toucan_context_ptr->window_open = false;
}
