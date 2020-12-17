#include <Toucan/Toucan.h>

#include <limits>

#include <cstdlib>
#include <cstdint>

constexpr int image_width = 640;
constexpr int image_height = 480;

template <typename T>
void compute_rgb_test_pattern(void* data_ptr, T max_value) {
	
	for (uint64_t v = 0; v < image_height; ++v) {
		for (uint64_t u = 0; u < image_width; ++u) {
			T* img_ptr = &reinterpret_cast<T*>(data_ptr)[v*image_width*3 + u*3];
			T* r_ptr = img_ptr++;
			T* g_ptr = img_ptr++;
			T* b_ptr = img_ptr++;
			
			if (v < 360) {
				if (u < 100) {
					*r_ptr = max_value;
					*g_ptr = 0;
					*b_ptr = 0;
				} else if (u < 200) {
					*r_ptr = 0;
					*g_ptr = max_value;
					*b_ptr = 0;
				} else if (u < 300) {
					*r_ptr = 0;
					*g_ptr = 0;
					*b_ptr = max_value;
				} else {
					*r_ptr = (u/10 % 2) ? max_value : 0;
					*g_ptr = (v/10 % 2) ? max_value : 0;
					*b_ptr = 0;
				}
			} else if (v < 400) {
				*r_ptr = max_value*u/image_width;
				*g_ptr = 0;
				*b_ptr = 0;
			} else if (v < 440) {
				*r_ptr = 0;
				*g_ptr = max_value*u/image_width;
				*b_ptr = 0;
			} else {
				*r_ptr = 0;
				*g_ptr = 0;
				*b_ptr = max_value*u/image_width;
			}
		}
	}
}

template <typename T>
void compute_rg_test_pattern(void* data_ptr, T max_value) {
	
	for (uint64_t v = 0; v < image_height; ++v) {
		for (uint64_t u = 0; u < image_width; ++u) {
			T* img_ptr = &reinterpret_cast<T*>(data_ptr)[v*image_width*2 + u*2];
			T* r_ptr = img_ptr++;
			T* g_ptr = img_ptr++;
			
			*r_ptr = max_value*u/image_width;
			*g_ptr = max_value*v/image_height;
		}
	}
}

