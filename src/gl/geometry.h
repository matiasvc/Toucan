#pragma once

#include <vector>

#include "Toucan/LinAlg.h"
#include "Toucan/DataTypes.h"

struct Vertex {
	Vertex(const Toucan::Vector3f& position, const Toucan::Vector3f& normal, const Toucan::Vector2f& uv) :
	position{position}, normal{normal}, uv{uv} { }
	
	Toucan::Vector3f position;
	Toucan::Vector3f normal;
	Toucan::Vector2f uv;
};

struct GeometryData {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

struct GeometryHandles {
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ebo = 0;
	unsigned int number_of_indices = 0;
};

GeometryData generate_sphere_geometry_data(int number_of_sectors = 32, int number_of_stacks = 20);
GeometryData generate_cube_geometry_data();
GeometryData generate_cylinder_geometry_data(int number_of_sectors = 32);

GeometryHandles generate_geometry_handles(const GeometryData& geometry_data);
