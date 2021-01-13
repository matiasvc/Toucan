#include <Toucan/Toucan.h>

#include <chrono>
#include <thread>

int main() {
	
	Toucan::Initialize();
	
	bool checkbox_value = true;
	
	float slider_float_value = 0.0;
	Toucan::Vector2f slider_float2_value = Toucan::Vector2f::Zero();
	Toucan::Vector3f slider_float3_value = Toucan::Vector3f::Zero();
	Toucan::Vector4f slider_float4_value = Toucan::Vector4f::Zero();
	
	int slider_int_value = 0;
	Toucan::Vector2i slider_int2_value = Toucan::Vector2i::Zero();
	Toucan::Vector3i slider_int3_value = Toucan::Vector3i::Zero();
	Toucan::Vector4i slider_int4_value = Toucan::Vector4i::Zero();
	
	while (Toucan::IsWindowOpen()) {
		Toucan::BeginInputWindow("Inputs");
		{
			if (Toucan::ShowButton("Button")) {
				std::cout << "Button clicked\n";
			}
			
			if (Toucan::ShowCheckbox("Checkbox", checkbox_value)) {
				std::cout << "Checkbox value changed to " << checkbox_value << '\n';
			}
			
			if (Toucan::ShowSliderFloat("Slider float", slider_float_value)) {
				std::cout << "Slider float value changed to " << slider_float_value << '\n';
			}
			
			if (Toucan::ShowSliderFloat2("Slider float 2", slider_float2_value)) {
				std::cout << "Slider float 2 value changed to " << slider_float2_value << '\n';
			}
			
			if (Toucan::ShowSliderFloat3("Slider float 3", slider_float3_value)) {
				std::cout << "Slider float 3 value changed to " << slider_float3_value << '\n';
			}
			
			if (Toucan::ShowSliderFloat4("Slider float 4", slider_float4_value)) {
				std::cout << "Slider float 4 value changed to " << slider_float4_value << '\n';
			}
			
			if (Toucan::ShowSliderInt("Slider int", slider_int_value)) {
				std::cout << "Slider int value changed to " << slider_int_value << '\n';
			}
			
			if (Toucan::ShowSliderInt2("Slider int 2", slider_int2_value)) {
				std::cout << "Slider int 2 value changed to " << slider_int2_value << '\n';
			}
			
			if (Toucan::ShowSliderInt3("Slider int 3", slider_int3_value)) {
				std::cout << "Slider int 3 value changed to " << slider_int3_value << '\n';
			}
			
			if (Toucan::ShowSliderInt4("Slider int 4", slider_int4_value)) {
				std::cout << "Slider int 4 value changed to " << slider_int4_value << '\n';
			}
		}
		Toucan::EndInputWindow();
		
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(250ms);
	}
	
	Toucan::Destroy();
	
	return 0;
}