int main() {
	
	Toucan::Initialize();
	
	{
		Toucan::Image2D rg_u8 = {};
		rg_u8.image_buffer_ptr = std::malloc(image_width * image_height * 2 * sizeof(uint8_t));
		rg_u8.width = image_width;
		rg_u8.height = image_height;
		rg_u8.format = Toucan::ImageFormat::RG_U8;
		
		compute_rg_test_pattern<uint8_t>(rg_u8.image_buffer_ptr, std::numeric_limits<uint8_t>::max());
		
		Toucan::BeginFigure2D("RG U8");
		Toucan::ShowImage2D("Image", rg_u8, 0);
		Toucan::EndFigure2D();
		
		std::free(rg_u8.image_buffer_ptr);
	}
	
	{
		Toucan::Image2D rg_u16 = {};
		rg_u16.image_buffer_ptr = std::malloc(image_width * image_height * 2 * sizeof(uint16_t));
		rg_u16.width = image_width;
		rg_u16.height = image_height;
		rg_u16.format = Toucan::ImageFormat::RG_U16;
		
		compute_rg_test_pattern<uint16_t>(rg_u16.image_buffer_ptr, std::numeric_limits<uint16_t>::max());
		
		Toucan::BeginFigure2D("RG U16");
		Toucan::ShowImage2D("Image", rg_u16, 0);
		Toucan::EndFigure2D();
		
		std::free(rg_u16.image_buffer_ptr);
	}
	
	{
		Toucan::Image2D rg_u32 = {};
		rg_u32.image_buffer_ptr = std::malloc(image_width * image_height * 2 * sizeof(uint32_t));
		rg_u32.width = image_width;
		rg_u32.height = image_height;
		rg_u32.format = Toucan::ImageFormat::RG_U32;
		
		compute_rg_test_pattern<uint32_t>(rg_u32.image_buffer_ptr, std::numeric_limits<uint32_t>::max());
		
		Toucan::BeginFigure2D("RG U32");
		Toucan::ShowImage2D("Image", rg_u32, 0);
		Toucan::EndFigure2D();
		
		std::free(rg_u32.image_buffer_ptr);
	}
	
	{
		Toucan::Image2D rg_f32 = {};
		rg_f32.image_buffer_ptr = std::malloc(image_width * image_height * 2 * sizeof(float));
		rg_f32.width = image_width;
		rg_f32.height = image_height;
		rg_f32.format = Toucan::ImageFormat::RG_F32;
		
		compute_rg_test_pattern<float>(rg_f32.image_buffer_ptr, 1.0f);
		
		Toucan::BeginFigure2D("RG F32");
		Toucan::ShowImage2D("Image", rg_f32, 0);
		Toucan::EndFigure2D();
		
		std::free(rg_f32.image_buffer_ptr);
	}
	
	{
		Toucan::Image2D rgb_u8 = {};
		rgb_u8.image_buffer_ptr = std::malloc(image_width * image_height * 3 * sizeof(uint8_t));
		rgb_u8.width = image_width;
		rgb_u8.height = image_height;
		rgb_u8.format = Toucan::ImageFormat::RGB_U8;
		
		compute_rgb_test_pattern<uint8_t>(rgb_u8.image_buffer_ptr, std::numeric_limits<uint8_t>::max());
		
		Toucan::BeginFigure2D("RGB U8");
		Toucan::ShowImage2D("Image", rgb_u8, 0);
		Toucan::EndFigure2D();
		
		std::free(rgb_u8.image_buffer_ptr);
	}
	
	{
		Toucan::Image2D rgb_u16 = {};
		rgb_u16.image_buffer_ptr = std::malloc(image_width * image_height * 3 * sizeof(uint16_t));
		rgb_u16.width = image_width;
		rgb_u16.height = image_height;
		rgb_u16.format = Toucan::ImageFormat::RGB_U16;
		
		compute_rgb_test_pattern<uint16_t>(rgb_u16.image_buffer_ptr, std::numeric_limits<uint16_t>::max());
		
		Toucan::BeginFigure2D("RGB U16");
		Toucan::ShowImage2D("Image", rgb_u16, 0);
		Toucan::EndFigure2D();
		
		std::free(rgb_u16.image_buffer_ptr);
	}
	
	{
		Toucan::Image2D rgb_u32 = {};
		rgb_u32.image_buffer_ptr = std::malloc(image_width * image_height * 3 * sizeof(uint32_t));
		rgb_u32.width = image_width;
		rgb_u32.height = image_height;
		rgb_u32.format = Toucan::ImageFormat::RGB_U32;
		
		compute_rgb_test_pattern<uint32_t>(rgb_u32.image_buffer_ptr, std::numeric_limits<uint32_t>::max());
		
		Toucan::BeginFigure2D("RGB U32");
		Toucan::ShowImage2D("Image", rgb_u32, 0);
		Toucan::EndFigure2D();
		
		std::free(rgb_u32.image_buffer_ptr);
	}
	
	{
		Toucan::Image2D rgb_f32 = {};
		rgb_f32.image_buffer_ptr = std::malloc(image_width * image_height * 3 * sizeof(float));
		rgb_f32.width = image_width;
		rgb_f32.height = image_height;
		rgb_f32.format = Toucan::ImageFormat::RGB_F32;
		
		compute_rgb_test_pattern<float>(rgb_f32.image_buffer_ptr, 1.0f);
		
		Toucan::BeginFigure2D("RGB F32");
		Toucan::ShowImage2D("Image", rgb_f32, 0);
		Toucan::EndFigure2D();
		
		std::free(rgb_f32.image_buffer_ptr);
	}
	
	Toucan::SleepUntilWindowClosed();
	
	Toucan::Destroy();
	
	return 0;
}
