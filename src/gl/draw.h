#pragma once

#include <Toucan/LinAlg.h>

void create_or_resize_framebuffer(unsigned int* framebuffer, unsigned int* framebuffer_color_texture, unsigned int* framebuffer_depth_texture, Toucan::Vector2i size);
