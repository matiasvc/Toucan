#include <Toucan/Toucan.h>

#include <cmath>

int main() {
	
	std::vector<float> ys;
	
	constexpr size_t number_of_points = 250;
	
	ys.reserve(number_of_points);
	
	for (int i = 0; i < number_of_points; ++i) {
		ys.emplace_back(std::cos((7*M_PI/number_of_points)*static_cast<float>(i)));
	}
	
	Toucan::Initialize();
	
	Toucan::BeginFigure2D("Simple Line Plot");
	
	Toucan::ShowLinePlot2DSettings line_plot_2d_settings;
	line_plot_2d_settings.line_color = Toucan::Color::Red();
	Toucan::ShowLinePlot2D("Plot", ys, 0, line_plot_2d_settings);
	Toucan::EndFigure2D();
	
	Toucan::SleepUntilWindowClosed();
	
	return 0;
}
