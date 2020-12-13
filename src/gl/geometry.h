#pragma once

#include <array>
#include "Toucan/DataTypes.h"

template <size_t number_of_vertices, size_t number_of_indices>
struct GeometryData {
	std::array<Toucan::Vertex, number_of_vertices> vertices;
	std::array<unsigned int, number_of_indices> indices;
};

constexpr int compute_number_of_sphere_vertices(int number_of_sectors, int number_of_stacks) {
	return (number_of_sectors + 1)*(number_of_stacks + 1);
}

constexpr int compute_number_of_sphere_indices(int number_of_sectors, int number_of_stacks) {
	return 3*2*(number_of_stacks - 1)*(number_of_sectors);
}


template<int number_of_sectors, int number_of_stacks>
constexpr GeometryData<
		compute_number_of_sphere_vertices(number_of_sectors, number_of_stacks),
		compute_number_of_sphere_indices(number_of_sectors, number_of_stacks)>
generate_sphere_vertices_and_indices() {
	std::array<Toucan::Vertex, compute_number_of_sphere_vertices(number_of_sectors, number_of_stacks)> vertices = {};
	std::array<unsigned int, compute_number_of_sphere_indices(number_of_sectors, number_of_stacks)> indices = {};
	
	
	constexpr float radius = 0.5f;
	constexpr float radius_inv = 1.0f / radius;
	
	const float sector_step = 2*M_PI/number_of_sectors;
	const float stack_step = M_PI/number_of_stacks;
	
	int vertex_index = 0;
	
	// TODO(Matias): Fix UV coordinates
	for (int stack_index = 0; stack_index <= number_of_stacks; ++stack_index) {
		
		const float stack_angle = 0.5f*static_cast<float>(M_PI) - static_cast<float>(stack_index)*stack_step;
		const float xz = radius * std::cos(stack_angle);
		const float y = radius * std::sin(stack_angle);
		
		for (int sector_index = 0; sector_index <= number_of_sectors; ++sector_index) {
			
			const float sector_angle = static_cast<float>(sector_index) * sector_step;
			
			const float x = xz * std::cos(sector_angle);
			const float z = xz * std::sin(sector_angle);
			
			const float u = static_cast<float>(sector_index) / number_of_sectors;
			const float v = static_cast<float>(stack_index) / number_of_stacks;
			
			vertices[vertex_index++] = Toucan::Vertex{
					Toucan::Vector3f(x, y, z),
					radius_inv * Toucan::Vector3f(x, y ,z),
					Toucan::Vector2f(u, v)
			};
		}
	}
	
	assert(vertex_index == vertices.size());
	
	int indices_index = 0;
	
	for (int stack_index = 0; stack_index < number_of_stacks; ++stack_index) {
		
		unsigned int k1 = stack_index * (number_of_sectors + 1);
		unsigned int k2 = k1 + number_of_sectors + 1;
		
		for (int sector_index = 0; sector_index < number_of_sectors; ++sector_index) {
			
			if (stack_index != 0) {
				indices[indices_index++] = k1;
				indices[indices_index++] = k2;
				indices[indices_index++] = k1 + 1;
			}
			
			if (stack_index != (number_of_stacks - 1)) {
				indices[indices_index++] = k1 + 1;
				indices[indices_index++] = k2;
				indices[indices_index++] = k2 + 1;
			}
			
			k1++;
			k2++;
		}
	}
	
	assert(indices_index == indices.size());
	
	return {vertices, indices};
}

