#include <Toucan/Toucan.h>

#include <cmath>


std::vector<float> compute_data() {
	constexpr size_t number_of_points = 250;
	
	std::vector<float> data;
	data.reserve(number_of_points);
	
	for (int i = 0; i < number_of_points; ++i) {
		data.emplace_back(std::cos((7 * M_PI / number_of_points) * static_cast<float>(i)));
	}
	
	return data;
}

int main() {
	
	const auto data = compute_data();
	
	Toucan::Initialize();
	
	Toucan::BeginFigure2D("Simple Line Plot");
	{
		Toucan::ShowLinePlot2DSettings line_plot_2d_settings;
		line_plot_2d_settings.line_color = Toucan::Color::Red();
		Toucan::ShowLinePlot2D("Plot", data, 0, line_plot_2d_settings);
	}
	Toucan::EndFigure2D();
	
	Toucan::SleepUntilWindowClosed();
	
	return 0;
}
