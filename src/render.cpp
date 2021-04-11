#include "render.h"

#include <numeric>
#include <cmath>
#include <cassert>
#include <algorithm>

#include <imgui.h>
#include <imgui_internal.h>

#include "Utils.h"

#include "gl/draw.h"
#include "gl/shader.h"
#include "gl/geometry.h"
#include "gl/projection.h"

#include "util/tick_number.h"


namespace Toucan {

bool Toucan::update_framebuffer_2d(Figure2D& figure_2d, Toucan::Vector2i size) {
	if (figure_2d.framebuffer == 0 or figure_2d.framebuffer_size != size) {
		size = Toucan::Vector2i(std::max(size.x(), 1), std::max(size.y(), 1));
		
		create_or_resize_framebuffer(&figure_2d.framebuffer, &figure_2d.framebuffer_color_texture, nullptr, size);
		figure_2d.framebuffer_size = size;
		
		return true;
	} else {
		return false;
	}
}

void Toucan::draw_element_2d(Element2D& element_2d, const Matrix4f& model_to_world_matrix, const Matrix4f& world_to_camera_matrix, ToucanContext* context) {
	
	switch (element_2d.type) {
		case ElementType2D::LinePlot2D: {
			
			//Create the buffers if they doesnt already exists
			if (element_2d.line_plot_2d_metadata.vao == 0) { glGenVertexArrays(1, &element_2d.line_plot_2d_metadata.vao); }
			if (element_2d.line_plot_2d_metadata.vbo == 0) { glGenBuffers(1, &element_2d.line_plot_2d_metadata.vbo); }
			
			if (element_2d.data_buffer_ptr != nullptr) {
				glBindVertexArray(element_2d.line_plot_2d_metadata.vao);
				glBindBuffer(GL_ARRAY_BUFFER, element_2d.line_plot_2d_metadata.vbo);
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vector2f) * element_2d.line_plot_2d_metadata.number_of_points), element_2d.data_buffer_ptr, GL_STATIC_DRAW);
				
				// Position
				constexpr auto position_location = 0;
				glVertexAttribPointer(position_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), reinterpret_cast<void*>(0));
				glEnableVertexAttribArray(position_location);
				
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
				
