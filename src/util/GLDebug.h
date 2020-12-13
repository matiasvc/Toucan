#pragma once

#ifdef NDEBUG // Release

#define glCheckError() ((void)0)

#else // Debug

#include <string>
#include <sstream>

#include <glad/glad.h>

inline void glCheckError_(const char *file, int line) {
	std::ostringstream ss;
	ss << "OpenGL Error: \n";
	GLenum errorCode;
	
	bool error = false;
	
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		error = true;
		std::string error_string;
		switch (errorCode)
		{
			case GL_INVALID_ENUM:                  error_string = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error_string = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error_string = "INVALID_OPERATION"; break;
			case GL_OUT_OF_MEMORY:                 error_string = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error_string = "INVALID_FRAMEBUFFER_OPERATION"; break;
			default:                               error_string = "UNKNOWN_ERROR"; break;
		}
		ss << "\t- " << error_string << " | " << file << " (line: " << line << ")\n";
	}
	if (error) {
		throw std::runtime_error(ss.str());
	}
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

#endif

