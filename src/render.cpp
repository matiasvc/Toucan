#include "render.h"

#include <numeric>
#include <cmath>
#include <cassert>

#include <iostream>

#include "util/GLDebug.h"
#include "Utils.h"

#include "asset.h"
#include "gl/shader.h"
#include "gl/geometry.h"
#include "gl/projection.h"

void recreate_framebuffer(unsigned int* framebuffer, unsigned int* framebuffer_color_texture, unsigned int* framebuffer_depth_texture, Toucan::Vector2i size) {
	
	glDeleteFramebuffers(1, framebuffer);
	glDeleteTextures(1, framebuffer_color_texture);
	
	if (framebuffer_depth_texture != nullptr) {
		glDeleteTextures(1, framebuffer_depth_texture);
	}
	
	glGenFramebuffers(1, framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
	
	glGenTextures(1, framebuffer_color_texture);
	glBindTexture(GL_TEXTURE_2D, *framebuffer_color_texture);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x(), size.y(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *framebuffer_color_texture, 0);
	
	if (framebuffer_depth_texture != nullptr) {
		glGenTextures(1, framebuffer_depth_texture);
		glBindTexture(GL_TEXTURE_2D, *framebuffer_depth_texture);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, size.x(), size.y(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *framebuffer_depth_texture, 0);
	}
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("ERROR! Unable to create framebuffer.");
	}
	
	glCheckError();
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Toucan::update_framebuffer_2d(Figure2D& figure_2d, Toucan::Vector2i size) {
	if (figure_2d.framebuffer == 0 or figure_2d.framebuffer_size != size) {
		size = Toucan::Vector2i(std::max(size.x(), 1), std::max(size.y(), 1));
		
		recreate_framebuffer(&figure_2d.framebuffer, &figure_2d.framebuffer_color_texture, nullptr, size);
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
			if (element_2d.line_plot_2d_metadata.vao == 0) { glGenVertexArrays(1, &element_2d.line_plot_2d_metadata.vao); glCheckError(); }
			if (element_2d.line_plot_2d_metadata.vbo == 0) { glGenBuffers(1, &element_2d.line_plot_2d_metadata.vbo); glCheckError(); }
			
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
				
				glCheckError();
			}
			
			if (element_2d.line_plot_2d_metadata.number_of_points == 0) { return; }
			
			unsigned int lineplot_2d_shader = get_lineplot_2d_shader(&context->asset_context);
			glUseProgram(lineplot_2d_shader);
			set_shader_uniform(lineplot_2d_shader, "line_color", element_2d.line_plot_2d_metadata.settings.line_color);
			
			const Toucan::Matrix4f model_matrix = model_to_world_matrix * element_2d.line_plot_2d_metadata.settings.scaled_transform.transformation_matrix_3d();
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
			
			glCheckError();
		} break;
		case Toucan::ElementType2D::Point2D: {
			
			//Create the buffers if they doesnt already exists
			if (element_2d.point_2d_metadata.vao == 0) { glGenVertexArrays(1, &element_2d.point_2d_metadata.vao); glCheckError(); }
			if (element_2d.point_2d_metadata.vbo == 0) { glGenBuffers(1, &element_2d.point_2d_metadata.vbo); glCheckError(); }
			
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
				
				glCheckError();
			}
			
			unsigned int point_2d_shader = get_point_2d_shader(&context->asset_context);
			glUseProgram(point_2d_shader);
			
			const Toucan::Matrix4f model_matrix = model_to_world_matrix * element_2d.point_2d_metadata.settings.scaled_transform.transformation_matrix_3d();
			set_shader_uniform(point_2d_shader, "model", model_matrix); // TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			set_shader_uniform(point_2d_shader, "view", world_to_camera_matrix); // TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			
			glBindVertexArray(element_2d.point_2d_metadata.vao);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glDrawArrays(GL_POINTS, 0, element_2d.point_2d_metadata.number_of_points);
			glBindVertexArray(0);
			
			glCheckError();
		} break;
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
					
					glCheckError();
				}
				
				// Upload Texture
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, element_2d.image_2d_metadata.texture);
				
				switch (element_2d.image_2d_metadata.format) {
					case ImageFormat::RG_U8: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RG, GL_UNSIGNED_BYTE, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::RG_U16: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RG, GL_UNSIGNED_SHORT, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::RG_U32: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RG, GL_UNSIGNED_INT, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::RG_F32: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RG, GL_FLOAT, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::RGB_U8: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RGB, GL_UNSIGNED_BYTE, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::RGB_U16: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RGB, GL_UNSIGNED_SHORT, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::RGB_U32: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RGB, GL_UNSIGNED_INT, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::RGB_F32: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RGB, GL_FLOAT, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::BGR_U8: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_BGR, GL_UNSIGNED_BYTE, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::BGR_U16: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_BGR, GL_UNSIGNED_SHORT, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::BGR_U32: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_BGR, GL_UNSIGNED_INT, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::BGR_F32: {
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_BGR, GL_FLOAT, element_2d.data_buffer_ptr);
					} break;
					case ImageFormat::GRAY_U16: {
						glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(element_2d.image_2d_metadata.width), static_cast<int>(element_2d.image_2d_metadata.height), 0, GL_RED, GL_UNSIGNED_SHORT, element_2d.data_buffer_ptr);
					} break;
				}
				
				glBindTexture(GL_TEXTURE_2D, 0);
				
				glCheckError();
				
				std::free(element_2d.data_buffer_ptr);
				element_2d.data_buffer_ptr = nullptr;
			}
			
			unsigned int image_draw_width = element_2d.image_2d_metadata.settings.image_display_width;
			if (image_draw_width == 0) { image_draw_width = element_2d.image_2d_metadata.width; }
			unsigned int image_draw_height = element_2d.image_2d_metadata.settings.image_display_height;
			if (image_draw_height == 0) { image_draw_height = element_2d.image_2d_metadata.height; }
			
			
			unsigned int image_2d_shader = get_image_2d_shader(&context->asset_context);
			glUseProgram(image_2d_shader);
			
			const Matrix4f image_size_matrix = ScaledTransform2Df(0.0f, Vector2f::Zero(), Vector2f(static_cast<float>(image_draw_width), static_cast<float>(image_draw_height))).transformation_matrix_3d();
			
			const Matrix4f model_matrix = model_to_world_matrix * element_2d.image_2d_metadata.settings.scaled_transform.transformation_matrix_3d() * image_size_matrix;
			set_shader_uniform(image_2d_shader, "model", model_matrix);
			set_shader_uniform(image_2d_shader, "view", world_to_camera_matrix);
			
			const IndexedGeometryHandles* geometry_handles_ptr = get_quad_handles_ptr(&context->asset_context);
			glBindVertexArray(geometry_handles_ptr->vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles_ptr->ebo);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, element_2d.image_2d_metadata.texture);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			
			glCheckError();
		} break;
	}
}