				std::free(element_2d.data_buffer_ptr);
				element_2d.data_buffer_ptr = nullptr;
				
			}
			
			if (element_2d.line_plot_2d_metadata.number_of_points == 0) { return; }
			
			unsigned int lineplot_2d_shader = get_lineplot_2d_shader(context->asset_context);
			glUseProgram(lineplot_2d_shader);
			set_shader_uniform(lineplot_2d_shader, "line_color", element_2d.line_plot_2d_metadata.settings.line_color);
			
			const Toucan::Matrix4f model_matrix = model_to_world_matrix * element_2d.line_plot_2d_metadata.settings.transform.transformation_matrix_3d();
			set_shader_uniform(lineplot_2d_shader, "model", model_matrix); // TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			set_shader_uniform(lineplot_2d_shader, "view", world_to_camera_matrix); // TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			
			glLineWidth(element_2d.line_plot_2d_metadata.settings.line_width);
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			glBindVertexArray(element_2d.line_plot_2d_metadata.vao);
			glDrawArrays(GL_LINE_STRIP, 0, element_2d.line_plot_2d_metadata.number_of_points);
			
			glDisable(GL_BLEND);
			glDisable(GL_LINE_SMOOTH);
			glBindVertexArray(0);
		}
			break;
		case Toucan::ElementType2D::Point2D: {
			
			//Create the buffers if they doesnt already exists
			if (element_2d.point_2d_metadata.vao == 0) { glGenVertexArrays(1, &element_2d.point_2d_metadata.vao); }
			if (element_2d.point_2d_metadata.vbo == 0) { glGenBuffers(1, &element_2d.point_2d_metadata.vbo); }
			
			if (element_2d.data_buffer_ptr != nullptr) { // Is there new data we need to upload
				glBindVertexArray(element_2d.point_2d_metadata.vao);
				glBindBuffer(GL_ARRAY_BUFFER, element_2d.point_2d_metadata.vbo);
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Point2D) * element_2d.point_2d_metadata.number_of_points), element_2d.data_buffer_ptr, GL_STATIC_DRAW);
				
				
				// Position
				constexpr auto position_location = 0;
				glVertexAttribPointer(position_location, 2, GL_FLOAT, GL_FALSE, sizeof(Point2D), reinterpret_cast<void*>(offset_of(&Point2D::position)));
				glEnableVertexAttribArray(position_location);
				
				// Color
				constexpr auto color_location = 1;
				glVertexAttribPointer(color_location, 3, GL_FLOAT, GL_FALSE, sizeof(Point2D), reinterpret_cast<void*>(offset_of(&Point2D::color)));
				glEnableVertexAttribArray(color_location);
				
				// Size
				constexpr auto size_location = 2;
				glVertexAttribPointer(size_location, 1, GL_FLOAT, GL_FALSE, sizeof(Point2D), reinterpret_cast<void*>(offset_of(&Point2D::size)));
				glEnableVertexAttribArray(size_location);
				
				// Shape
				constexpr auto shape_location = 3;
				glVertexAttribIPointer(shape_location, 1, GL_UNSIGNED_BYTE, sizeof(Point2D), reinterpret_cast<void*>(offset_of(&Point2D::shape)));
				glEnableVertexAttribArray(shape_location);
				
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
				
				std::free(element_2d.data_buffer_ptr);
				element_2d.data_buffer_ptr = nullptr;
			}
			
			unsigned int point_2d_shader = get_point_2d_shader(context->asset_context);
			glUseProgram(point_2d_shader);
			
			const Toucan::Matrix4f model_matrix = model_to_world_matrix * element_2d.point_2d_metadata.settings.transform.transformation_matrix_3d();
			set_shader_uniform(point_2d_shader, "model", model_matrix); // TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			set_shader_uniform(point_2d_shader, "view", world_to_camera_matrix); // TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			
			glBindVertexArray(element_2d.point_2d_metadata.vao);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glDrawArrays(GL_POINTS, 0, element_2d.point_2d_metadata.number_of_points);
			glBindVertexArray(0);
		}
			break;
		case Toucan::ElementType2D::Image2D: {
			if (element_2d.data_buffer_ptr != nullptr) { // Is there new data we need to upload?
				
				// Create the texture if it does not already exists
				if (element_2d.image_2d_metadata.texture == 0) {
					glGenTextures(1, &element_2d.image_2d_metadata.texture);
					
					glBindTexture(GL_TEXTURE_2D, element_2d.image_2d_metadata.texture);
					
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				
				// Upload Texture
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, element_2d.image_2d_metadata.texture);
				
				switch (element_2d.image_2d_metadata.format) {
					case ImageFormat::RG_U8:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RG, GL_UNSIGNED_BYTE, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::RG_U16:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RG, GL_UNSIGNED_SHORT, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::RG_U32:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RG, GL_UNSIGNED_INT, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::RG_F32:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RG, GL_FLOAT, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::RGB_U8:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RGB, GL_UNSIGNED_BYTE, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::RGB_U16:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RGB, GL_UNSIGNED_SHORT, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::RGB_U32:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RGB, GL_UNSIGNED_INT, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::RGB_F32:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RGB, GL_FLOAT, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::BGR_U8:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_BGR, GL_UNSIGNED_BYTE, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::BGR_U16:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_BGR, GL_UNSIGNED_SHORT, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::BGR_U32:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_BGR, GL_UNSIGNED_INT, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::BGR_F32:
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_BGR, GL_FLOAT, element_2d.data_buffer_ptr);
						break;
					case ImageFormat::GRAY_U16:
						glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RED, GL_UNSIGNED_SHORT, element_2d.data_buffer_ptr);
						break;
				}
				
				glBindTexture(GL_TEXTURE_2D, 0);
				
				std::free(element_2d.data_buffer_ptr);
				element_2d.data_buffer_ptr = nullptr;
			}
			
			unsigned int image_draw_width = element_2d.image_2d_metadata.settings.image_display_width;
			if (image_draw_width == 0) { image_draw_width = element_2d.image_2d_metadata.width; }
			unsigned int image_draw_height = element_2d.image_2d_metadata.settings.image_display_height;
			if (image_draw_height == 0) { image_draw_height = element_2d.image_2d_metadata.height; }
			
			
			unsigned int image_2d_shader = get_image_2d_shader(context->asset_context);
			glUseProgram(image_2d_shader);
			
			const Matrix4f image_size_matrix = ScaledTransform2Df(0.0f, Vector2f::Zero(), Vector2f(static_cast<float>(image_draw_width), static_cast<float>(image_draw_height))).transformation_matrix_3d();
			
			const Matrix4f model_matrix = model_to_world_matrix * element_2d.image_2d_metadata.settings.transform.transformation_matrix_3d() * image_size_matrix;
			set_shader_uniform(image_2d_shader, "model", model_matrix);
			set_shader_uniform(image_2d_shader, "view", world_to_camera_matrix);
			
			const IndexedGeometryHandles& geometry_handles = get_quad_handles(context->asset_context);
			glBindVertexArray(geometry_handles.vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles.ebo);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, element_2d.image_2d_metadata.texture);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
			break;
	}
}

