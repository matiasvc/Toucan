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
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Toucan/DataTypes.h>

#include "util/GLDebug.h"
#include "internal.h"
#include "render.h"
#include "Utils.h"
#include "util/tick_number.h"
#include "gl/projection.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <renderdoc/renderdoc.h>
#include <dlfcn.h>

Toucan::ToucanContext* toucan_context_ptr = nullptr;

// Forward declerations
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
if (toucan_context_ptr->current_input_window == nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called without an active Figure2D. Did you forget to call 'Toucan::BeginFigure2D'?"); }

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
		
		Toucan::Element2D new_element_2d = {};
		new_element_2d.name = name;
		new_element_2d.draw_layer = draw_layer;
		new_element_2d.type = type;
		
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
		
		Toucan::Element3D new_element_3d = {};
		new_element_3d.name = name;
		new_element_3d.type = type;
		
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


void Toucan::BeginFigure2D(const std::string& name, const Figure2DSettings& settings) {
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
	
	figure_2d_ptr->settings = settings;
	toucan_context.current_figure_2d = figure_2d_ptr;
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

void Toucan::ShowLinePlot2D(const std::string& name, const Toucan::Buffer<Toucan::Vector2f>& line_buffer, int draw_layer, const ShowLinePlot2DSettings& settings) {
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
	current_element.line_plot_2d_metadata.settings = settings;
}

void Toucan::ShowPoints2D(const std::string& name, const Buffer <Point2D>& points_buffer, int draw_layer, const ShowPoints2DSettings& settings) {
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
	current_element.point_2d_metadata.settings = settings;
}


void Toucan::ShowImage2D(const std::string& name, const Image2D& image, int draw_layer, const ShowImage2DSettings& settings) {
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
	current_element.image_2d_metadata.settings = settings;
}

void Toucan::BeginFigure3D(const std::string& name, const Toucan::Figure3DSettings& settings) {
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
		Toucan::Element3D grid_element = {};
		grid_element.name = "Grid";
		grid_element.type = ElementType3D::Grid3D;
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
	
	figure_3d_ptr->settings = settings;
	toucan_context.current_figure_3d = figure_3d_ptr;
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

void Toucan::ShowAxis3D(const std::string& name, const ShowAxis3DSettings& settings) {
	validate_initialized(ShowAxis3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(ShowAxis3D)
	auto& current_figure = *context.current_figure_3d;
	
	auto& current_element = get_or_create_element_3d(current_figure, name, Toucan::ElementType3D::Axis3D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	current_element.data_buffer_ptr = reinterpret_cast<void*>(1);
	current_element.axis_3d_metadata.settings = settings;
}

void Toucan::ShowPoints3D(const std::string& name, const Toucan::Buffer<Toucan::Point3D>& points_buffer, const ShowPoints3DSettings& settings) {
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
	current_element.point_3d_metadata.settings = settings;
}


void Toucan::ShowLines3D(const std::string& name, const Toucan::Buffer<Toucan::LineVertex3D>& lines_buffer, const ShowLines3DSettings& settings) {
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
	current_element.line_3d_metadata.settings = settings;
}

void Toucan::ShowPrimitives3D(const std::string& name, const Toucan::Buffer<Toucan::Primitive3D>& primitives_buffer, const ShowPrimitives3DSettings& settings) {
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
	current_element.primitive_3d_metadata.settings = settings;
}

void Toucan::BeginInputWindow(const std::string& name, const InputSettings& settings) {
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
	
	input_window_ptr->settings = settings;
	toucan_context.current_input_window = input_window_ptr;
}

void Toucan::EndInputWindow() {
	validate_initialized(BeginInputWindow)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(BeginInputWindow)
	
	toucan_context.current_input_window->mutex.unlock();
	toucan_context.current_input_window = nullptr;
}

bool Toucan::ShowButton(const std::string& name, const ShowButtonSettings& settings) {
	validate_initialized(ShowButton)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowButton)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::BUTTON);
	
	current_element.show_button_metadata.settings = settings;
	if (current_element.show_button_metadata.number_of_click_events > 0) {
		current_element.show_button_metadata.number_of_click_events--;
		return true;
	} else {
		return false;
	}
}

bool Toucan::ShowCheckbox(const std::string& name, bool& value, const ShowCheckboxSettings& settings) {
	validate_initialized(ShowSliderFloat)
	auto &toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto &current_input_window = *toucan_context.current_input_window;
	
	auto &current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::CHECKBOX);
	
	current_element.show_checkbox_metadata.settings = settings;
	if (current_element.show_checkbox_metadata.value_changed) {
		value = current_element.show_checkbox_metadata.value;
		current_element.show_checkbox_metadata.value_changed = false;
		return true;
	} else {
		current_element.show_checkbox_metadata.value = value;
		return false;
	}
}

bool Toucan::ShowSliderFloat(const std::string& name, float& value, const ShowSliderFloatSettings& settings) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_FLOAT);
	
	current_element.show_slider_float_metadata.settings = settings;
	if (current_element.show_slider_float_metadata.value_changed) {
		value = current_element.show_slider_float_metadata.value;
		current_element.show_slider_float_metadata.value_changed = false;
		return true;
	} else {
		current_element.show_slider_float_metadata.value = value;
		return false;
	}
}

