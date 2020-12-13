#pragma once

#include <cstdint>
#include <string>

#include "Toucan/DataTypes.h"

unsigned int compile_shader(const char* shader_source, uint32_t type);

unsigned int create_shader_program(const char* vertex_source, const char* fragment_source);
unsigned int create_shader_program(const char* vertex_source, const char* geometry_source, const char* fragment_source);

void set_shader_uniform(unsigned int shader, const std::string& name, int value);
void set_shader_uniform(unsigned int shader, const std::string& name, float value);
void set_shader_uniform(unsigned int shader, const std::string& name, const Toucan::Vector2f& value);
void set_shader_uniform(unsigned int shader, const std::string& name, const Toucan::Vector3f& value);
void set_shader_uniform(unsigned int shader, const std::string& name, const Toucan::Color& value);
void set_shader_uniform(unsigned int shader, const std::string& name, const Toucan::Matrix4f& value);
