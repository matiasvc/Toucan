#pragma once

constexpr auto line_3d_vs = R"GLSL(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 line_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	line_color = color;
	gl_Position = projection * view * model * vec4(position, 1.0);
}

)GLSL";

const auto line_3d_fs = R"GLSL(
#version 330 core

in vec3 line_color;

out vec4 frag_color;

void main() {
	frag_color = vec4(line_color, 1.0);
}
)GLSL";
