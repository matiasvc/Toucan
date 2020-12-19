#include "asset.h"

#include "gl/shader.h"

#include "shaders/shader_lineplot2d.h"
#include "shaders/shader_point2d.h"
#include "shaders/shader_image2d.h"

#include "shaders/shader_line3d.h"
#include "shaders/shader_point3d.h"
#include "shaders/shader_mesh3d.h"

unsigned int get_lineplot_2d_shader(AssetContext* context) {
	if (context->lineplot_2d_shader != 0) { return context->lineplot_2d_shader; }
	context->lineplot_2d_shader = create_shader_program(lineplot_2d_vs, lineplot_2d_fs);
	
	assert(context->lineplot_2d_shader != 0);
	return context->lineplot_2d_shader;
}

unsigned int get_point_2d_shader(AssetContext* context) {
	if (context->point_2d_shader != 0) { return context->point_2d_shader; }
	context->point_2d_shader = create_shader_program(point_2d_vs, point_2d_fs);
	
	assert(context->point_2d_shader != 0);
	return context->point_2d_shader;
}

unsigned int get_image_2d_shader(AssetContext* context) {
	if (context->image_2d_shader != 0) { return context->image_2d_shader; }
	context->image_2d_shader = create_shader_program(image_2d_vs, image_2d_rgb_fs);
	
	assert(context->image_2d_shader != 0);
	return context->image_2d_shader;
}


unsigned int get_point_3d_shader(AssetContext* context) {
	if (context->point_3d_shader != 0) { return context->point_3d_shader; }
	context->point_3d_shader = create_shader_program(point_3d_vs, point_3d_fs);
	
	assert(context->point_3d_shader != 0);
	return context->point_3d_shader;
}

unsigned int get_line_3d_shader(AssetContext* context) {
	if (context->line_3d_shader != 0) { return context->line_3d_shader; }
	context->line_3d_shader = create_shader_program(line_3d_vs, line_3d_fs);
	
	assert(context->line_3d_shader != 0);
	return context->line_3d_shader;
}

unsigned int get_mesh_3d_shader(AssetContext* context) {
	if (context->mesh_3d_shader != 0) { return context->mesh_3d_shader; }
	context->mesh_3d_shader = create_shader_program(mesh_3d_vs, mesh_3d_fs);
	
	assert(context->mesh_3d_shader != 0);
	return context->mesh_3d_shader;
}

const GeometryHandles* get_sphere_handles_ptr(AssetContext* context) {
	if (context->sphere_geometry_handles.vao != 0) { return &context->sphere_geometry_handles; }
	
	GeometryData geometry_data = generate_sphere_geometry_data();
	context->sphere_geometry_handles = generate_geometry_handles(geometry_data);
	
	return &context->sphere_geometry_handles;
}

const GeometryHandles* get_cube_handles_ptr(AssetContext* context) {
	if (context->cube_geometry_handles.vao != 0) { return &context->cube_geometry_handles; }
	
	GeometryData geometry_data = generate_cube_geometry_data();
	context->cube_geometry_handles = generate_geometry_handles(geometry_data);
	
	return &context->cube_geometry_handles;
}

const GeometryHandles* get_cylinder_handles_ptr(AssetContext* context) {
	if (context->cylinder_geometry_handles.vao != 0) { return &context->cylinder_geometry_handles; }
	
	GeometryData geometry_data = generate_cylinder_geometry_data();
	context->cylinder_geometry_handles = generate_geometry_handles(geometry_data);
	
	return &context->cylinder_geometry_handles;
}

