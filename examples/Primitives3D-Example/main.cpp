#include <Toucan/Toucan.h>

#include <cmath>

#include <thread>
#include <chrono>

int main() {
	
	Toucan::Initialize();
	
	float time = 0.0f;
	
	while (Toucan::IsWindowOpen()) {
		
		float sphere_size_x = 0.5f + 0.25f*std::sin(time);
		float cube_angle = time * M_PI;
		
		Toucan::BeginFigure3D("Primitives 3D");
		{
			std::array<Toucan::Primitive3D, 3> primitives = {
					Toucan::Primitive3D(
							Toucan::PrimitiveType::Sphere,
							Toucan::ScaledTransform3D(Toucan::Quaternionf::Identity(), Toucan::Vector3f::UnitX(), Toucan::Vector3f(sphere_size_x, 1.0f, 1.0f)),
							Toucan::Color::Red()),
					Toucan::Primitive3D(
							Toucan::PrimitiveType::Cylinder,
							Toucan::ScaledTransform3D(Toucan::Quaternionf::Identity(), Toucan::Vector3f::UnitY()),
							Toucan::Color::Green()),
					Toucan::Primitive3D(
							Toucan::PrimitiveType::Cube,
							Toucan::ScaledTransform3D(Toucan::Quaternionf(Toucan::Vector3f::Ones().normalized(), cube_angle), Toucan::Vector3f::UnitZ()),
							Toucan::Color::Blue()),
			};
			
			Toucan::ShowPrimitives3D("primitives", primitives);
		}
		Toucan::EndFigure3D();
		
		using namespace std::literals;
		std::this_thread::sleep_for(50ms);
		
		time += 50*(1.0f/1000.0f);
	}
	
	
	Toucan::Destroy();
	
	return 0;
}