bool Toucan::update_framebuffer_3d(Figure3D& figure_3d, Toucan::Vector2i size) {
	if (figure_3d.framebuffer == 0 or figure_3d.framebuffer_size != size) {
		size = Vector2i(std::max(size.x(), 1), std::max(size.y(), 1));
		
		recreate_framebuffer(&figure_3d.framebuffer, &figure_3d.framebuffer_color_texture, &figure_3d.framebuffer_depth_texture, size);
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
			if (element_3d.grid_3d_metadata.vao_major == 0) { glGenVertexArrays(1, &element_3d.grid_3d_metadata.vao_major); glCheckError(); }
			if (element_3d.grid_3d_metadata.vao_minor == 0) { glGenVertexArrays(1, &element_3d.grid_3d_metadata.vao_minor); glCheckError(); }
			if (element_3d.grid_3d_metadata.vbo_major == 0) { glGenBuffers(1, &element_3d.grid_3d_metadata.vbo_major); glCheckError(); }
			if (element_3d.grid_3d_metadata.vbo_minor == 0) { glGenBuffers(1, &element_3d.grid_3d_metadata.vbo_minor); glCheckError(); }
			
			if (element_3d.data_buffer_ptr != nullptr) { // Do we need to create the grid points again
				const auto line_extent = element_3d.grid_3d_metadata.lines;
				const auto spacing = element_3d.grid_3d_metadata.spacing;
				const auto line_extent_position = line_extent * spacing;
				const auto number_of_lines = 1 + 2*line_extent;
				const auto number_of_vertices = 2*number_of_lines;
				
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
						line_vertices_major.emplace_back(Vector3f(spacing*line_index, -line_extent_position, 0.0f), line_color_origin);
						line_vertices_major.emplace_back(Vector3f(spacing*line_index, line_extent_position, 0.0f), line_color_origin);
						
						// Y-axis
						line_vertices_major.emplace_back(Vector3f(-line_extent_position, spacing*line_index, 0.0f), line_color_origin);
						line_vertices_major.emplace_back(Vector3f(line_extent_position, spacing*line_index, 0.0f), line_color_origin);
					} else if (line_index % 5 == 0) { // Major lines
						// X-axis
						line_vertices_major.emplace_back(Vector3f(spacing*line_index, -line_extent_position, 0.0f), line_color_major);
						line_vertices_major.emplace_back(Vector3f(spacing*line_index, line_extent_position, 0.0f), line_color_major);
						
						// Y-axis
						line_vertices_major.emplace_back(Vector3f(-line_extent_position, spacing*line_index, 0.0f), line_color_major);
						line_vertices_major.emplace_back(Vector3f(line_extent_position, spacing*line_index, 0.0f), line_color_major);
					} else { // Minor lines
						// X-axis
						line_vertices_minor.emplace_back(Vector3f(spacing*line_index, -line_extent_position, 0.0f), line_color_minor);
						line_vertices_minor.emplace_back(Vector3f(spacing*line_index, line_extent_position, 0.0f), line_color_minor);
						
						// Y-axis
						line_vertices_minor.emplace_back(Vector3f(-line_extent_position, spacing*line_index, 0.0f), line_color_minor);
						line_vertices_minor.emplace_back(Vector3f(line_extent_position, spacing*line_index, 0.0f), line_color_minor);
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
				
				glCheckError();
				
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
				
				glCheckError();
				
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
				
				// The pointer was just used to indicate change in settings, and did not actually point to any actual data.
				// Therefore, there is no need to free anything here.
				element_3d.data_buffer_ptr = nullptr;
				
			}
			
			unsigned int line_3d_shader = get_line_3d_shader(&context->asset_context);
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
			
			glCheckError();
		} break;
		case Toucan::ElementType3D::Axis3D: {
			unsigned int line_3d_shader = get_line_3d_shader(&context->asset_context);
			glUseProgram(line_3d_shader);
			
			const Toucan::Matrix4f  model_matrix = model_to_world_matrix * orientation_and_handedness_matrix * Toucan::ScaledTransform3Df(
					Quaternionf::Identity(), Toucan::Vector3f::Zero(), Toucan::Vector3f::Ones()*element_3d.axis_3d_metadata.settings.size
			).transformation_matrix();
			
			set_shader_uniform(line_3d_shader, "model", model_matrix);
			set_shader_uniform(line_3d_shader, "view", world_to_camera_matrix);
			set_shader_uniform(line_3d_shader, "projection", projection_matrix);
			
			const GeometryHandles* geometry_handles_ptr = get_axis_handles_ptr(&context->asset_context);
			
			glLineWidth(1.0);
			glBindVertexArray(geometry_handles_ptr->vao);
			glDrawArrays(GL_LINES, 0, geometry_handles_ptr->number_of_vertices);
			glBindVertexArray(0);
			glCheckError();
		} break;
		case Toucan::ElementType3D::Point3D: {
			// Create buffers if they doesnt already exists
			if (element_3d.point_3d_metadata.vao == 0) { glGenVertexArrays(1, &element_3d.point_3d_metadata.vao); glCheckError(); }
			if (element_3d.point_3d_metadata.vbo == 0) { glGenBuffers(1, &element_3d.point_3d_metadata.vbo); glCheckError(); }
			
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
			
			unsigned int point_3d_shader = get_point_3d_shader(&context->asset_context);
			glUseProgram(point_3d_shader);
			
			// TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			const Toucan::Matrix4f model_matrix = model_to_world_matrix * orientation_and_handedness_matrix * element_3d.point_3d_metadata.settings.scaled_transform.transformation_matrix();
			set_shader_uniform(point_3d_shader, "model", model_matrix);
			set_shader_uniform(point_3d_shader, "view", world_to_camera_matrix);
			set_shader_uniform(point_3d_shader, "projection", projection_matrix);
			
			glBindVertexArray(element_3d.point_3d_metadata.vao);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glDrawArrays(GL_POINTS, 0, element_3d.point_3d_metadata.number_of_points);
			glBindVertexArray(0);
			
			glCheckError();
		} break;
		case Toucan::ElementType3D::Line3D: {
			if (element_3d.line_3d_metadata.vao == 0) { glGenVertexArrays(1, &element_3d.line_3d_metadata.vao); glCheckError(); }
			if (element_3d.line_3d_metadata.vbo == 0) { glGenBuffers(1, &element_3d.line_3d_metadata.vbo); glCheckError(); }
			
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
			
			unsigned int line_3d_shader = get_line_3d_shader(&context->asset_context);
			glUseProgram(line_3d_shader);
			
			// TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			const Toucan::Matrix4f model_matrix = model_to_world_matrix * orientation_and_handedness_matrix * element_3d.line_3d_metadata.settings.scaled_transform.transformation_matrix();
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
			glCheckError();
		} break;
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
			
			unsigned int mesh_3d_shader = get_mesh_3d_shader(&context->asset_context);
			glUseProgram(mesh_3d_shader);
			
			// TODO(Matias): Use uniform buffer object to set the matrix once for all objects that are rendered
			set_shader_uniform(mesh_3d_shader, "view", world_to_camera_matrix);
			set_shader_uniform(mesh_3d_shader, "projection", projection_matrix);
			set_shader_uniform(mesh_3d_shader, "light_vector", element_3d.primitive_3d_metadata.settings.light_vector);
			
			for (int primitive_index = 0; primitive_index < element_3d.primitive_3d_metadata.number_of_primitives; ++primitive_index) {
				const Primitive3D& primitive = element_3d.primitive_3d_metadata.vertex_data_ptr[primitive_index];
				
				const IndexedGeometryHandles* geometry_handles_ptr = nullptr;
				
				switch (primitive.type) {
					case PrimitiveType::Cube: { geometry_handles_ptr = get_cube_handles_ptr(&context->asset_context); } break;
					case PrimitiveType::Sphere: { geometry_handles_ptr = get_sphere_handles_ptr(&context->asset_context); } break;
					case PrimitiveType::Cylinder: { geometry_handles_ptr = get_cylinder_handles_ptr(&context->asset_context); } break;
				}
				
				const Toucan::Matrix4f model_matrix = model_to_world_matrix * orientation_and_handedness_matrix * primitive.scaled_transform.transformation_matrix() * orientation_and_handedness_matrix.transpose();
				set_shader_uniform(mesh_3d_shader, "model", model_matrix);
				set_shader_uniform(mesh_3d_shader, "color", primitive.color);
				
				glBindVertexArray(geometry_handles_ptr->vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles_ptr->ebo);
				glDrawElements(GL_TRIANGLES, geometry_handles_ptr->number_of_indices, GL_UNSIGNED_INT, nullptr);
			}
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			
			glCheckError();
		} break;
	}
}