bool Toucan::ShowSliderFloat2(const std::string& name, Vector2f& value, const ShowSliderFloatSettings& settings) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_FLOAT2);
	
	current_element.show_slider_float2_metadata.settings = settings;
	if (current_element.show_slider_float2_metadata.value_changed) {
		value = current_element.show_slider_float2_metadata.value;
		current_element.show_slider_float2_metadata.value_changed = false;
		return true;
	} else {
		current_element.show_slider_float2_metadata.value = value;
		return false;
	}
}

bool Toucan::ShowSliderFloat3(const std::string& name, Vector3f& value, const ShowSliderFloatSettings& settings) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_FLOAT3);
	
	current_element.show_slider_float3_metadata.settings = settings;
	if (current_element.show_slider_float3_metadata.value_changed) {
		value = current_element.show_slider_float3_metadata.value;
		current_element.show_slider_float3_metadata.value_changed = false;
		return true;
	} else {
		current_element.show_slider_float3_metadata.value = value;
		return false;
	}
}

bool Toucan::ShowSliderFloat4(const std::string& name, Vector4f& value, const ShowSliderFloatSettings& settings) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_FLOAT4);
	
	current_element.show_slider_float4_metadata.settings = settings;
	if (current_element.show_slider_float4_metadata.value_changed) {
		value = current_element.show_slider_float4_metadata.value;
		current_element.show_slider_float4_metadata.value_changed = false;
		return true;
	} else {
		current_element.show_slider_float4_metadata.value = value;
		return false;
	}
}

bool Toucan::ShowSliderInt(const std::string& name, int& value, const ShowSliderIntSettings& settings) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_INT);
	
	current_element.show_slider_int_metadata.settings = settings;
	if (current_element.show_slider_int_metadata.value_changed) {
		value = current_element.show_slider_int_metadata.value;
		current_element.show_slider_int_metadata.value_changed = false;
		return true;
	} else {
		current_element.show_slider_int_metadata.value = value;
		return false;
	}
}

bool Toucan::ShowSliderInt2(const std::string& name, Vector2i& value, const ShowSliderIntSettings& settings) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_INT2);
	
	current_element.show_slider_int2_metadata.settings = settings;
	if (current_element.show_slider_int2_metadata.value_changed) {
		value = current_element.show_slider_int2_metadata.value;
		current_element.show_slider_int2_metadata.value_changed = false;
		return true;
	} else {
		current_element.show_slider_int2_metadata.value = value;
		return false;
	}
}

bool Toucan::ShowSliderInt3(const std::string& name, Vector3i& value, const ShowSliderIntSettings& settings) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_INT3);
	
	current_element.show_slider_int3_metadata.settings = settings;
	if (current_element.show_slider_int3_metadata.value_changed) {
		value = current_element.show_slider_int3_metadata.value;
		current_element.show_slider_int3_metadata.value_changed = false;
		return true;
	} else {
		current_element.show_slider_int3_metadata.value = value;
		return false;
	}
}

