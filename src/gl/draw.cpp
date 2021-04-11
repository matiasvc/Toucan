#include "draw.h"

#include <glad/glad.h>

void create_or_resize_framebuffer(unsigned int* framebuffer, unsigned int* framebuffer_color_texture, unsigned int* framebuffer_depth_texture, Toucan::Vector2i size) {
	
	glDeleteFramebuffers(1, framebuffer);
	glDeleteTextures(1, framebuffer_color_texture);
	
	if (framebuffer_depth_texture != nullptr) {
		glDeleteTextures(1, framebuffer_depth_texture);
	}
	
	glGenFramebuffers(1, framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
	
	glGenTextures(1, framebuffer_color_texture);
	glBindTexture(GL_TEXTURE_2D, *framebuffer_color_texture);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x(), size.y(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *framebuffer_color_texture, 0);
	
	if (framebuffer_depth_texture != nullptr) {
		glGenTextures(1, framebuffer_depth_texture);
		glBindTexture(GL_TEXTURE_2D, *framebuffer_depth_texture);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, size.x(), size.y(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *framebuffer_depth_texture, 0);
	}
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("ERROR! Unable to create framebuffer.");
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
