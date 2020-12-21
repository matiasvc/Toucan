#include "geometry.h"

#include <glad/glad.h>
#include "util/GLDebug.h"
#include "Utils.h"

struct TexturedVertex {
	Toucan::Vector3f position;
	Toucan::Vector3f normal;
	Toucan::Vector2f uv;
};

struct ColoredVertex {
	Toucan::Vector3f position;
	Toucan::Vector3f color;
};

struct TexturedMeshGeometryData {
	std::vector<TexturedVertex> vertices;
	std::vector<unsigned int> indices;
};


IndexedGeometryHandles generate_geometry_handles(const TexturedMeshGeometryData& geometry_data) {
	IndexedGeometryHandles geometry_handles;
	geometry_handles.number_of_indices = geometry_data.indices.size();
	
	glGenVertexArrays(1, &geometry_handles.vao);
	glGenBuffers(1, &geometry_handles.vbo);
	glGenBuffers(1, &geometry_handles.ebo);
	
	glBindVertexArray(geometry_handles.vao);
	glBindBuffer(GL_ARRAY_BUFFER, geometry_handles.vbo);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(TexturedVertex) * geometry_data.vertices.size()), geometry_data.vertices.data(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry_handles.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(unsigned int) * geometry_data.indices.size()), geometry_data.indices.data(), GL_STATIC_DRAW);
	
	// Position
	constexpr auto position_location = 0;
	glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(offset_of(&TexturedVertex::position)));
	glEnableVertexAttribArray(position_location);
	
	// Normal
	constexpr auto normal_location = 1;
	glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(offset_of(&TexturedVertex::normal)));
	glEnableVertexAttribArray(normal_location);
	
	// UV
	constexpr auto uv_location = 2;
	glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(offset_of(&TexturedVertex::uv)));
	glEnableVertexAttribArray(uv_location);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	return geometry_handles;
}

