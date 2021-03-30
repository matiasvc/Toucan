#pragma once

#include <cassert>

#include <Toucan/LinAlg.h>
#include <Toucan/Setting.h>

namespace Toucan {

template<typename T, typename U>
constexpr size_t offset_of(U T::*member) {
	return (char*) &((T*) nullptr->*member) - (char*) nullptr;
}

inline Toucan::Matrix4f create_3d_orientation_and_handedness_matrix(const Orientation& orientation, const Handedness& handedness) {
	Toucan::Vector3f x_vec = Toucan::Vector3f::UnitX();
	Toucan::Vector3f y_vec = Toucan::Vector3f::UnitY();
	Toucan::Vector3f z_vec = Toucan::Vector3f::UnitZ();
	
	switch (orientation) {
		case Orientation::X_UP : {
			x_vec = Toucan::Vector3f::UnitZ();
			y_vec = Toucan::Vector3f::UnitX();
			z_vec = x_vec.cross_product(y_vec);
			if (handedness == Handedness::LEFT_HANDED) { z_vec = -z_vec; }
		} break;
		case Orientation::X_DOWN : {
			x_vec = -Toucan::Vector3f::UnitZ();
			y_vec = Toucan::Vector3f::UnitX();
			z_vec = x_vec.cross_product(y_vec);
			if (handedness == Handedness::LEFT_HANDED) { z_vec = -z_vec; }
		} break;
		case Orientation::Y_UP : {
			y_vec = Toucan::Vector3f::UnitZ();
			z_vec = Toucan::Vector3f::UnitX();
			x_vec = y_vec.cross_product(z_vec);
			if (handedness == Handedness::LEFT_HANDED) { x_vec = -x_vec; }
		} break;
		case Orientation::Y_DOWN : {
			y_vec = -Toucan::Vector3f::UnitZ();
			z_vec = Toucan::Vector3f::UnitX();
			x_vec = y_vec.cross_product(z_vec);
			if (handedness == Handedness::LEFT_HANDED) { x_vec = -x_vec; }
		} break;
		case Orientation::Z_UP : {
			z_vec = Toucan::Vector3f::UnitZ();
			x_vec = Toucan::Vector3f::UnitX();
			y_vec = z_vec.cross_product(x_vec);
			if (handedness == Handedness::LEFT_HANDED) { y_vec = -y_vec; }
		} break;
		case Orientation::Z_DOWN : {
			z_vec = -Toucan::Vector3f::UnitZ();
			x_vec = Toucan::Vector3f::UnitX();
			y_vec = z_vec.cross_product(x_vec);
			if (handedness == Handedness::LEFT_HANDED) { y_vec = -y_vec; }
		} break;
	}
	
	
	Toucan::Matrix4f m(
			x_vec.x(), y_vec.x(), z_vec.x(), 0.0f,
			x_vec.y(), y_vec.y(), z_vec.y(), 0.0f,
			x_vec.z(), y_vec.z(), z_vec.z(), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
			);
	
	return m;
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
