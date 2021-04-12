#include <Toucan/Toucan.h>

#include <cstring>

#include "validate.h"

namespace Toucan {

Element3D& get_or_create_element_3d(Figure3D& figure, const std::string& name, ElementType3D type) {
	// Does the Element3D object with that name already exist?
	Element3D* current_element_ptr = nullptr;
	for (auto& element : figure.elements) {
		if (element.name == name) {
			current_element_ptr = &element;
			break;
		}
	}
	
	// If it does not exist, we must create a new one.
	if (current_element_ptr == nullptr) {
		
		Element3D new_element_3d(name, type);
		
		auto& inserted_element = figure.elements.emplace_back(new_element_3d);
		current_element_ptr = &inserted_element;
	}
	
	return *current_element_ptr;
}

Figure3DSettingsBuilder BeginFigure3D(const std::string& name) {
	validate_initialized(BeginFigure3D)
	auto& toucan_context = *toucan_context_ptr;
	validate_inactive_figure3d(BeginFigure3D)
	
	Figure3D* figure_3d_ptr = nullptr;
	for (auto& figure_3d : toucan_context.figures_3d) { // Does the Figure2D already exists?
		if (figure_3d.name == name) {
			figure_3d_ptr = &figure_3d;
			break;
		}
	}
	
	if (figure_3d_ptr == nullptr) { // Do we need to create a new Figure2D?
		auto& figure_3d = toucan_context.figures_3d.emplace_back();
		
		// Add default 3D grid
		Element3D grid_element("Grid", ElementType3D::Grid3D);
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
	return Figure3DSettingsBuilder(&figure_3d_ptr->settings);
}

void EndFigure3D() {
	validate_initialized(EndFigure3D)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_figure3d(EndFigure3D)
	auto& figure_3d = *toucan_context.current_figure_3d;
	
	figure_3d.pose_stack.clear();
	
	toucan_context.current_figure_3d->mutex.unlock();
	toucan_context.current_figure_3d = nullptr;
}

void PushPose3D(const RigidTransform3Df& pose) {
	validate_initialized(PushPose3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(PushPose3D)
	auto& current_figure = *context.current_figure_3d;
	
	const auto& parent_pose = current_figure.pose_stack.back();
	current_figure.pose_stack.emplace_back(parent_pose * pose);
}

void PopPose3D() {
	validate_initialized(PopPose3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(PopPose3D)
	auto& current_figure = *context.current_figure_3d;
	
	if (current_figure.pose_stack.size() <= 1) { throw std::runtime_error("Toucan error! 'PopPose3D' was called without a matching call to `PushPose3D`."); }
	
	current_figure.pose_stack.pop_back();
}

void ClearPose3D() {
	validate_initialized(PopPose3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(PopPose3D)
	auto& current_figure = *context.current_figure_3d;
	
	if (current_figure.pose_stack.size() <= 1) { throw std::runtime_error("Toucan error! 'ClearPose3D' was called without any matching call to `PushPose3D`."); }
	
	current_figure.pose_stack.clear();
	current_figure.pose_stack.emplace_back(); // Add identity pose back
}

ShowAxis3DSettingsBuilder ShowAxis3D(const std::string& name) {
	validate_initialized(ShowAxis3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(ShowAxis3D)
	auto& current_figure = *context.current_figure_3d;
	
	auto& current_element = get_or_create_element_3d(current_figure, name, ElementType3D::Axis3D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	current_element.data_buffer_ptr = reinterpret_cast<void*>(1);
	return ShowAxis3DSettingsBuilder(&current_element.axis_3d_metadata.settings);
}

ShowPoints3DSettingsBuilder ShowPoints3D(const std::string& name, const Buffer<Point3D>& points_buffer) {
	validate_initialized(ShowPoints3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(ShowPoints3D)
	auto& current_figure = *context.current_figure_3d;
	
	auto& current_element = get_or_create_element_3d(current_figure, name, ElementType3D::Point3D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	const size_t data_buffer_size = sizeof(Point3D) * points_buffer.number_of_elements;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, points_buffer.data_ptr, data_buffer_size);
	
	current_element.point_3d_metadata.number_of_points = points_buffer.number_of_elements;
	return ShowPoints3DSettingsBuilder(&current_element.point_3d_metadata.settings);
}

ShowLines3DSettingsBuilder ShowLines3D(const std::string& name, const Buffer<LineVertex3D>& lines_buffer) {
	validate_initialized(ShowLines3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(ShowLines3D)
	auto& current_figure = *context.current_figure_3d;
	
	auto& current_element = get_or_create_element_3d(current_figure, name, ElementType3D::Line3D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	const size_t data_buffer_size = sizeof(LineVertex3D) * lines_buffer.number_of_elements;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, lines_buffer.data_ptr, data_buffer_size);
	
	current_element.line_3d_metadata.number_of_line_vertices = lines_buffer.number_of_elements;
	return ShowLines3DSettingsBuilder(&current_element.line_3d_metadata.settings);
}

ShowPrimitives3DSettingsBuilder ShowPrimitives3D(const std::string& name, const Buffer<Primitive3D>& primitives_buffer) {
	validate_initialized(ShowPrimitives3D)
	auto& context = *toucan_context_ptr;
	validate_active_figure3d(ShowPrimitives3D)
	auto& current_figure = *context.current_figure_3d;
	
	auto& current_element = get_or_create_element_3d(current_figure, name, ElementType3D::Primitive3D);
	
	current_element.pose = current_figure.pose_stack.back();
	
	// Drop any existing draw data that has not yet been sent to the GPU
	if (current_element.data_buffer_ptr != nullptr) {
		std::free(current_element.data_buffer_ptr); // TODO(Matias): Check if this happens often. Could be a problem.
		current_element.data_buffer_ptr = nullptr;
	}
	
	const size_t data_buffer_size = sizeof(Primitive3D) * primitives_buffer.number_of_elements;
	
	current_element.data_buffer_ptr = std::malloc(data_buffer_size);
	std::memcpy(current_element.data_buffer_ptr, primitives_buffer.data_ptr, data_buffer_size);
	
	current_element.primitive_3d_metadata.number_of_primitives = primitives_buffer.number_of_elements;
	return ShowPrimitives3DSettingsBuilder(&current_element.primitive_3d_metadata.settings);
}

} // namespace Toucan
