#pragma once

#include "gl/geometry.h"

struct AssetContext {
	unsigned int lineplot_2d_shader = 0;
	unsigned int point_2d_shader = 0;
	unsigned int image_2d_shader = 0;
	
	unsigned int point_3d_shader = 0;
	unsigned int line_3d_shader = 0;
	unsigned int mesh_3d_shader = 0;
	
	IndexedGeometryHandles quad_geometry_handles = {};
	IndexedGeometryHandles sphere_geometry_handles = {};
	IndexedGeometryHandles cube_geometry_handles = {};
	IndexedGeometryHandles cylinder_geometry_handles = {};
};

unsigned int get_lineplot_2d_shader(AssetContext* context);
unsigned int get_point_2d_shader(AssetContext* context);
unsigned int get_image_2d_shader(AssetContext* context);

unsigned int get_point_3d_shader(AssetContext* context);
unsigned int get_line_3d_shader(AssetContext* context);
unsigned int get_mesh_3d_shader(AssetContext* context);

const IndexedGeometryHandles* get_quad_handles_ptr(AssetContext* context);
const IndexedGeometryHandles* get_sphere_handles_ptr(AssetContext* context);
const IndexedGeometryHandles* get_cube_handles_ptr(AssetContext* context);
const IndexedGeometryHandles* get_cylinder_handles_ptr(AssetContext* context);