constexpr GeometryData<24, 36> generate_cube_vertices_and_indices() {
	// TODO(Matias): Fix UV coordinates
	std::array<Toucan::Vertex, 24> vertices = {
			// Front
			Toucan::Vertex{Toucan::Vector3f( 0.5f,  0.5f,  0.5f),  Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 0
			Toucan::Vertex{Toucan::Vector3f(-0.5f,  0.5f,  0.5f),  Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 1
			Toucan::Vertex{Toucan::Vector3f(-0.5f, -0.5f,  0.5f),  Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 2
			Toucan::Vertex{Toucan::Vector3f( 0.5f, -0.5f,  0.5f),  Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 3
			// Top
			Toucan::Vertex{Toucan::Vector3f( 0.5f, -0.5f,  0.5f), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 4
			Toucan::Vertex{Toucan::Vector3f(-0.5f, -0.5f,  0.5f), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 5
			Toucan::Vertex{Toucan::Vector3f(-0.5f, -0.5f, -0.5f), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 6
			Toucan::Vertex{Toucan::Vector3f( 0.5f, -0.5f, -0.5f), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 7
			// Right
			Toucan::Vertex{Toucan::Vector3f( 0.5f,  0.5f,  0.5f),  Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 8
			Toucan::Vertex{Toucan::Vector3f( 0.5f,  0.5f, -0.5f),  Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 9
			Toucan::Vertex{Toucan::Vector3f( 0.5f, -0.5f, -0.5f),  Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 10
			Toucan::Vertex{Toucan::Vector3f( 0.5f, -0.5f,  0.5f),  Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 11
			// Back
			Toucan::Vertex{Toucan::Vector3f( 0.5f, -0.5f, -0.5f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 12
			Toucan::Vertex{Toucan::Vector3f(-0.5f, -0.5f, -0.5f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 13
			Toucan::Vertex{Toucan::Vector3f(-0.5f,  0.5f, -0.5f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 14
			Toucan::Vertex{Toucan::Vector3f( 0.5f,  0.5f, -0.5f), -Toucan::Vector3f::UnitZ(), Toucan::Vector2f::Zero()}, // 15
			// Left
			Toucan::Vertex{Toucan::Vector3f(-0.5f, -0.5f, -0.5f), -Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 16
			Toucan::Vertex{Toucan::Vector3f(-0.5f, -0.5f,  0.5f), -Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 17
			Toucan::Vertex{Toucan::Vector3f(-0.5f,  0.5f,  0.5f), -Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 18
			Toucan::Vertex{Toucan::Vector3f(-0.5f,  0.5f, -0.5f), -Toucan::Vector3f::UnitX(), Toucan::Vector2f::Zero()}, // 19
			// Bottom
			Toucan::Vertex{Toucan::Vector3f( 0.5f,  0.5f, -0.5f),  Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 20
			Toucan::Vertex{Toucan::Vector3f(-0.5f,  0.5f, -0.5f),  Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 21
			Toucan::Vertex{Toucan::Vector3f(-0.5f,  0.5f,  0.5f),  Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 22
			Toucan::Vertex{Toucan::Vector3f( 0.5f,  0.5f,  0.5f),  Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()}, // 23
	};
	std::array<unsigned int, 36> indices = {
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
	
	return {vertices, indices};
}

constexpr int compute_number_of_cylinder_vertices(int number_of_sectors) {
	//     Top and bottom cap              Top and bottom edge
	return 2 + 2*(number_of_sectors + 1) + 2*(number_of_sectors + 1);
}

constexpr int compute_number_of_cylinder_indices(int number_of_sector) {
	//     Top and bottom cap     Side
	return 2*3*number_of_sector + 2*3*number_of_sector;
}


template<int number_of_sectors>
constexpr GeometryData<compute_number_of_cylinder_vertices(number_of_sectors), compute_number_of_cylinder_indices(number_of_sectors)>
generate_cylinder_vertices_and_indices() {
	std::array<Toucan::Vertex, compute_number_of_cylinder_vertices(number_of_sectors)> vertices = {};
	std::array<unsigned int, compute_number_of_cylinder_indices(number_of_sectors)> indices = {};
	
	const float radius = 0.5f;
	const float sector_angle = 2*M_PI/number_of_sectors;
	
	int vertex_index = 0;
	const int top_vertex_index = vertex_index++;
	const int bottom_vertex_index = vertex_index++;
	
	// TODO(Matias): Fix UV coordinates
	vertices[top_vertex_index] = Toucan::Vertex{Toucan::Vector3f(0.0f, -0.5f, 0.0f), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()};
	vertices[bottom_vertex_index] = Toucan::Vertex{Toucan::Vector3f(0.0f, 0.5f, 0.0f), Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()};
	
	int indices_index = 0;
	
	// Top cap
	for (int sector_index = 0; sector_index <= number_of_sectors; ++sector_index) {
		const float angle = sector_angle * sector_index;
		
		const float x = std::cos(angle);
		const float z = std::sin(angle);
		
		vertices[vertex_index++] = Toucan::Vertex{Toucan::Vector3f(radius*x, -0.5f, radius*z), -Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()};
		vertices[vertex_index++] = Toucan::Vertex{Toucan::Vector3f(radius*x, -0.5f, radius*z), Toucan::Vector3f(x, 0.0f, z), Toucan::Vector2f::Zero()};
		vertices[vertex_index++] = Toucan::Vertex{Toucan::Vector3f(radius*x, 0.5f, radius*z), Toucan::Vector3f(x, 0.0f, z), Toucan::Vector2f::Zero()};
		vertices[vertex_index++] = Toucan::Vertex{Toucan::Vector3f(radius*x, 0.5f, radius*z), Toucan::Vector3f::UnitY(), Toucan::Vector2f::Zero()};
		
		if (sector_index != 0) {
			// Top
			indices[indices_index++] = vertex_index - 8;
			indices[indices_index++] = vertex_index - 4;
			indices[indices_index++] = top_vertex_index;
			// Side
			indices[indices_index++] = vertex_index - 6;
			indices[indices_index++] = vertex_index - 3;
			indices[indices_index++] = vertex_index - 7;
			
			indices[indices_index++] = vertex_index - 6;
			indices[indices_index++] = vertex_index - 2;
			indices[indices_index++] = vertex_index - 3;
			// Bottom
			indices[indices_index++] = vertex_index - 5;
			indices[indices_index++] = bottom_vertex_index;
			indices[indices_index++] = vertex_index - 1;
		}
	}
	
	assert(vertex_index == vertices.size());
	assert(indices_index == indices.size());
	
	return {vertices, indices};
}

void generate_primitive_data(Toucan::PrimitiveType primitive_type, unsigned int* const vao_ptr, unsigned int* const vbo_ptr, unsigned int* const ebo_ptr, unsigned int* const number_of_indices_ptr) {
	
	glGenVertexArrays(1, vao_ptr);
	glGenBuffers(1, vbo_ptr);
	glGenBuffers(1, ebo_ptr);
	
	switch (primitive_type) {
		case Toucan::PrimitiveType::Sphere: {
			const auto [vertices, indices] = generate_sphere_vertices_and_indices<32, 20>();
			*number_of_indices_ptr = indices.size();
			
			glBindVertexArray(*vao_ptr);
			glBindBuffer(GL_ARRAY_BUFFER, *vbo_ptr);
			glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Toucan::Vertex) * vertices.size()), vertices.data(), GL_STATIC_DRAW);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo_ptr);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(unsigned int) * indices.size()), indices.data(), GL_STATIC_DRAW);
		} break;
		case Toucan::PrimitiveType::Cube: {
			const auto [vertices, indices] = generate_cube_vertices_and_indices();
			*number_of_indices_ptr = indices.size();
			
			glBindVertexArray(*vao_ptr);
			glBindBuffer(GL_ARRAY_BUFFER, *vbo_ptr);
			glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Toucan::Vertex) * vertices.size()), vertices.data(), GL_STATIC_DRAW);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo_ptr);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(unsigned int) * indices.size()), indices.data(), GL_STATIC_DRAW);
		} break;
		case Toucan::PrimitiveType::Cylinder: {
			const auto [vertices, indices] = generate_cylinder_vertices_and_indices<32>();
			*number_of_indices_ptr = indices.size();
			
			glBindVertexArray(*vao_ptr);
			glBindBuffer(GL_ARRAY_BUFFER, *vbo_ptr);
			glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Toucan::Vertex) * vertices.size()), vertices.data(), GL_STATIC_DRAW);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo_ptr);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(unsigned int) * indices.size()), indices.data(), GL_STATIC_DRAW);
		} break;
	}
	
	// Position
	constexpr auto position_location = 0;
	glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(Toucan::Vertex), reinterpret_cast<void*>(offset_of(&Toucan::Vertex::position)));
	glEnableVertexAttribArray(position_location);
	
	// Normal
	constexpr auto normal_location = 1;
	glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(Toucan::Vertex), reinterpret_cast<void*>(offset_of(&Toucan::Vertex::normal)));
	glEnableVertexAttribArray(normal_location);
	
	// UV
	constexpr auto uv_location = 2;
	glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, sizeof(Toucan::Vertex), reinterpret_cast<void*>(offset_of(&Toucan::Vertex::uv)));
	glEnableVertexAttribArray(uv_location);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glCheckError();
}
