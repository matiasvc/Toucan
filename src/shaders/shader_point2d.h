#pragma once

const auto point_2d_vs = R"GLSL(
#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;
layout (location = 2) in float size;
layout (location = 3) in int shape;

out vec3 point_color;
flat out int point_shape;

uniform mat4 model;
uniform mat4 view;

void main(){
	point_color = color;
	point_shape = shape;
	
	gl_PointSize = size;
	gl_Position = view * model * vec4(position, 0.0, 1.0);
}
)GLSL";

const auto point_2d_fs = R"GLSL(
#version 330 core

in vec3 point_color;
flat in int point_shape;

out vec4 frag_color;

void main(){
	vec2 center_coord = 2.0 * gl_PointCoord - 1.0;
	
	switch (point_shape) {
		case 0: // Square
			// Square means we never discard
			break;
		case 1: // Cirle
			if(dot(center_coord, center_coord) > 1.0) {
				discard;
			}
			break;
		case 2: // Diamond
			if ((abs(center_coord.x) + abs(center_coord.y)) > 1.0) {
				discard;
			}
			break;
		case 3:// Cross
			if (abs(abs(center_coord.x) - abs(center_coord.y)) > 0.25) {
				discard;
			}
			break;
		case 4:
			float r = dot(center_coord, center_coord);
			if((r > 1.0) || (r < 0.55)) {
				discard;
			}
			break;
		default: { // Draw pink color as debug
			frag_color = vec4(1.0, 0.0, 1.0, 1.0);
			return;
		}
	}
	
	frag_color = vec4(point_color, 1.0);
}
)GLSL";