bool Toucan::update_framebuffer_3d(Figure3D& figure_3d, Toucan::Vector2i size) {
	if (figure_3d.framebuffer == 0 or figure_3d.framebuffer_size != size) {
		size = Vector2i(std::max(size.x(), 1), std::max(size.y(), 1));
		
		create_or_resize_framebuffer(&figure_3d.framebuffer, &figure_3d.framebuffer_color_texture, &figure_3d.framebuffer_depth_texture, size);
		figure_3d.framebuffer_size = size;
		
		return true;
	} else {
		return false;
	}
}


void Toucan::draw_element_3d(Toucan::Element3D& element_3d, const Matrix4f& model_to_world_matrix, const Matrix4f& orientation_and_handedness_matrix, const Matrix4f& world_to_camera_matrix, const Matrix4f& projection_matrix,
                             Toucan::ToucanContext* context) {
	switch (element_3d.type) {
		case Toucan::ElementType3D::Grid3D: {
			
			//Create the buffers if they doesnt already exists
			if (element_3d.grid_3d_metadata.vao_major == 0) { glGenVertexArrays(1, &element_3d.grid_3d_metadata.vao_major); }
			if (element_3d.grid_3d_metadata.vao_minor == 0) { glGenVertexArrays(1, &element_3d.grid_3d_metadata.vao_minor); }
			if (element_3d.grid_3d_metadata.vbo_major == 0) { glGenBuffers(1, &element_3d.grid_3d_metadata.vbo_major); }
			if (element_3d.grid_3d_metadata.vbo_minor == 0) { glGenBuffers(1, &element_3d.grid_3d_metadata.vbo_minor); }
			
			if (element_3d.data_buffer_ptr != nullptr) { // Do we need to create the grid points again
				const auto line_extent = element_3d.grid_3d_metadata.lines;
				const auto spacing = element_3d.grid_3d_metadata.spacing;
				const auto line_extent_position = line_extent * spacing;
				const auto number_of_lines = 1 + 2 * line_extent;
				const auto number_of_vertices = 2 * number_of_lines;
				
				std::vector<LineVertex3D> line_vertices_major;
				line_vertices_major.reserve(number_of_vertices);
				
				std::vector<LineVertex3D> line_vertices_minor;
				line_vertices_minor.reserve(number_of_vertices);
				
				// TODO(Matias): Define these colors with a setting
				const Color line_color_origin(0.8, 0.8, 0.8);
				const Color line_color_major(0.4, 0.4, 0.4);
				const Color line_color_minor(0.3, 0.3, 0.3);
				
				for (int line_index = -line_extent; line_index <= line_extent; ++line_index) {
					if (line_index == 0) { // Special case for lines crossing the origin
						// X-axis
						line_vertices_major.emplace_back(Vector3f(spacing * line_index, -line_extent_position, 0.0f), line_color_origin);
						line_vertices_major.emplace_back(Vector3f(spacing * line_index, line_extent_position, 0.0f), line_color_origin);
						
						// Y-axis
						line_vertices_major.emplace_back(Vector3f(-line_extent_position, spacing * line_index, 0.0f), line_color_origin);
						line_vertices_major.emplace_back(Vector3f(line_extent_position, spacing * line_index, 0.0f), line_color_origin);
					} else if (line_index % 5 == 0) { // Major lines
						// X-axis
						line_vertices_major.emplace_back(Vector3f(spacing * line_index, -line_extent_position, 0.0f), line_color_major);
						line_vertices_major.emplace_back(Vector3f(spacing * line_index, line_extent_position, 0.0f), line_color_major);
						
						// Y-axis
						line_vertices_major.emplace_back(Vector3f(-line_extent_position, spacing * line_index, 0.0f), line_color_major);
						line_vertices_major.emplace_back(Vector3f(line_extent_position, spacing * line_index, 0.0f), line_color_major);
					} else { // Minor lines
						// X-axis
						line_vertices_minor.emplace_back(Vector3f(spacing * line_index, -line_extent_position, 0.0f), line_color_minor);
						line_vertices_minor.emplace_back(Vector3f(spacing * line_index, line_extent_position, 0.0f), line_color_minor);
						
						// Y-axis
						line_vertices_minor.emplace_back(Vector3f(-line_extent_position, spacing * line_index, 0.0f), line_color_minor);
						line_vertices_minor.emplace_back(Vector3f(line_extent_position, spacing * line_index, 0.0f), line_color_minor);
					}
				}
				
				constexpr auto position_location = 0;
				constexpr auto color_location = 1;
				
				// Minor lines
				glBindVertexArray(element_3d.grid_3d_metadata.vao_minor);
				
				glBindBuffer(GL_ARRAY_BUFFER, element_3d.grid_3d_metadata.vbo_minor);
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(LineVertex3D) * line_vertices_minor.size()), line_vertices_minor.data(), GL_STATIC_DRAW);
				element_3d.grid_3d_metadata.number_of_minor_vertices = line_vertices_minor.size();
				
				// Position
				glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex3D), reinterpret_cast<void*>(offset_of(&LineVertex3D::position)));
				glEnableVertexAttribArray(position_location);
				
				// Color
				glVertexAttribPointer(color_location, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex3D), reinterpret_cast<void*>(offset_of(&LineVertex3D::color)));
				glEnableVertexAttribArray(color_location);
				
				// Major lines
				glBindVertexArray(element_3d.grid_3d_metadata.vao_major);
				
				glBindBuffer(GL_ARRAY_BUFFER, element_3d.grid_3d_metadata.vbo_major);
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(LineVertex3D) * line_vertices_major.size()), line_vertices_major.data(), GL_STATIC_DRAW);
				element_3d.grid_3d_metadata.number_of_major_vertices = line_vertices_major.size();
				
				
				// Position
				glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex3D), reinterpret_cast<void*>(offset_of(&LineVertex3D::position)));
				glEnableVertexAttribArray(position_location);
				
				// Color
				glVertexAttribPointer(color_location, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex3D), reinterpret_cast<void*>(offset_of(&LineVertex3D::color)));
				glEnableVertexAttribArray(color_location);
				
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
				
				// The pointer was just used to indicate change in settings, and did not actually point to any actual data.
				// Therefore, there is no need to free anything here.
				element_3d.data_buffer_ptr = nullptr;
				
			}
			
			unsigned int line_3d_shader = get_line_3d_shader(context->asset_context);
			glUseProgram(line_3d_shader);
			
			const Toucan::Matrix4f model_matrix = model_to_world_matrix;
			
			// TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			set_shader_uniform(line_3d_shader, "model", model_matrix);
			set_shader_uniform(line_3d_shader, "view", world_to_camera_matrix);
			set_shader_uniform(line_3d_shader, "projection", projection_matrix);
			
			glLineWidth(1.0f);
			glBindVertexArray(element_3d.grid_3d_metadata.vao_minor);
			glDrawArrays(GL_LINES, 0, element_3d.grid_3d_metadata.number_of_minor_vertices);
			
			glLineWidth(2.0f);
			glBindVertexArray(element_3d.grid_3d_metadata.vao_major);
			glDrawArrays(GL_LINES, 0, element_3d.grid_3d_metadata.number_of_major_vertices);
			
			glLineWidth(1.0);
			glBindVertexArray(0);
			
		}
			break;
		case Toucan::ElementType3D::Axis3D: {
			unsigned int line_3d_shader = get_line_3d_shader(context->asset_context);
			glUseProgram(line_3d_shader);
			
			const Toucan::Matrix4f model_matrix = model_to_world_matrix * orientation_and_handedness_matrix * Toucan::ScaledTransform3Df(
					Quaternionf::Identity(), Toucan::Vector3f::Zero(), Toucan::Vector3f::Ones() * element_3d.axis_3d_metadata.settings.size
			).transformation_matrix();
			
			set_shader_uniform(line_3d_shader, "model", model_matrix);
			set_shader_uniform(line_3d_shader, "view", world_to_camera_matrix);
			set_shader_uniform(line_3d_shader, "projection", projection_matrix);
			
			const GeometryHandles& geometry_handles = get_axis_handles(context->asset_context);
			
			glLineWidth(1.0);
			glBindVertexArray(geometry_handles.vao);
			glDrawArrays(GL_LINES, 0, geometry_handles.number_of_vertices);
			glBindVertexArray(0);
		}
			break;
		case Toucan::ElementType3D::Point3D: {
			// Create buffers if they doesnt already exists
			if (element_3d.point_3d_metadata.vao == 0) { glGenVertexArrays(1, &element_3d.point_3d_metadata.vao); }
			if (element_3d.point_3d_metadata.vbo == 0) { glGenBuffers(1, &element_3d.point_3d_metadata.vbo); }
			
			if (element_3d.data_buffer_ptr != nullptr) {
				glBindVertexArray(element_3d.point_3d_metadata.vao);
				glBindBuffer(GL_ARRAY_BUFFER, element_3d.point_3d_metadata.vbo);
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Point3D) * element_3d.point_3d_metadata.number_of_points), element_3d.data_buffer_ptr, GL_STATIC_DRAW);
				
				// Position
				constexpr auto position_location = 0;
				glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(Point3D), reinterpret_cast<void*>(offset_of(&Point3D::position)));
				glEnableVertexAttribArray(position_location);
				
				// Color
				constexpr auto color_location = 1;
				glVertexAttribPointer(color_location, 3, GL_FLOAT, GL_FALSE, sizeof(Point3D), reinterpret_cast<void*>(offset_of(&Point3D::color)));
				glEnableVertexAttribArray(color_location);
				
				// Size
				constexpr auto size_location = 2;
				glVertexAttribPointer(size_location, 1, GL_FLOAT, GL_FALSE, sizeof(Point3D), reinterpret_cast<void*>(offset_of(&Point3D::size)));
				glEnableVertexAttribArray(size_location);
				
				// Shape
				constexpr auto shape_location = 3;
				glVertexAttribIPointer(shape_location, 1, GL_UNSIGNED_BYTE, sizeof(Point3D), reinterpret_cast<void*>(offset_of(&Point3D::shape)));
				glEnableVertexAttribArray(shape_location);
				
				std::free(element_3d.data_buffer_ptr);
				element_3d.data_buffer_ptr = nullptr;
			}
			
			unsigned int point_3d_shader = get_point_3d_shader(context->asset_context);
			glUseProgram(point_3d_shader);
			
			// TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			const Toucan::Matrix4f model_matrix = model_to_world_matrix * orientation_and_handedness_matrix * element_3d.point_3d_metadata.settings.transform.transformation_matrix();
			set_shader_uniform(point_3d_shader, "model", model_matrix);
			set_shader_uniform(point_3d_shader, "view", world_to_camera_matrix);
			set_shader_uniform(point_3d_shader, "projection", projection_matrix);
			
			glBindVertexArray(element_3d.point_3d_metadata.vao);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glDrawArrays(GL_POINTS, 0, element_3d.point_3d_metadata.number_of_points);
			glBindVertexArray(0);
		}
			break;
		case Toucan::ElementType3D::Line3D: {
			if (element_3d.line_3d_metadata.vao == 0) { glGenVertexArrays(1, &element_3d.line_3d_metadata.vao); }
			if (element_3d.line_3d_metadata.vbo == 0) { glGenBuffers(1, &element_3d.line_3d_metadata.vbo); }
			
			if (element_3d.data_buffer_ptr != nullptr) {
				glBindVertexArray(element_3d.line_3d_metadata.vao);
				glBindBuffer(GL_ARRAY_BUFFER, element_3d.line_3d_metadata.vbo);
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Point3D) * element_3d.line_3d_metadata.number_of_line_vertices), element_3d.data_buffer_ptr, GL_STATIC_DRAW);
				
				// Position
				constexpr auto position_location = 0;
				glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex3D), reinterpret_cast<void*>(offset_of(&LineVertex3D::position)));
				glEnableVertexAttribArray(position_location);
				
				// Color
				constexpr auto color_location = 1;
				glVertexAttribPointer(color_location, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex3D), reinterpret_cast<void*>(offset_of(&LineVertex3D::color)));
				glEnableVertexAttribArray(color_location);
				
				std::free(element_3d.data_buffer_ptr);
				element_3d.data_buffer_ptr = nullptr;
			}
			
			unsigned int line_3d_shader = get_line_3d_shader(context->asset_context);
			glUseProgram(line_3d_shader);
			
			// TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			const Toucan::Matrix4f model_matrix = model_to_world_matrix * orientation_and_handedness_matrix * element_3d.line_3d_metadata.settings.transform.transformation_matrix();
			set_shader_uniform(line_3d_shader, "model", model_matrix);
			set_shader_uniform(line_3d_shader, "view", world_to_camera_matrix);
			set_shader_uniform(line_3d_shader, "projection", projection_matrix);
			
			glLineWidth(element_3d.line_3d_metadata.settings.line_width);
			glBindVertexArray(element_3d.line_3d_metadata.vao);
			
			switch (element_3d.line_3d_metadata.settings.line_type) {
				case LineType::LINE_SEGMENTS:
					glDrawArrays(GL_LINES, 0, element_3d.line_3d_metadata.number_of_line_vertices);
					break;
				case LineType::LINE_STRIP:
					glDrawArrays(GL_LINE_STRIP, 0, element_3d.line_3d_metadata.number_of_line_vertices);
					break;
				case LineType::LINE_LOOP:
					glDrawArrays(GL_LINE_LOOP, 0, element_3d.line_3d_metadata.number_of_line_vertices);
					break;
			}
			
			glBindVertexArray(0);
		}
			break;
		case Toucan::ElementType3D::Primitive3D: {
			if (element_3d.data_buffer_ptr != nullptr) {
				
				// Free old data
				if (element_3d.primitive_3d_metadata.vertex_data_ptr != nullptr) {
					std::free(element_3d.primitive_3d_metadata.vertex_data_ptr);
				}
				
				// Move new data from data buffer to metadata
				element_3d.primitive_3d_metadata.vertex_data_ptr = reinterpret_cast<Toucan::Primitive3D*>(element_3d.data_buffer_ptr);
				element_3d.data_buffer_ptr = nullptr;
			}
			
			unsigned int mesh_3d_shader = get_mesh_3d_shader(context->asset_context);
			glUseProgram(mesh_3d_shader);
			
			// TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			set_shader_uniform(mesh_3d_shader, "view", world_to_camera_matrix);
			set_shader_uniform(mesh_3d_shader, "projection", projection_matrix);
			set_shader_uniform(mesh_3d_shader, "light_vector", element_3d.primitive_3d_metadata.settings.light_vector);
			
			for (int primitive_index = 0; primitive_index < element_3d.primitive_3d_metadata.number_of_primitives; ++primitive_index) {
				const Primitive3D& primitive = element_3d.primitive_3d_metadata.vertex_data_ptr[primitive_index];
				
				IndexedGeometryHandles geometry_handles;
				switch (primitive.type) {
					case PrimitiveType::Cube: {
						geometry_handles = get_cube_handles(context->asset_context);
					}
						break;
					case PrimitiveType::Sphere: {
						geometry_handles = get_sphere_handles(context->asset_context);
					}
						break;
					case PrimitiveType::Cylinder: {
						geometry_handles = get_cylinder_handles(context->asset_context);
					}
						break;
				}
				
				const Toucan::Matrix4f model_matrix = model_to_world_matrix * orientation_and_handedness_matrix * primitive.scaled_transform.transformation_matrix() * orientation_and_handedness_matrix.transpose();
				set_shader_uniform(mesh_3d_shader, "model", model_matrix);
				set_shader_uniform(mesh_3d_shader, "color", primitive.color);
				
				glBindVertexArray(geometry_handles.vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles.ebo);
				glDrawElements(GL_TRIANGLES, geometry_handles.number_of_indices, GL_UNSIGNED_INT, nullptr);
			}
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			
		}
			break;
	}
}

