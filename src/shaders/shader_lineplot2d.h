#pragma once

const auto lineplot_2d_vs = R"GLSL(
#version 330 core

layout (location = 0) in vec2 position;

uniform mat4 model;
uniform mat4 view;

void main(){
	gl_Position = view * model * vec4(position, 0.0, 1.0);
}

)GLSL";

const auto lineplot_2d_fs = R"GLSL(
#version 330 core

uniform vec3 line_color;

out vec4 frag_color;

void main() {
	frag_color = vec4(line_color, 1.0);
}
)GLSL";