bool Toucan::ShowSliderInt4(const std::string& name, Vector4i& value, const ShowSliderIntSettings& settings) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::SLIDER_INT4);
	
	current_element.show_slider_int4_metadata.settings = settings;
	if (current_element.show_slider_int4_metadata.value_changed) {
		value = current_element.show_slider_int4_metadata.value;
		current_element.show_slider_int4_metadata.value_changed = false;
		return true;
	} else {
		current_element.show_slider_int4_metadata.value = value;
		return false;
	}
}

bool Toucan::ShowColorPicker(const std::string& name, Color& value, const ShowColorPickerSettings& settings) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = * toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, Toucan::ElementInputType::COLOR_PICKER);
	
	current_element.show_color_picker_metadata.settings = settings;
	if (current_element.show_color_picker_metadata.value_changed) {
		value = current_element.show_color_picker_metadata.value;
		current_element.show_color_picker_metadata.value_changed = false;
		return true;
	} else {
		current_element.show_color_picker_metadata.value = value;
		return false;
	}
}

Toucan::Rectangle get_lineplot_2d_data_bounds(const Toucan::Element2D& element_2d, const Toucan::RigidTransform2Df& local_transform) {
	assert(element_2d.type == Toucan::ElementType2D::LinePlot2D);
	
	const Toucan::ScaledTransform2Df& data_transform = element_2d.line_plot_2d_metadata.settings.scaled_transform;
	auto* point_2d_data_ptr = reinterpret_cast<Toucan::Vector2f*>(element_2d.data_buffer_ptr);
	
	auto min_x =  std::numeric_limits<float>::infinity();
	auto max_x = -std::numeric_limits<float>::infinity();
	auto min_y =  std::numeric_limits<float>::infinity();
	auto max_y = -std::numeric_limits<float>::infinity();
	
	for (int point_2d_data_index = 0; point_2d_data_index < element_2d.line_plot_2d_metadata.number_of_points; ++point_2d_data_index) {
		const Toucan::Vector2f local_point = point_2d_data_ptr[point_2d_data_index];
		const Toucan::Vector2f global_point = local_transform * (data_transform * local_point);
		const auto& x = global_point.x();
		const auto& y = global_point.y();
		
		min_x = std::min(min_x, x);
		max_x = std::max(max_x, x);
		min_y = std::min(min_y, y);
		max_y = std::max(max_y, y);
	}
	
	assert(min_x <= max_x and min_y <= max_y);
	assert(not std::isinf(min_x) and not std::isinf(max_x) and not std::isinf(min_y) and not std::isinf(max_y));
	
	return Toucan::Rectangle(Toucan::Vector2f(min_x, min_y), Toucan::Vector2f(max_x, max_y));
}

Toucan::Rectangle get_point_2d_data_bounds(const Toucan::Element2D& element_2d, const Toucan::RigidTransform2Df& local_transform) {
	assert(element_2d.type == Toucan::ElementType2D::Point2D);
	
	const Toucan::ScaledTransform2Df& data_transform = element_2d.point_2d_metadata.settings.scaled_transform;
	auto* point_2d_data_ptr = reinterpret_cast<Toucan::Point2D*>(element_2d.data_buffer_ptr);
	
	auto min_x =  std::numeric_limits<float>::infinity();
	auto max_x = -std::numeric_limits<float>::infinity();
	auto min_y =  std::numeric_limits<float>::infinity();
	auto max_y = -std::numeric_limits<float>::infinity();
	
	for (int point_2d_data_index = 0; point_2d_data_index < element_2d.point_2d_metadata.number_of_points; ++point_2d_data_index) {
		const Toucan::Vector2f local_point = point_2d_data_ptr[point_2d_data_index].position;
		const Toucan::Vector2f global_point = local_transform * (data_transform * local_point);
		const auto& x = global_point.x();
		const auto& y = global_point.y();
		
		min_x = std::min(min_x, x);
		max_x = std::max(max_x, x);
		min_y = std::min(min_y, y);
		max_y = std::max(max_y, y);
	}
	
	assert(min_x <= max_x and min_y <= max_y);
	assert(not std::isinf(min_x) and not std::isinf(max_x) and not std::isinf(min_y) and not std::isinf(max_y));
	
	return Toucan::Rectangle(Toucan::Vector2f(min_x, min_y), Toucan::Vector2f(max_x, max_y));
}