void Toucan::draw_axis_gizmo_3d(const Toucan::RigidTransform3Df& camera_transform, const Toucan::Vector2i& framebuffer_size, const Toucan::Matrix4f& orientation_and_handedness_matrix, Toucan::ToucanContext* context) {
	// TODO(Matias): Make these user editable
	constexpr float gizmo_size_fraction = 0.15f;
	constexpr int gizmo_max_absolute_size = 180;
	constexpr int gizmo_min_absolute_size = 75;
	
	const int width = static_cast<int>(framebuffer_size.x() * gizmo_size_fraction);
	const int height = static_cast<int>(framebuffer_size.y() * gizmo_size_fraction);
	
	int size = std::min(width, height);
	size = std::min(size, gizmo_max_absolute_size);
	size = std::max(size, gizmo_min_absolute_size);
	
	glClear(GL_DEPTH_BUFFER_BIT);
	
	// TODO(Matias): Make gizmo corner user editable
	glViewport(0, 0, size, size);
	
	unsigned int mesh_3d_shader = get_mesh_3d_shader(context->asset_context);
	glUseProgram(mesh_3d_shader);
	
	const Toucan::Matrix4f world_to_camera_matrix = camera_transform.transformation_matrix();
	const Toucan::Matrix4f projection_matrix = create_3d_projection_matrix<float>(0.01f, 150.0f, 4.0f * size, Toucan::Vector2i(size, size));
	
	set_shader_uniform(mesh_3d_shader, "view", world_to_camera_matrix);
	set_shader_uniform(mesh_3d_shader, "projection", projection_matrix);
	set_shader_uniform(mesh_3d_shader, "light_vector", Toucan::Vector3f(1.0f, 1.5f, 1.8f).normalized());
	
	{
		set_shader_uniform(mesh_3d_shader, "color", Toucan::Color(0.8f, 0.8f, 0.8f));
		const Toucan::Matrix4f model_matrix = Toucan::ScaledTransform3Df(Quaternionf::Identity(), Vector3f::Zero(), Vector3f::Ones()).transformation_matrix() * orientation_and_handedness_matrix;
		set_shader_uniform(mesh_3d_shader, "model", model_matrix);
		
		const IndexedGeometryHandles geometry_handles = get_cube_handles(context->asset_context);
		glBindVertexArray(geometry_handles.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles.ebo);
		glDrawElements(GL_TRIANGLES, geometry_handles.number_of_indices, GL_UNSIGNED_INT, nullptr);
	}
	
	{ // X-axis
		set_shader_uniform(mesh_3d_shader, "color", Toucan::Color::Red());
		const Toucan::Matrix4f model_matrix = orientation_and_handedness_matrix * Toucan::ScaledTransform3Df(
				Quaternionf(Vector3f::UnitY(), M_PI / 2),
				Vector3f(5.5f, 0.0f, 0.0f),
				Vector3f(0.8f, 0.8f, 10.0f)).transformation_matrix();
		set_shader_uniform(mesh_3d_shader, "model", model_matrix);
		
		const IndexedGeometryHandles geometry_handles = get_cylinder_handles(context->asset_context);
		glBindVertexArray(geometry_handles.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles.ebo);
		glDrawElements(GL_TRIANGLES, geometry_handles.number_of_indices, GL_UNSIGNED_INT, nullptr);
	}
	
	{ // Y-axis
		set_shader_uniform(mesh_3d_shader, "color", Toucan::Color::Green());
		const Toucan::Matrix4f model_matrix = orientation_and_handedness_matrix * Toucan::ScaledTransform3Df(
				Quaternionf(Vector3f::UnitX(), M_PI / 2),
				Vector3f(0.0f, 5.5f, 0.0f),
				Vector3f(0.8f, 0.8f, 10.0f)).transformation_matrix();
		set_shader_uniform(mesh_3d_shader, "model", model_matrix);
		
		const IndexedGeometryHandles geometry_handles = get_cylinder_handles(context->asset_context);
		glBindVertexArray(geometry_handles.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles.ebo);
		glDrawElements(GL_TRIANGLES, geometry_handles.number_of_indices, GL_UNSIGNED_INT, nullptr);
	}
	
	{ // Z-axis
		set_shader_uniform(mesh_3d_shader, "color", Toucan::Color::Blue());
		const Toucan::Matrix4f model_matrix = orientation_and_handedness_matrix * Toucan::ScaledTransform3Df(
				Quaternionf(Vector3f::UnitZ(), M_PI / 2),
				Vector3f(0.0f, 0.0f, 5.5f),
				Vector3f(0.8f, 0.8f, 10.0f)).transformation_matrix();
		set_shader_uniform(mesh_3d_shader, "model", model_matrix);
		
		const IndexedGeometryHandles geometry_handles_ptr = get_cylinder_handles(context->asset_context);
		glBindVertexArray(geometry_handles_ptr.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles_ptr.ebo);
		glDrawElements(GL_TRIANGLES, geometry_handles_ptr.number_of_indices, GL_UNSIGNED_INT, nullptr);
	}
	
	glViewport(0, 0, framebuffer_size.x(), framebuffer_size.y());
}
Toucan::Rectangle get_lineplot_2d_data_bounds(const Toucan::Element2D& element_2d, const Toucan::RigidTransform2Df& local_transform) {
	assert(element_2d.type == Toucan::ElementType2D::LinePlot2D);
	
	const Toucan::ScaledTransform2Df& data_transform = element_2d.line_plot_2d_metadata.settings.transform;
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
	
	const Toucan::ScaledTransform2Df& data_transform = element_2d.point_2d_metadata.settings.transform;
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

void draw_figure_2d_list(std::list<Figure2D>& figures, ToucanContext& context) {
	auto& io = ImGui::GetIO();
	
	for (auto& figure_2d : figures) {
		// TODO(Matias): This call seems to make the window size stuck. Possibly a ImGui bug.
		//ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 200.0f), ImVec2(-1.0f, -1.0f));
		if (ImGui::Begin(figure_2d.name.c_str())) {
			auto* window = ImGui::GetCurrentWindow();
			auto* window_draw_list = ImGui::GetWindowDrawList();
			auto& style = ImGui::GetStyle();
			
			std::unique_lock lock(figure_2d.mutex);
			// ***** Compute rects *****
			
			// Get settings
			const float& axis_x_size  = figure_2d.settings.axis_x_size;
			const float& axis_y_size  = figure_2d.settings.axis_y_size;
			const float& tick_width   = figure_2d.settings.tick_width;
			const float& plot_padding = figure_2d.settings.figure_padding;
			
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
#if !NDEBUG
				if(context.rdoc_api) context.rdoc_api->StartFrameCapture(nullptr, nullptr);
#endif
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
					Toucan::draw_element_2d(element, model_to_world, view_matrix, &context);
				}
				
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if !NDEBUG
				if(context.rdoc_api) context.rdoc_api->EndFrameCapture(nullptr, nullptr);
#endif
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
}

} // namespace Toucan