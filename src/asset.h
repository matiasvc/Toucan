#pragma once

#include "gl/geometry.h"

struct AssetContext {
	unsigned int lineplot_2d_shader = 0;
	unsigned int point_2d_shader = 0;
	unsigned int image_2d_shader = 0;
	
	unsigned int point_3d_shader = 0;
	unsigned int line_3d_shader = 0;
	unsigned int mesh_3d_shader = 0;
	
	GeometryHandles sphere_geometry_handles = {};
	GeometryHandles cube_geometry_handles = {};
	GeometryHandles cylinder_geometry_handles = {};
};

unsigned int get_lineplot_2d_shader(AssetContext* context);
unsigned int get_point_2d_shader(AssetContext* context);
unsigned int get_image_2d_shader(AssetContext* context);

unsigned int get_point_3d_shader(AssetContext* context);
unsigned int get_line_3d_shader(AssetContext* context);
unsigned int get_mesh_3d_shader(AssetContext* context);

const GeometryHandles* get_sphere_handles_ptr(AssetContext* context);
const GeometryHandles* get_cube_handles_ptr(AssetContext* context);
const GeometryHandles* get_cylinder_handles_ptr(AssetContext* context);
