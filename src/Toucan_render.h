#pragma once

#include "Toucan_internals.h"

namespace Toucan {

bool update_framebuffer_2d(Figure2D& figure_2d, Toucan::Vector2i size);
void draw_element_2d(Element2D& element_2d, const Matrix4f& model_to_world_matrix, const Matrix4f& world_to_camera_matrix, ToucanContext* context);

bool update_framebuffer_3d(Figure3D& figure_3d, Toucan::Vector2i size);
void draw_element_3d(Toucan::Element3D& element_3d, const Matrix4f& model_to_world_matrix, const Matrix4f& world_to_camera_matrix, const Matrix4f& projection_matrix, Toucan::ToucanContext* context);

} // namespace Toucan
