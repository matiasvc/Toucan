#pragma once

#include <vector>

#include "Toucan/LinAlg.h"
#include "Toucan/DataTypes.h"

struct GeometryHandles {
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int number_of_vertices = 0;
};

struct IndexedGeometryHandles {
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ebo = 0;
	unsigned int number_of_indices = 0;
};

GeometryHandles generate_axis();

IndexedGeometryHandles generate_quad();
IndexedGeometryHandles generate_sphere(int number_of_sectors = 32, int number_of_stacks = 20);
IndexedGeometryHandles generate_cube();
IndexedGeometryHandles generate_cylinder(int number_of_sectors = 32);