Toucan::Rectangle get_image_2d_data_bounds(const Toucan::Element2D& element_2d, const Toucan::RigidTransform2Df& local_transform) {
	assert(element_2d.type == Toucan::ElementType2D::Image2D);
	
	unsigned int image_draw_width = element_2d.image_2d_metadata.settings.image_display_width;
	if (image_draw_width == 0) { image_draw_width = element_2d.image_2d_metadata.width; }
	unsigned int image_draw_height = element_2d.image_2d_metadata.settings.image_display_height;
	if (image_draw_height == 0) { image_draw_height = element_2d.image_2d_metadata.height; }
	
	const std::array<Toucan::Vector2f, 4> image_corners = {
			local_transform * Toucan::Vector2f(0.0f, 0.0f),
			local_transform * Toucan::Vector2f(static_cast<float>(image_draw_width), 0.0f),
			local_transform * Toucan::Vector2f(0.0f, static_cast<float>(image_draw_height)),
			local_transform * Toucan::Vector2f(static_cast<float>(image_draw_width), static_cast<float>(image_draw_height))
	};
	
	auto min_x =  std::numeric_limits<float>::infinity();
	auto max_x = -std::numeric_limits<float>::infinity();
	auto min_y =  std::numeric_limits<float>::infinity();
	auto max_y = -std::numeric_limits<float>::infinity();
	
	
	for (const auto& image_corner : image_corners) {
		const auto& x = image_corner.x();
		const auto& y = image_corner.y();
		
		min_x = std::min(min_x, x);
		max_x = std::max(max_x, x);
		min_y = std::min(min_y, y);
		max_y = std::max(max_y, y);
	}
	
	assert(min_x <= max_x and min_y <= max_y);
	assert(not std::isinf(min_x) and not std::isinf(max_x) and not std::isinf(min_y) and not std::isinf(max_y));
	
	return Toucan::Rectangle(Toucan::Vector2f(min_x, min_y), Toucan::Vector2f(max_x, max_y));
}

