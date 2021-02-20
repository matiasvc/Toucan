#include <array>

#include <Toucan/Toucan.h>

void draw_figure3d(const std::string& figure_name, const Toucan::Figure3DSettings& settings) {
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
	
	Toucan::BeginFigure3D(figure_name, settings);
	{
		Toucan::ShowPrimitives3D("Primitives", primitive_array);
	}
	Toucan::EndFigure3D();
}

int main() {
	
	Toucan::Initialize();
	
	// X
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::X_UP;
		settings.handedness = Toucan::Handedness::RIGHT_HANDED;
		draw_figure3d("X up:R", settings);
	}
	
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::X_UP;
		settings.handedness = Toucan::Handedness::LEFT_HANDED;
		draw_figure3d("X up:L", settings);
	}
	
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::X_DOWN;
		settings.handedness = Toucan::Handedness::RIGHT_HANDED;
		draw_figure3d("X down:R", settings);
	}
	
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::X_DOWN;
		settings.handedness = Toucan::Handedness::LEFT_HANDED;
		draw_figure3d("X down:L", settings);
	}
	
	// Y
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::Y_UP;
		settings.handedness = Toucan::Handedness::RIGHT_HANDED;
		draw_figure3d("Y up:R", settings);
	}
	
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::Y_UP;
		settings.handedness = Toucan::Handedness::LEFT_HANDED;
		draw_figure3d("Y up:L", settings);
	}
	
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::Y_DOWN;
		settings.handedness = Toucan::Handedness::RIGHT_HANDED;
		draw_figure3d("Y down:R", settings);
	}
	
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::Y_DOWN;
		settings.handedness = Toucan::Handedness::LEFT_HANDED;
		draw_figure3d("Y down:L", settings);
	}
	
	// Z
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::Z_UP;
		settings.handedness = Toucan::Handedness::RIGHT_HANDED;
		draw_figure3d("Z up:R", settings);
	}
	
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::Z_UP;
		settings.handedness = Toucan::Handedness::LEFT_HANDED;
		draw_figure3d("Z up:L", settings);
	}
	
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::Z_DOWN;
		settings.handedness = Toucan::Handedness::RIGHT_HANDED;
		draw_figure3d("Z down:R", settings);
	}
	
	{
		Toucan::Figure3DSettings settings;
		settings.orientation = Toucan::Orientation::Z_DOWN;
		settings.handedness = Toucan::Handedness::LEFT_HANDED;
		draw_figure3d("Z down:L", settings);
	}
	
	Toucan::SleepUntilWindowClosed();
	
	Toucan::Destroy();
}
