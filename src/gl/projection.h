#pragma once

#include <Toucan/LinAlg.h>

template <typename scalar_t>
constexpr inline Toucan::Matrix4<scalar_t> create_3d_projection_matrix(scalar_t z_near, scalar_t z_far, scalar_t x_left, scalar_t x_right, scalar_t y_top, scalar_t y_bottom) {
	Toucan::Matrix4<scalar_t> m(
			scalar_t(2) * z_near / (x_right - x_left), scalar_t(0),                               (x_right + x_left) / (x_right - x_left),  scalar_t(0),
			scalar_t(0),                               scalar_t(2) * z_near / (y_bottom - y_top), (y_bottom + y_top) / (y_bottom - y_top),  scalar_t(0),
			scalar_t(0),                               scalar_t(0),                               (z_far + z_near) / (z_far - z_near),     -scalar_t(2) * z_far * z_near / (z_far - z_near),
			scalar_t(0),                               scalar_t(0),                               scalar_t(1),                              scalar_t(0)
	);
	
	return m;
}

template <typename scalar_t>
constexpr inline Toucan::Matrix4<scalar_t> create_3d_projection_matrix(scalar_t z_near, scalar_t z_far, scalar_t fx, scalar_t fy, scalar_t cx, scalar_t cy, Toucan::Vector2i image_size) {
	scalar_t x_min = -z_near * cx / fx;
	scalar_t x_max =  z_near * (image_size.x() - cx) / fx;
	scalar_t y_min = -z_near * cy / fy;
	scalar_t y_max =  z_near * (image_size.y() - cy) / fy;
	return create_3d_projection_matrix<scalar_t>(z_near, z_far, x_min, x_max, y_min, y_max);
}

template <typename scalar_t>
constexpr inline Toucan::Matrix4<scalar_t> create_3d_projection_matrix(float z_min, float z_max, float f, Toucan::Vector2i image_size) {
	return create_3d_projection_matrix<scalar_t>(z_min, z_max, f, f, scalar_t(0.5) * image_size.x(), scalar_t(0.5) * image_size.y(), image_size);
}
