#include <array>

#include <Toucan/Toucan.h>

void draw_figure3d(const std::string& figure_name, Toucan::Orientation orientation, Toucan::Handedness handedness) {
	std::array<Toucan::Primitive3D, 3> primitive_array = {
		Toucan::Primitive3D(
				Toucan::PrimitiveType::Cylinder,
				Toucan::ScaledTransform3Df(Toucan::Quaternionf::Identity(), 2.0f*Toucan::Vector3f::UnitX()),
				Toucan::Color::Red()
		),
		Toucan::Primitive3D(
				Toucan::PrimitiveType::Cube,
				Toucan::ScaledTransform3Df(Toucan::Quaternionf::Identity(), 2.0f*Toucan::Vector3f::UnitY()),
				Toucan::Color::Green()
		),
		Toucan::Primitive3D(
				Toucan::PrimitiveType::Sphere,
				Toucan::ScaledTransform3Df(Toucan::Quaternionf::Identity(), 2.0f*Toucan::Vector3f::UnitZ()),
				Toucan::Color::Blue()
		),
	};
	
	Toucan::BeginFigure3D(figure_name)
		.SetOrientation(orientation)
		.SetHandedness(handedness);
	{
		Toucan::ShowPrimitives3D("Primitives", primitive_array);
	}
	Toucan::EndFigure3D();
}

int main() {
	
	Toucan::Initialize();
	
	// X
	draw_figure3d("X up:R", Toucan::Orientation::X_UP, Toucan::Handedness::RIGHT_HANDED);
	draw_figure3d("X up:L", Toucan::Orientation::X_UP, Toucan::Handedness::LEFT_HANDED);
	draw_figure3d("X down:R", Toucan::Orientation::X_DOWN, Toucan::Handedness::RIGHT_HANDED);
	draw_figure3d("X down:L", Toucan::Orientation::X_DOWN, Toucan::Handedness::LEFT_HANDED);
	
	// Y
	draw_figure3d("Y up:R", Toucan::Orientation::Y_UP, Toucan::Handedness::RIGHT_HANDED);
	draw_figure3d("Y up:L", Toucan::Orientation::Y_UP, Toucan::Handedness::LEFT_HANDED);
	draw_figure3d("Y down:R", Toucan::Orientation::Y_DOWN, Toucan::Handedness::RIGHT_HANDED);
	draw_figure3d("Y down:L", Toucan::Orientation::Y_DOWN, Toucan::Handedness::LEFT_HANDED);
	
	// Z
	draw_figure3d("Z up:R", Toucan::Orientation::Z_UP, Toucan::Handedness::RIGHT_HANDED);
	draw_figure3d("Z up:L", Toucan::Orientation::Z_UP, Toucan::Handedness::LEFT_HANDED);
	draw_figure3d("Z down:R", Toucan::Orientation::Z_DOWN, Toucan::Handedness::RIGHT_HANDED);
	draw_figure3d("Z down:L", Toucan::Orientation::Z_DOWN, Toucan::Handedness::LEFT_HANDED);
	
	Toucan::SleepUntilWindowClosed();
	
	Toucan::Destroy();
}