void update_figure_2d_view_data(Toucan::Figure2D& figure_2d) {
	if (figure_2d.elements.empty()) { // There are no elements in the figure so set a reasonable default view
		figure_2d.view = Toucan::Rectangle(Toucan::Vector2f(-5.0f, -5.0f), Toucan::Vector2f(5.0f, 5.0f));
		return;
	}
	
	std::vector<Toucan::Rectangle> data_bounds_vec;
	data_bounds_vec.reserve(figure_2d.elements.size());
	
	for (auto& element_2d : figure_2d.elements) {
		
		if (element_2d.data_buffer_ptr == nullptr) { // Use view from cache
			data_bounds_vec.emplace_back(element_2d.data_bounds_cache);
		} else { // Compute updated view
			const Toucan::RigidTransform2Df& local_transform = element_2d.pose;
			
			Toucan::Rectangle data_bounds;
			
			switch (element_2d.type) {
				case Toucan::ElementType2D::LinePlot2D: {
					data_bounds = get_lineplot_2d_data_bounds(element_2d, local_transform);
				} break;
				case Toucan::ElementType2D::Point2D: {
					data_bounds = get_point_2d_data_bounds(element_2d, local_transform);
				} break;
				case Toucan::ElementType2D::Image2D: {
					data_bounds = get_image_2d_data_bounds(element_2d, local_transform);
				} break;
			}
			element_2d.data_bounds_cache = data_bounds;
			data_bounds_vec.emplace_back(data_bounds);
		}
	}
	
	Toucan::Rectangle figure_data_bounds = data_bounds_vec.front();
	
	for (const auto& data_bounds : data_bounds_vec) {
		figure_data_bounds = Toucan::get_union_of_rectangles(figure_data_bounds, data_bounds);
	}
	// TODO(Matias): Make buffer size user editable
	
	
	// In the case we only have a single value to base the view on in an axis, then we need to set a default view range in that axis.
	if (figure_data_bounds.width() <= std::numeric_limits<float>::epsilon()) {
		figure_data_bounds.min.x() -= 5.0f;
		figure_data_bounds.max.x() += 5.0f;
	}
	
	if (figure_data_bounds.height() <= std::numeric_limits<float>::epsilon()) {
		figure_data_bounds.min.y() -= 5.0f;
		figure_data_bounds.max.y() += 5.0f;
	}
	
	figure_2d.view = Toucan::get_resized_rectangle(figure_data_bounds, Toucan::Vector2f(0.05f*figure_data_bounds.width(), 0.05f*figure_data_bounds.height()));
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
	
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(toucan_context_ptr->window_ptr, true);
	constexpr auto imgui_glsl_version = "#version 330 core";
	ImGui_ImplOpenGL3_Init(imgui_glsl_version);
	
	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	
	// Load renderdoc
	if (void* mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD )) {
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_1, (void**)&toucan_context_ptr->rdoc_api);
		assert(ret == 1);
	}
	
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
		
		for (auto& figure_2d : toucan_context_ptr->figures_2d) {
			// TODO(Matias): This call seems to make the window size stuck. Possibly a ImGui bug.
			//ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 200.0f), ImVec2(-1.0f, -1.0f));
			if (ImGui::Begin(figure_2d.name.c_str())) {
				ImGuiWindow* window = ImGui::GetCurrentWindow();
				if (window->SkipItems) {
					continue;
				}
				
				auto* window_draw_list = ImGui::GetWindowDrawList();
				auto& style = ImGui::GetStyle();
				
				
				std::unique_lock lock(figure_2d.mutex);
				// ***** Compute rects *****
				
				// Get settings
				const float axis_x_size = figure_2d.settings.axis_x_size;
				const float axis_y_size = figure_2d.settings.axis_y_size;
				const float tick_width = figure_2d.settings.tick_width;
				const float plot_padding = figure_2d.settings.figure_padding;
				
				const ImVec2 window_local_content_min = ImGui::GetWindowContentRegionMin();
				const ImVec2 window_local_content_max = ImGui::GetWindowContentRegionMax();
				const ImVec2 window_global_pos = ImGui::GetWindowPos();
				const ImRect window_local_content_rect(window_local_content_min, window_local_content_max);
				
				const ImVec2 window_global_content_min(window_local_content_min.x + window_global_pos.x, window_local_content_min.y + window_global_pos.y);
				const ImVec2 window_global_content_max(window_local_content_max.x + window_global_pos.x, window_local_content_max.y + window_global_pos.y);
				
				const ImVec2 axis_y_min(window_global_content_min.x + plot_padding, window_global_content_min.y + plot_padding);
				const ImVec2 axis_y_max(axis_y_min.x + axis_y_size, window_global_content_max.y - (plot_padding + axis_x_size));
				const ImRect axis_y_rect(axis_y_min, axis_y_max);
				
				const ImVec2 axis_x_min(window_global_content_min.x + (plot_padding + axis_y_size), window_global_content_max.y - (plot_padding + axis_x_size));
				const ImVec2 axis_x_max(window_global_content_max.x - plot_padding, window_global_content_max.y - plot_padding);
				const ImRect axis_x_rect(axis_x_min, axis_x_max);
				
				const ImVec2 plot_min(axis_y_max.x, window_global_content_min.y + plot_padding);
				const ImVec2 plot_max(window_global_content_max.x - plot_padding, window_global_content_max.y - (plot_padding + axis_x_size));
				const ImRect plot_rect(plot_min, plot_max);
				
				const Toucan::Vector2i figure_draw_size(
						static_cast<int>(std::lround(window_global_content_max.x - window_global_content_min.x)),
						static_cast<int>(std::lround(window_global_content_max.y - window_global_content_min.y))
				);
				
				// ***** Input *****
				
				// capture scroll with child region
				ImGui::BeginChild(figure_2d.name.c_str());
				
				bool view_changed_this_frame = false;
				
				bool plot_hovered, plot_held;
				const ImGuiID plot_id = window->GetID("plot");
				ImGui::ButtonBehavior(plot_rect, plot_id, &plot_hovered, &plot_held);
				
				bool axis_x_hovered, axis_x_held;
				const ImGuiID axis_x_id = window->GetID("axis_x");
				ImGui::ButtonBehavior(axis_x_rect, axis_x_id, &axis_x_hovered, &axis_x_held);
				
				bool axis_y_hovered, axis_y_held;
				const ImGuiID axis_y_id = window->GetID("axis_y");
				ImGui::ButtonBehavior(axis_y_rect, axis_y_id, &axis_y_hovered, &axis_y_held);
				
				const auto scroll = static_cast<int>(io.MouseWheel);
				int scroll_x = 0;
				int scroll_y = 0;
				
				if (scroll != 0) {
					
					if (plot_hovered) {
						scroll_x = scroll;
						scroll_y = scroll;
						
						figure_2d.user_changed_view = true;
						view_changed_this_frame = true;
					} else if (axis_x_hovered) {
						scroll_x = scroll;
						
						figure_2d.user_changed_view = true;
						view_changed_this_frame = true;
					} else if (axis_y_hovered) {
						scroll_y = scroll;
						
						figure_2d.user_changed_view = true;
						view_changed_this_frame = true;
					}
				}
				
				constexpr float zoom_amount = 0.10f; // TODO: Make configurable for user
				if (scroll_x != 0) {
					const auto mouse_plot_value_x = Toucan::remap(io.MousePos.x, plot_min.x, plot_max.x,
					                                              figure_2d.view.min.x(), figure_2d.view.max.x());
					if (scroll > 0) {
						for (int scroll_index = 0; scroll_index < scroll; scroll_index++) {
							figure_2d.view.min.x() = (1.0f - zoom_amount) * figure_2d.view.min.x() + (zoom_amount) * mouse_plot_value_x;
							figure_2d.view.max.x() = (1.0f - zoom_amount) * figure_2d.view.max.x() + (zoom_amount) * mouse_plot_value_x;
						}
					} else {
						for (int scroll_index = 0; scroll_index > scroll; scroll_index--) {
							figure_2d.view.min.x() = (1.0f + zoom_amount) * figure_2d.view.min.x() + (-zoom_amount) * mouse_plot_value_x;
							figure_2d.view.max.x() = (1.0f + zoom_amount) * figure_2d.view.max.x() + (-zoom_amount) * mouse_plot_value_x;
						}
					}
				}
				
				if (scroll_y != 0) {
					float mouse_plot_value_y;
					
					if (figure_2d.settings.y_axis_direction == Toucan::YAxisDirection::UP) {
						mouse_plot_value_y = Toucan::remap(io.MousePos.y, plot_min.y, plot_max.y, figure_2d.view.max.y(), figure_2d.view.min.y());
					} else {
						mouse_plot_value_y = Toucan::remap(io.MousePos.y, plot_min.y, plot_max.y, figure_2d.view.min.y(), figure_2d.view.max.y());
					}
					if (scroll > 0) {
						for (int scroll_index = 0; scroll_index < scroll; scroll_index++) {
							figure_2d.view.min.y() = (1.0f - zoom_amount) * figure_2d.view.min.y() + (zoom_amount) * mouse_plot_value_y;
							figure_2d.view.max.y() = (1.0f - zoom_amount) * figure_2d.view.max.y() + (zoom_amount) * mouse_plot_value_y;
						}
					} else {
						for (int scroll_index = 0; scroll_index > scroll; scroll_index--) {
							figure_2d.view.min.y() = (1.0f + zoom_amount) * figure_2d.view.min.y() + (-zoom_amount) * mouse_plot_value_y;
							figure_2d.view.max.y() = (1.0f + zoom_amount) * figure_2d.view.max.y() + (-zoom_amount) * mouse_plot_value_y;
						}
					}
				}
				
				const float unit_per_pixel_x = figure_2d.view.width() / plot_rect.GetWidth();
				const float unit_per_pixel_y = figure_2d.view.height() / plot_rect.GetHeight();
				
				if (plot_held and ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
					const float delta_x = unit_per_pixel_x * static_cast<float>(io.MouseDelta.x);
					const float delta_y = unit_per_pixel_y * static_cast<float>(io.MouseDelta.y);
					
					figure_2d.view.min.x() -= delta_x;
					figure_2d.view.max.x() -= delta_x;
					if (figure_2d.settings.y_axis_direction == Toucan::YAxisDirection::UP) {
						figure_2d.view.min.y() += delta_y;
						figure_2d.view.max.y() += delta_y;
					} else {
						figure_2d.view.min.y() -= delta_y;
						figure_2d.view.max.y() -= delta_y;
					}
					figure_2d.user_changed_view = true;
					view_changed_this_frame = true;
				}
				
				if (plot_hovered and ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					ImGui::OpenPopup("popup");
				}
				
				if (ImGui::BeginPopup("popup")) {
					
					if (ImGui::Button("Reset view")) {
						figure_2d.user_changed_view = false;
						view_changed_this_frame = true;
						ImGui::CloseCurrentPopup();
					}
					
					ImGui::EndPopup();
				}
				
				// ***** Drawing *****
				
				// Check if framebuffer needs resizing or we received new data
				const bool framebuffer_was_updated = Toucan::update_framebuffer_2d(figure_2d, figure_draw_size);
				const bool elements_has_new_data = std::any_of(
						figure_2d.elements.cbegin(), figure_2d.elements.cend(),
						[](const Toucan::Element2D& element) { return element.data_buffer_ptr != nullptr; }
				);
				
				// Draw figure
				if (view_changed_this_frame or framebuffer_was_updated or elements_has_new_data) {
					if(toucan_context_ptr->rdoc_api) toucan_context_ptr->rdoc_api->StartFrameCapture(nullptr, nullptr);
					glBindFramebuffer(GL_FRAMEBUFFER, figure_2d.framebuffer);
					glViewport(0, 0, figure_2d.framebuffer_size.x(), figure_2d.framebuffer_size.y());
					
					glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
					glDisable(GL_DEPTH_TEST);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // NOLINT
					
					if (not figure_2d.user_changed_view) {
						update_figure_2d_view_data(figure_2d);
					}
					
					const Toucan::Matrix4f view_matrix = create_2d_view_matrix(figure_2d.view, figure_2d.settings.y_axis_direction);
					for (auto& element : figure_2d.elements) {
						const auto& model_to_world = element.pose.transformation_matrix_3d();
						Toucan::draw_element_2d(element, model_to_world, view_matrix, toucan_context_ptr);
					}
					
					glCheckError();
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					if(toucan_context_ptr->rdoc_api) toucan_context_ptr->rdoc_api->EndFrameCapture(nullptr, nullptr);
				}
				
				// Axis ticks
				const auto x_axis_from_value = figure_2d.view.min.x();
				const auto x_axis_to_value = figure_2d.view.max.x();
				
				constexpr float min_label_distance = 85.0f; // TODO(Matias): Compute min distance based on text size.
				
				assert(x_axis_from_value < x_axis_to_value);
				const int number_of_x_ticks = std::max(static_cast<int>(std::floor(axis_x_rect.GetWidth()/min_label_distance)), 2);
				const auto [x_tick_values, x_tick_strings] = get_axis_ticks(x_axis_from_value, x_axis_to_value, number_of_x_ticks);
				const auto x_ticks_position = Toucan::data_to_pixel(x_tick_values, x_axis_from_value, x_axis_to_value, axis_x_min.x, axis_x_max.x);
				
				const auto y_axis_from_value = figure_2d.view.min.y();
				const auto y_axis_to_value = figure_2d.view.max.y();
				
				assert(y_axis_from_value < y_axis_to_value);
				const int number_of_y_ticks = std::max(static_cast<int>(std::floor(axis_y_rect.GetHeight()/min_label_distance)), 2);
				const auto [y_ticks_values, y_tick_strings] = get_axis_ticks(y_axis_from_value, y_axis_to_value, number_of_y_ticks);
				std::vector<float> y_ticks_positions;
				if (figure_2d.settings.y_axis_direction == Toucan::YAxisDirection::UP) {
					y_ticks_positions = Toucan::data_to_pixel(y_ticks_values, y_axis_from_value, y_axis_to_value, axis_y_max.y, axis_y_min.y);
				} else {
					y_ticks_positions = Toucan::data_to_pixel(y_ticks_values, y_axis_from_value, y_axis_to_value, axis_y_min.y, axis_y_max.y);
				}
				
				// Background
				window_draw_list->AddRectFilled(axis_x_min, axis_x_max, ImGui::GetColorU32(style.Colors[ImGuiCol_PopupBg]), 3.0f);
				window_draw_list->AddRectFilled(axis_y_min, axis_y_max, ImGui::GetColorU32(style.Colors[ImGuiCol_PopupBg]), 3.0f);
				window_draw_list->AddRectFilled(plot_min, plot_max, ImGui::GetColorU32(style.Colors[ImGuiCol_PopupBg]), 3.0f);
				
				// Draw x axis
				window_draw_list->AddRect(axis_x_min, axis_x_max, ImGui::GetColorU32(style.Colors[ImGuiCol_Border]));
				window_draw_list->PushClipRect(axis_x_min, axis_x_max);
				
				for (size_t x_tick_index = 0; x_tick_index < x_tick_strings.size(); ++x_tick_index) {
					const auto x_tick_string = x_tick_strings.at(x_tick_index);
					const auto x_tick_position = std::round(x_ticks_position.at(x_tick_index));
					
					window_draw_list->AddLine(ImVec2(x_tick_position, axis_x_min.y), ImVec2(x_tick_position, axis_x_min.y + tick_width), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 1.0f);
					
					const ImVec2 text_size = ImGui::CalcTextSize(x_tick_string.c_str());
					window_draw_list->AddText(ImVec2(static_cast<float>(x_tick_position) - 0.5f * text_size.x, axis_x_min.y + 0.5f * axis_x_size), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), x_tick_string.c_str());
				}
				window_draw_list->PopClipRect();
				
				// Y-axis
				window_draw_list->AddRect(axis_y_min, axis_y_max, ImGui::GetColorU32(style.Colors[ImGuiCol_Border]));
				window_draw_list->PushClipRect(axis_y_min, axis_y_max);
				
				for (size_t y_tick_index = 0; y_tick_index < y_tick_strings.size(); ++y_tick_index) {
					const auto y_tick_string = y_tick_strings.at(y_tick_index);
					const auto y_tick_position = std::round(y_ticks_positions.at(y_tick_index));
					
					window_draw_list->AddLine(ImVec2(axis_y_max.x - tick_width, y_tick_position), ImVec2(axis_y_max.x, y_tick_position), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 1.0f);
					const ImVec2 text_size = ImGui::CalcTextSize(y_tick_string.c_str());
					const auto axis_y_width = axis_y_max.x - axis_y_min.x;
					window_draw_list->AddText(ImVec2(axis_y_min.x + axis_y_width - (tick_width + text_size.x + 3.0f), y_tick_position - 0.5f * text_size.y), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), y_tick_string.c_str());
				}
				
				window_draw_list->PopClipRect();
				
				// Plot
				window_draw_list->PushClipRect(plot_min, plot_max);
				
				for (size_t x_tick_index = 0; x_tick_index < x_tick_strings.size(); ++x_tick_index) {
					const auto x_tick_position = std::round(x_ticks_position.at(x_tick_index));
					window_draw_list->AddLine(ImVec2(x_tick_position, plot_min.y), ImVec2(x_tick_position, plot_max.y), ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.25f, 1.0f)), 1.0f);
				}
				for (size_t y_tick_index = 0; y_tick_index < y_tick_strings.size(); ++y_tick_index) {
					const auto y_tick_position = std::round(y_ticks_positions.at(y_tick_index));
					window_draw_list->AddLine(ImVec2(plot_min.x, y_tick_position), ImVec2(plot_max.x, y_tick_position), ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.25f, 1.0f)), 1.0f);
				}
				window_draw_list->AddImage(reinterpret_cast<void*>(figure_2d.framebuffer_color_texture), plot_min, plot_max);
				
				window_draw_list->AddRect(plot_min, plot_max, ImGui::GetColorU32(style.Colors[ImGuiCol_Border]), 0.0f, ImDrawCornerFlags_All, 1.0f);
				
				window_draw_list->PopClipRect();
				ImGui::EndChild();
			}
			ImGui::End();
		}
		
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
					glCheckError();
					if (figure_3d.settings.show_axis_gizmo) {
						Toucan::draw_axis_gizmo_3d(figure_3d.camera.get_orbit_pose(100.0f).inverse(), figure_draw_size, orientation_and_handedness_matrix, toucan_context_ptr);
					}
					glCheckError();
					
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