IndexedGeometryHandles generate_quad() {
	TexturedMeshGeometryData geometry_data;
	geometry_data.vertices.reserve(4);
	geometry_data.indices.reserve(6);
	
	geometry_data.vertices = {
			TexturedVertex{Toucan::Vector3f(0.0f, 0.0f, 0.0f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f(0.0f, 0.0f)},
			TexturedVertex{Toucan::Vector3f(1.0f, 0.0f, 0.0f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f(1.0f, 0.0f)},
			TexturedVertex{Toucan::Vector3f(0.0f, 1.0f, 0.0f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f(0.0f, 1.0f)},
			TexturedVertex{Toucan::Vector3f(1.0f, 1.0f, 0.0f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f(1.0f, 1.0f)},
	};
	
	geometry_data.indices = {
		0, 2, 1,
		1, 2, 3,
	};
	
	assert(geometry_data.vertices.size() == 4);
	assert(geometry_data.indices.size() == 6);
	return generate_geometry_handles(geometry_data);
}

IndexedGeometryHandles generate_sphere(int number_of_sectors, int number_of_stacks) {
	const size_t number_of_vertices = (number_of_sectors + 1)*(number_of_stacks + 1);
	const size_t number_of_indices = 3*2*(number_of_stacks - 1)*(number_of_sectors);
	
	TexturedMeshGeometryData geometry_data;
	geometry_data.vertices.reserve(number_of_vertices);
	geometry_data.indices.reserve(number_of_indices);
	
	constexpr float radius = 0.5f;
	constexpr float radius_inv = 1.0f / radius;
	
	const float sector_step = 2*M_PI/number_of_sectors;
	const float stack_step = M_PI/number_of_stacks;
	
	// TODO(Matias): Fix UV coordinates
	// TODO(Matias): Use a single vertex for top and bottom
	for (int stack_index = 0; stack_index <= number_of_stacks; ++stack_index) {
		const float stack_angle = 0.5f*static_cast<float>(M_PI) - static_cast<float>(stack_index)*stack_step;
		const float xz = radius * std::cos(stack_angle);
		const float y = radius * std::sin(stack_angle);
		
		for (int sector_index = 0; sector_index <= number_of_sectors; ++sector_index) {
			
			const float sector_angle = static_cast<float>(sector_index) * sector_step;
			
			const float x = xz * std::cos(sector_angle);
			const float z = xz * std::sin(sector_angle);
			
			const float u = static_cast<float>(sector_index) / static_cast<float>(number_of_sectors);
			const float v = static_cast<float>(stack_index) / static_cast<float>(number_of_stacks);
			
			geometry_data.vertices.emplace_back(TexturedVertex{
				Toucan::Vector3f(x, y, z),
				radius_inv * Toucan::Vector3f(x, y, z),
				Toucan::Vector2f(u, v)
			});
		}
	}
	assert(geometry_data.vertices.size() == number_of_vertices);
	
	for (int stack_index = 0; stack_index < number_of_stacks; ++stack_index) {
		unsigned int k1 = stack_index * (number_of_sectors + 1);
		unsigned int k2 = k1 + number_of_sectors + 1;
		
		for (int sector_index = 0; sector_index < number_of_sectors; ++sector_index) {
			
			if (stack_index != 0) {
				geometry_data.indices.emplace_back(k1);
				geometry_data.indices.emplace_back(k2);
				geometry_data.indices.emplace_back(k1 + 1);
			}
			
			if (stack_index != (number_of_stacks - 1)) {
				geometry_data.indices.emplace_back(k1 + 1);
				geometry_data.indices.emplace_back(k2);
				geometry_data.indices.emplace_back(k2 + 1);
			}
			
			k1++;
			k2++;
		}
	}
	
	assert(geometry_data.vertices.size() == number_of_vertices);
	assert(geometry_data.indices.size() == number_of_indices);
	return generate_geometry_handles(geometry_data);
}

IndexedGeometryHandles generate_cube() {
	TexturedMeshGeometryData geometry_data;
	geometry_data.vertices.reserve(24);
	geometry_data.indices.reserve(36);
	
	// TODO(Matias): Fix UV coordinates
	geometry_data.vertices = {
			// Front
			TexturedVertex{Toucan::Vector3f(0.5f, 0.5f, 0.5f), Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 0
			TexturedVertex{Toucan::Vector3f(-0.5f, 0.5f, 0.5f), Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 1
			TexturedVertex{Toucan::Vector3f(-0.5f, -0.5f, 0.5f), Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 2
			TexturedVertex{Toucan::Vector3f(0.5f, -0.5f, 0.5f), Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 3
			// Top
			TexturedVertex{Toucan::Vector3f(0.5f, -0.5f, 0.5f), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 4
			TexturedVertex{Toucan::Vector3f(-0.5f, -0.5f, 0.5f), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 5
			TexturedVertex{Toucan::Vector3f(-0.5f, -0.5f, -0.5f), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 6
			TexturedVertex{Toucan::Vector3f(0.5f, -0.5f, -0.5f), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 7
			// Right
			TexturedVertex{Toucan::Vector3f(0.5f, 0.5f, 0.5f), Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 8
			TexturedVertex{Toucan::Vector3f(0.5f, 0.5f, -0.5f), Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 9
			TexturedVertex{Toucan::Vector3f(0.5f, -0.5f, -0.5f), Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 10
			TexturedVertex{Toucan::Vector3f(0.5f, -0.5f, 0.5f), Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 11
			// Back
			TexturedVertex{Toucan::Vector3f(0.5f, -0.5f, -0.5f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 12
			TexturedVertex{Toucan::Vector3f(-0.5f, -0.5f, -0.5f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 13
			TexturedVertex{Toucan::Vector3f(-0.5f, 0.5f, -0.5f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 14
			TexturedVertex{Toucan::Vector3f(0.5f, 0.5f, -0.5f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 15
			// Left
			TexturedVertex{Toucan::Vector3f(-0.5f, -0.5f, -0.5f), -Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 16
			TexturedVertex{Toucan::Vector3f(-0.5f, -0.5f, 0.5f), -Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 17
			TexturedVertex{Toucan::Vector3f(-0.5f, 0.5f, 0.5f), -Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 18
			TexturedVertex{Toucan::Vector3f(-0.5f, 0.5f, -0.5f), -Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 19
			// Bottom
			TexturedVertex{Toucan::Vector3f(0.5f, 0.5f, -0.5f), Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 20
			TexturedVertex{Toucan::Vector3f(-0.5f, 0.5f, -0.5f), Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 21
			TexturedVertex{Toucan::Vector3f(-0.5f, 0.5f, 0.5f), Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 22
			TexturedVertex{Toucan::Vector3f(0.5f, 0.5f, 0.5f), Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 23
	};
	
	geometry_data.indices = {
			// Front
			2, 0, 1,
			2, 3, 0,
			// Top
			6, 4, 5,
			6, 7, 4,
			// Right
			10, 8, 9,
			10, 11, 8,
			// Back
			14, 12, 13,
			14, 15, 12,
			// Left
			18, 16, 17,
			18, 19, 16,
			// Bottom
			22, 20, 21,
			22, 23, 20,
	};
	
	assert(geometry_data.vertices.size() == 24);
	assert(geometry_data.indices.size() == 36);
	return generate_geometry_handles(geometry_data);
}

IndexedGeometryHandles generate_cylinder(int number_of_sectors) {
	const size_t number_of_vertices = 2 + 2*(number_of_sectors + 1) + 2*(number_of_sectors + 1);
	const size_t number_of_indices = 2*3*number_of_sectors + 2*3*number_of_sectors;
	
	TexturedMeshGeometryData geometry_data;
	geometry_data.vertices.reserve(number_of_vertices);
	geometry_data.indices.reserve(number_of_indices);
	
	const float radius = 0.5f;
	const float sector_angle = 2*M_PI/number_of_sectors;
	
	constexpr int top_vertex_index = 0;
	constexpr int bottom_vertex_index = 1;
	int vertex_index = 2;
	
	// TODO(Matias): Fix UV coordinates
	geometry_data.vertices.emplace_back(TexturedVertex{Toucan::Vector3f(0.0f, -0.5f, 0.0f), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()});
	geometry_data.vertices.emplace_back(TexturedVertex{Toucan::Vector3f(0.0f, 0.5f, 0.0f), Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()});
	
	// Top cap
	for (int sector_index = 0; sector_index <= number_of_sectors; ++sector_index) {
		const float angle = sector_angle * sector_index;
		
		const float x = std::cos(angle);
		const float z = std::sin(angle);
		
		geometry_data.vertices.emplace_back(TexturedVertex{Toucan::Vector3f(radius*x, -0.5f, radius*z), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()});
		geometry_data.vertices.emplace_back(TexturedVertex{Toucan::Vector3f(radius*x, -0.5f, radius*z), Toucan::Vector3f(x, 0.0f, z), Toucan::Vector2f::Zero()});
		geometry_data.vertices.emplace_back(TexturedVertex{Toucan::Vector3f(radius*x, 0.5f, radius*z), Toucan::Vector3f(x, 0.0f, z), Toucan::Vector2f::Zero()});
		geometry_data.vertices.emplace_back(TexturedVertex{Toucan::Vector3f(radius*x, 0.5f, radius*z), Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()});
		vertex_index += 4;
		
		if (sector_index != 0) {
			// Top
			geometry_data.indices.emplace_back(vertex_index - 8);
			geometry_data.indices.emplace_back(vertex_index - 4);
			geometry_data.indices.emplace_back(top_vertex_index);
			// Side
			geometry_data.indices.emplace_back(vertex_index - 6);
			geometry_data.indices.emplace_back(vertex_index - 3);
			geometry_data.indices.emplace_back(vertex_index - 7);
			
			geometry_data.indices.emplace_back(vertex_index - 6);
			geometry_data.indices.emplace_back(vertex_index - 2);
			geometry_data.indices.emplace_back(vertex_index - 3);
			// Bottom
			geometry_data.indices.emplace_back(vertex_index - 5);
			geometry_data.indices.emplace_back(bottom_vertex_index);
			geometry_data.indices.emplace_back(vertex_index - 1);
		}
	}
	
	assert(geometry_data.vertices.size() == number_of_vertices);
	assert(geometry_data.indices.size() == number_of_indices);
	return generate_geometry_handles(geometry_data);
}


