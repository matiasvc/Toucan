#include <Toucan/Toucan.h>

#include <cstring>

#include "validate.h"

namespace Toucan {

Element2D& get_or_create_element_2d(Figure2D& figure, const std::string& name, int draw_layer, ElementType2D type) {
	// Does the Element2D object with that name already exist?
	Element2D* current_element_ptr = nullptr;
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
		
		Element2D new_element_2d(name, type, draw_layer);
		
		auto insertion_iterator = figure.elements.insert(element_iterator, std::move(new_element_2d));
		current_element_ptr = &(*insertion_iterator);
	}
	
	return *current_element_ptr;
}

Figure2DSettingsBuilder BeginFigure2D(const std::string& name) {
	validate_initialized(BeginFigure2D)
	auto& toucan_context = *toucan_context_ptr;
	validate_inactive_figure2d(BeginFigure2D)
	
	Figure2D* figure_2d_ptr = nullptr;
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
	return Figure2DSettingsBuilder(&figure_2d_ptr->settings);
}

void EndFigure2D() {
	validate_initialized(EndFigure2D)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_figure2d(EndFigure2D)
	
	auto& figure_2d = *toucan_context.current_figure_2d;
	figure_2d.pose_stack.clear();
	
	toucan_context.current_figure_2d->mutex.unlock();
	toucan_context.current_figure_2d = nullptr;
}

void PushPose2D(const RigidTransform2Df& pose) {
	validate_initialized(PushPose2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(PushPose2D)
	auto& current_figure = *context.current_figure_2d;
	
	const auto& parent_pose = current_figure.pose_stack.back();
	current_figure.pose_stack.emplace_back(parent_pose * pose);
}

void PopPose2D() {
	validate_initialized(PopPose2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(PopPose2D)
	auto& current_figure = *context.current_figure_2d;
	
	if (current_figure.pose_stack.size() <= 1) { throw std::runtime_error("Toucan error! 'PopPose2D' was called without a matching call to `PushPose2D`."); }
	
	current_figure.pose_stack.pop_back();
}

void ClearPose2D() {
	validate_initialized(PopPose2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(PopPose2D)
	auto& current_figure = *context.current_figure_2d;
	
	if (current_figure.pose_stack.size() <= 1) { throw std::runtime_error("Toucan error! 'ClearPose2D' was called without any matching call to `PushPose2D`."); }
	
	current_figure.pose_stack.clear();
	current_figure.pose_stack.emplace_back(); // Add identity pose back
}

ShowLinePlot2DSettingsBuilder ShowLinePlot2D(const std::string& name, const Buffer<Vector2f>& line_buffer, int draw_layer) {
	validate_initialized(PopPose2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(PopPose2D)
	auto& current_figure = *context.current_figure_2d;
	
	auto& current_element = get_or_create_element_2d(current_figure, name, draw_layer, ElementType2D::LinePlot2D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	const size_t data_buffer_size = sizeof(Vector2f) * line_buffer.number_of_elements;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, line_buffer.data_ptr, data_buffer_size);
	
	current_element.line_plot_2d_metadata.number_of_points = line_buffer.number_of_elements;
	return ShowLinePlot2DSettingsBuilder(&current_element.line_plot_2d_metadata.settings);
}

ShowPoints2DSettingsBuilder ShowPoints2D(const std::string& name, const Buffer<Point2D>& points_buffer, int draw_layer) {
	validate_initialized(ShowPoints2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(ShowPoints2D)
	auto& current_figure = *context.current_figure_2d;
	
	auto& current_element = get_or_create_element_2d(current_figure, name, draw_layer, ElementType2D::Point2D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	const size_t data_buffer_size = sizeof(Point2D) * points_buffer.number_of_elements;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, points_buffer.data_ptr, data_buffer_size);
	
	current_element.point_2d_metadata.number_of_points = points_buffer.number_of_elements;
	return ShowPoints2DSettingsBuilder(&current_element.point_2d_metadata.settings);
}


ShowImage2DSettingsBuilder ShowImage2D(const std::string& name, const Image2D& image, int draw_layer) {
	validate_initialized(ShowImage2D)
	auto& context = *toucan_context_ptr;
	validate_active_figure2d(ShowImage2D)
	auto& current_figure = *context.current_figure_2d;
	
	assert(image.width > 0 and image.height > 0 and image.image_buffer_ptr != nullptr);
	
	auto& current_element = get_or_create_element_2d(current_figure, name, draw_layer, ElementType2D::Image2D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	size_t bytes_per_pixel = get_bytes_per_pixel(image.format);
	
	const size_t data_buffer_size = bytes_per_pixel * image.width * image.height;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, image.image_buffer_ptr, data_buffer_size);
	
	// TODO(Matias): Move quad data to context struct to have one global quad.
	current_element.image_2d_metadata.width = image.width;
	current_element.image_2d_metadata.height = image.height;
	current_element.image_2d_metadata.format = image.format;
	return ShowImage2DSettingsBuilder(&current_element.image_2d_metadata.settings);
}

} // namespace Toucan