void Toucan::draw_axis_gizmo_3d(const Toucan::RigidTransform3Df& camera_transform, const Toucan::Vector2i& framebuffer_size, const Toucan::Matrix4f& orientation_and_handedness_matrix, Toucan::ToucanContext* context) {
	// TODO(Matias): Make these user editable
	constexpr float gizmo_size_fraction = 0.15f;
	constexpr int gizmo_max_absolute_size = 180;
	constexpr int gizmo_min_absolute_size = 75;
	
	const int width = static_cast<int>(framebuffer_size.x()*gizmo_size_fraction);
	const int height = static_cast<int>(framebuffer_size.y()*gizmo_size_fraction);
	
	int size = std::min(width, height);
	size = std::min(size, gizmo_max_absolute_size);
	size = std::max(size, gizmo_min_absolute_size);
	
	glClear(GL_DEPTH_BUFFER_BIT);
	
	// TODO(Matias): Make gizmo corner user editable
	glViewport(0, 0, size, size);
	
	unsigned int mesh_3d_shader = get_mesh_3d_shader(&context->asset_context);
	glUseProgram(mesh_3d_shader);
	
	const Toucan::Matrix4f world_to_camera_matrix = camera_transform.transformation_matrix();
	const Toucan::Matrix4f projection_matrix = create_3d_projection_matrix<float>(0.01f, 150.0f, 4.0f*size, Toucan::Vector2i(size, size));
	
	set_shader_uniform(mesh_3d_shader, "view", world_to_camera_matrix);
	set_shader_uniform(mesh_3d_shader, "projection", projection_matrix);
	set_shader_uniform(mesh_3d_shader, "light_vector", Toucan::Vector3f(1.0f, 1.5f, 1.8f).normalized());
	
	{
		set_shader_uniform(mesh_3d_shader, "color", Toucan::Color(0.8f, 0.8f, 0.8f));
		const Toucan::Matrix4f model_matrix = Toucan::ScaledTransform3Df(Quaternionf::Identity(), Vector3f::Zero(), Vector3f::Ones()).transformation_matrix() * orientation_and_handedness_matrix;
		set_shader_uniform(mesh_3d_shader, "model", model_matrix);
		
		const IndexedGeometryHandles* geometry_handles_ptr = get_cube_handles_ptr(&context->asset_context);
		glBindVertexArray(geometry_handles_ptr->vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles_ptr->ebo);
		glDrawElements(GL_TRIANGLES, geometry_handles_ptr->number_of_indices, GL_UNSIGNED_INT, nullptr);
	}
	
	{ // X-axis
		set_shader_uniform(mesh_3d_shader, "color", Toucan::Color::Red());
		const Toucan::Matrix4f model_matrix = orientation_and_handedness_matrix * Toucan::ScaledTransform3Df(
				Quaternionf(Vector3f::UnitY(), M_PI/2),
				Vector3f(5.5f, 0.0f, 0.0f),
				Vector3f(0.8f, 0.8f, 10.0f)).transformation_matrix();
		set_shader_uniform(mesh_3d_shader, "model", model_matrix);
		
		const IndexedGeometryHandles* geometry_handles_ptr = get_cylinder_handles_ptr(&context->asset_context);
		glBindVertexArray(geometry_handles_ptr->vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles_ptr->ebo);
		glDrawElements(GL_TRIANGLES, geometry_handles_ptr->number_of_indices, GL_UNSIGNED_INT, nullptr);
	}
	
	{ // Y-axis
		set_shader_uniform(mesh_3d_shader, "color", Toucan::Color::Green());
		const Toucan::Matrix4f model_matrix = orientation_and_handedness_matrix * Toucan::ScaledTransform3Df(
				Quaternionf(Vector3f::UnitX(), M_PI/2),
				Vector3f(0.0f, 5.5f, 0.0f),
				Vector3f(0.8f, 0.8f, 10.0f)).transformation_matrix();
		set_shader_uniform(mesh_3d_shader, "model", model_matrix);
		
		const IndexedGeometryHandles* geometry_handles_ptr = get_cylinder_handles_ptr(&context->asset_context);
		glBindVertexArray(geometry_handles_ptr->vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles_ptr->ebo);
		glDrawElements(GL_TRIANGLES, geometry_handles_ptr->number_of_indices, GL_UNSIGNED_INT, nullptr);
	}
	
	{ // Z-axis
		set_shader_uniform(mesh_3d_shader, "color", Toucan::Color::Blue());
		const Toucan::Matrix4f model_matrix = orientation_and_handedness_matrix * Toucan::ScaledTransform3Df(
				Quaternionf(Vector3f::UnitZ(), M_PI/2),
				Vector3f(0.0f, 0.0f, 5.5f),
				Vector3f(0.8f, 0.8f, 10.0f)).transformation_matrix();
		set_shader_uniform(mesh_3d_shader, "model", model_matrix);
		
		const IndexedGeometryHandles* geometry_handles_ptr = get_cylinder_handles_ptr(&context->asset_context);
		glBindVertexArray(geometry_handles_ptr->vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles_ptr->ebo);
		glDrawElements(GL_TRIANGLES, geometry_handles_ptr->number_of_indices, GL_UNSIGNED_INT, nullptr);
	}
	
	glViewport(0, 0, framebuffer_size.x(), framebuffer_size.y());
}
