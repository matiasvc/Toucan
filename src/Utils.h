#pragma once

#include <cassert>

#include <Toucan/LinAlg.h>

namespace Toucan {

template<typename T, typename U>
constexpr size_t offset_of(U T::*member) {
	return (char*) &((T*) nullptr->*member) - (char*) nullptr;
}

constexpr inline Toucan::Matrix4f create_3d_projection_matrix(float z_near, float z_far, float x_left, float x_right, float y_top, float y_bottom) {
	Toucan::Matrix4f m(
			2.0f * z_near / (x_right - x_left), 0.0f,                               (x_right + x_left) / (x_right - x_left),  0.0f,
			0.0f,                               2.0f * z_near / (y_bottom - y_top), (y_bottom + y_top) / (y_bottom - y_top),  0.0f,
			0.0f,                               0.0f,                               (z_far + z_near) / (z_far - z_near),     -2.0f * z_far * z_near / (z_far - z_near),
			0.0f,                               0.0f,                               1.0f,                                     0.0f
	);
	
	return m;
}

constexpr inline Toucan::Matrix4f create_3d_projection_matrix(float z_near, float z_far, float fx, float fy, float cx, float cy, Vector2i image_size) {
	float x_min = -z_near * cx / fx;
	float x_max =  z_near * (image_size.x() - cx) / fx;
	float y_min = -z_near * cy / fy;
	float y_max =  z_near * (image_size.y() - cy) / fy;
	return create_3d_projection_matrix(z_near, z_far, x_min, x_max, y_min, y_max);
}

constexpr inline Toucan::Matrix4f create_3d_projection_matrix(float z_min, float z_max, float f, Vector2i image_size) {
	return create_3d_projection_matrix(z_min, z_max, f, f, 0.5f * image_size.x(), 0.5f * image_size.y(), image_size);
}

inline Toucan::Matrix4f create_2d_view_matrix(const Rectangle& draw_view, YAxisDirection y_axis_direction) {
	const float& a = draw_view.min.x();
	const float& b = draw_view.min.y();
	const float w = draw_view.width();
	const float h = draw_view.height();
	
	
	
	if (y_axis_direction == YAxisDirection::UP) {
		Toucan::Matrix4f m(
				2.0f/w,  0.0f,   0.0f, -2.0f*a/w - 1.0f ,
				0.0f,   -2.0f/h, 0.0f,  2.0f*b/h + 1.0f ,
				0.0f,    0.0f,   1.0f,  0.0f            ,
				0.0f,    0.0f,   0.0f,  1.0f
		);
		
		return m;
	} else {
		Toucan::Matrix4f m(
				2.0f/w, 0.0f,   0.0f, -2.0f*a/w - 1.0f ,
				0.0f,   2.0f/h, 0.0f, -2.0f*b/h - 1.0f ,
				0.0f,   0.0f,   1.0f,  0.0f            ,
				0.0f,   0.0f,   0.0f,  1.0f
		);
		
		return m;
	}
	
	
}

inline std::vector<float> data_to_pixel(const std::vector<float>& data_coords,
                                         float data_from_value, float data_to_value, float pixel_from_value, float pixel_to_value) {
	std::vector<float> pixel_coords;
	pixel_coords.reserve(data_coords.size());
	
	const float data_width = data_to_value - data_from_value;
	const float pixel_width = pixel_to_value - pixel_from_value;
	
	for (auto data_coord : data_coords) {
		float normalized_coord = (data_coord - data_from_value)/data_width;
		float pixel_coord = pixel_width*normalized_coord + pixel_from_value;
		pixel_coords.emplace_back(pixel_coord);
	}
	
	return pixel_coords;
}

template<typename T>
constexpr inline
T remap(T x, T x0, T x1, T y0, T y1) {
	return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

template<typename T>
constexpr inline
T sgn(T value) {
	return value < T(0) ? T(-1) :
	       value > T(0) ? T(1)  :
	       T(0);
}

} // namespace Toucan
