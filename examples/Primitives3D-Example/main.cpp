#include <Toucan/Toucan.h>

#include <thread>
#include <chrono>

int main() {
	
	Toucan::Initialize();
	
	while (Toucan::IsWindowOpen()) {
		Toucan::BeginFigure3D("Primitives 3D");
		{
			std::array<Toucan::Primitive3D, 3> primitives = {
					Toucan::Primitive3D(Toucan::PrimitiveType::Sphere, Toucan::ScaledTransform3D(Toucan::Quaternionf::Identity(), Toucan::Vector3f::UnitX()), Toucan::Color::Red()),
					Toucan::Primitive3D(Toucan::PrimitiveType::Cylinder, Toucan::ScaledTransform3D(Toucan::Quaternionf::Identity(), Toucan::Vector3f::UnitY()), Toucan::Color::Green()),
					Toucan::Primitive3D(Toucan::PrimitiveType::Cube, Toucan::ScaledTransform3D(Toucan::Quaternionf::Identity(), Toucan::Vector3f::UnitZ()), Toucan::Color::Blue()),
			};
			
			Toucan::ShowPrimitives3D("primitives", primitives);
		}
		Toucan::EndFigure3D();
	}
	
	Toucan::Destroy();
	
	return 0;
}
