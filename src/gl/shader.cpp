#include "shader.h"

#include <glad/glad.h>

unsigned int compile_shader(const char* shader_source, uint32_t type) {
	unsigned int shader = glCreateShader(type);
	
	glShaderSource(shader, 1, &shader_source, nullptr);
	glCompileShader(shader);
	
	int success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success){
		constexpr size_t log_length = 2048;
		char info_log[log_length];
		glGetShaderInfoLog(shader, log_length, nullptr, info_log);
		std::ostringstream ss;
		ss << "ERROR! Shader compilation failed: " << info_log;
		throw std::runtime_error(ss.str());
	}
	
	return shader;
}

unsigned int create_shader_program(const char* vertex_source, const char* fragment_source) {
	auto vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER);
	auto fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER);
	
	auto program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	
	
	int success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if(!success){
		constexpr size_t log_length = 2048;
		char info_log[log_length];
		glGetProgramInfoLog(program, log_length, nullptr, info_log);
		std::stringstream ss;
		ss << "Shader program linking failed: " << info_log;
		throw std::runtime_error(ss.str());
	}
	
	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	
	return program;
}

unsigned int create_shader_program(const char* vertex_source, const char* geometry_source, const char* fragment_source) {
	auto vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER);
	auto geometry_shader = compile_shader(geometry_source, GL_GEOMETRY_SHADER);
	auto fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER);
	
	auto program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, geometry_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	
	
	int success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if(!success){
		constexpr size_t log_length = 2048;
		char info_log[log_length];
		glGetProgramInfoLog(program, log_length, nullptr, info_log);
		std::stringstream ss;
		ss << "Shader program linking failed: " << info_log;
		throw std::runtime_error(ss.str());
	}
	
	glDetachShader(program, vertex_shader);
	glDetachShader(program, geometry_shader);
	glDetachShader(program, fragment_shader);
	
	glDeleteShader(vertex_shader);
	glDeleteShader(geometry_shader);
	glDeleteShader(fragment_shader);
	
	return program;
	
}

void set_shader_uniform(unsigned int shader, const std::string& name, int value) {
	auto location = glGetUniformLocation(shader, name.c_str());
	assert(location != -1);
	glUniform1i(location, value);
}

void set_shader_uniform(unsigned int shader, const std::string& name, float value) {
	auto location = glGetUniformLocation(shader, name.c_str());
	assert(location != -1);
	glUniform1f(location, value);
}

void set_shader_uniform(unsigned int shader, const std::string& name, const Toucan::Vector2f& value) {
	auto location = glGetUniformLocation(shader, name.c_str());
	assert(location != -1);
	glUniform2f(location, static_cast<float>(value.x()), static_cast<float>(value.y()));
}

void set_shader_uniform(unsigned int shader, const std::string& name, const Toucan::Vector3f& value) {
	auto location = glGetUniformLocation(shader, name.c_str());
	assert(location != -1);
	glUniform3f(location, static_cast<float>(value.x()), static_cast<float>(value.y()), static_cast<float>(value.z()));
}

void set_shader_uniform(unsigned int shader, const std::string& name, const Toucan::Color& value) {
	auto location = glGetUniformLocation(shader, name.c_str());
	assert(location != -1);
	glUniform3f(location, static_cast<float>(value.r), static_cast<float>(value.g), static_cast<float>(value.b));
}

void set_shader_uniform(unsigned int shader, const std::string& name, const Toucan::Matrix4f& value) {
	auto location = glGetUniformLocation(shader, name.c_str());
	assert(location != -1);
	
	glUniformMatrix4fv(location, 1, GL_TRUE, value.data());
}
