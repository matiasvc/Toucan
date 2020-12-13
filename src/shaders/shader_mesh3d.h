#pragma once

const auto mesh_3d_vs = R"GLSL(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 vertex_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main () {
	vertex_normal = mat3(transpose(inverse(model))) * normal;
	gl_Position = projection * view * model * vec4(position, 1.0);
}

)GLSL";

const auto mesh_3d_fs = R"GLSL(
#version 330 core

in vec3 vertex_normal;

out vec4 frag_color;

uniform vec3 light_vector;
uniform vec3 color;

void main() {
	float angle = dot(vertex_normal, light_vector);
	frag_color = vec4((0.5 + max(0.0, 0.25*angle))*color, 1.0);
}

)GLSL";
