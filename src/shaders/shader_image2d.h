#pragma once

const auto image_2d_vs = R"GLSL(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uv;

out vec2 uv_coordinate;

uniform mat4 model;
uniform mat4 view;

void main() {
	uv_coordinate = uv;

	gl_Position = view * model * vec4(position, 1.0);
}
)GLSL";

const auto image_2d_rgb_fs = R"GLSL(
#version 330 core

in vec2 uv_coordinate;

uniform sampler2D image;

out vec4 fragment_color;

void main() {
	fragment_color = vec4(texture(image, uv_coordinate).rgb, 1.0);
}
)GLSL";

const auto image_2d_r_monochrome_fs = R"GLSL(
#version 330 core

in vec2 uv_coordinate;

uniform sampler2D image;

out vec4 fragment_color;

void main() {
	fragment_color = vec4(texture(image, uv_coordinate).rrr, 1.0);
}
)GLSL";
