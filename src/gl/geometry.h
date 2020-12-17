#pragma once

#include <vector>
#include "Toucan/DataTypes.h"

struct GeometryData {
	std::vector<Toucan::Vertex> vertices;
	std::vector<unsigned int> indices;
};

GeometryData generate_sphere_geometry_data(int number_of_sectors = 32, int number_of_stacks = 20);
GeometryData generate_cube_geometry_data();
GeometryData generate_cylinder_geometry_data(int number_of_sectors = 32);
void generate_primitive_data(Toucan::PrimitiveType primitive_type, unsigned int* vao_ptr, unsigned int* vbo_ptr, unsigned int* ebo_ptr, unsigned int* number_of_indices_ptr);
