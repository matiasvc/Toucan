#pragma once

struct Image {
	int width = 0;
	int height = 0;
	int channels = 0;
	void* buffer_ptr = nullptr;
};
