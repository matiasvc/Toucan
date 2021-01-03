#pragma once

#include <vector>
#include <string>
#include <optional>


#include "Toucan/DataTypes.h"

namespace Toucan {

void Initialize(ToucanSettings settings = {});
void Destroy();
bool IsWindowOpen();

void SleepUntilWindowClosed();

// ***** Figure 2D *****

void BeginFigure2D(const std::string& name, const Figure2DSettings& settings = {});
void EndFigure2D();

// ***** Elements 2D *****

void PushPose2D(const Toucan::RigidTransform2Df& pose);
void PopPose2D();

void ShowLinePlot2D(const std::string& name, const Toucan::Buffer<Toucan::Vector2f>& line_buffer, int draw_layer = 0, const ShowLinePlot2DSettings& settings = {});
void ShowPoints2D(const std::string& name, const Toucan::Buffer<Toucan::Point2D>& points_buffer, int draw_layer = 0, const ShowPoints2DSettings& settings = {});
void ShowImage2D(const std::string& name, const Image2D& image, int draw_layer, const ShowImage2DSettings& settings = {});

// Helper functions
template <size_t N>
inline void ShowPoints2D(const std::string& name, const std::array<Toucan::Point2D, N>& points, int draw_layer = 0, const ShowPoints2DSettings& settings = {}) {
	Toucan::Buffer<Toucan::Point2D> buffer = {points.data(), points.size()};
	ShowPoints2D(name, buffer, draw_layer, settings);
}

template <typename Allocator>
inline void ShowPoints2D(const std::string& name, const std::vector<Toucan::Point2D, Allocator>& points, int draw_layer = 0, const ShowPoints2DSettings& settings = {}) {
	Toucan::Buffer<Toucan::Point2D> buffer = {points.data(), points.size()};
	ShowPoints2D(name, buffer, draw_layer, settings);
}

// ***** Figure 3D *****
void BeginFigure3D(const std::string& name, const Figure3DSettings& settings = {});
void EndFigure3D();

// ***** Elements 3D *****
void PushPose3D(const Toucan::RigidTransform3Df& pose);
void PopPose3D();

void ShowAxis3D(const std::string& name, const ShowAxis3DSettings& settings = {});
void ShowPoints3D(const std::string& name, const Toucan::Buffer<Toucan::Point3D>& points_buffer, const ShowPoints3DSettings& settings = {});
void ShowLines3D(const std::string& name, const Toucan::Buffer<Toucan::LineVertex3D>& lines_buffer, const ShowLines3DSettings& settings = {});
void ShowPrimitives3D(const std::string& name, const Toucan::Buffer<Toucan::Primitive3D>& primitives_buffer, const ShowPrimitives3DSettings& settings = {});

// Helper functions
template <size_t N>
inline void ShowLinePlot2D(const std::string& name, const std::array<Toucan::Vector2f, N>& points, int draw_layer = 0, const ShowLinePlot2DSettings& settings = {}) {
	Toucan::Buffer<Toucan::Vector2f> buffer = {points.data(), points.size()};
	ShowLinePlot2D(name, buffer, draw_layer, settings);
}

template <typename Allocator>
inline void ShowLinePlot2D(const std::string& name, const std::vector<Toucan::Vector2f, Allocator>& points, int draw_layer = 0, const ShowLinePlot2DSettings& settings = {}) {
	Toucan::Buffer<Toucan::Vector2f> buffer = {points.data(), points.size()};
	ShowLinePlot2D(name, buffer, draw_layer, settings);
}

template <size_t N>
inline void ShowLinePlot2D(const std::string& name, const std::array<float, N>& values, int draw_layer = 0, const ShowLinePlot2DSettings& settings = {}) {
	std::array<Toucan::Vector2f, N> points;
	
	for (int point_index = 0; point_index < N; ++point_index) {
		points[point_index] = Toucan::Vector2f(static_cast<float>(point_index), values[point_index]);
	}
	
	Toucan::Buffer<Toucan::Vector2f> buffer = {points.data(), points.size()};
	ShowLinePlot2D(name, buffer, draw_layer, settings);
}

template <typename Allocator>
inline void ShowLinePlot2D(const std::string& name, const std::vector<float, Allocator>& values, int draw_layer = 0, const ShowLinePlot2DSettings& settings = {}) {
	const auto number_of_points = values.size();
	std::vector<Toucan::Vector2f> points;
	points.reserve(number_of_points);
	
	for (int point_index = 0; point_index < number_of_points; ++point_index) {
		points.template emplace_back(static_cast<float>(point_index), values[point_index]);
	}
	
	Toucan::Buffer<Toucan::Vector2f> buffer = {points.data(), points.size()};
	ShowLinePlot2D(name, buffer, draw_layer, settings);
}

template <size_t N>
inline void ShowPoints3D(const std::string& name, const std::array<Toucan::Point3D, N>& points, const ShowPoints3DSettings& settings = {}) {
	Toucan::Buffer<Toucan::Point3D> buffer = {points.data(), points.size()};
	ShowPoints3D(name, buffer, settings);
}

template <typename Allocator>
inline void ShowPoints3D(const std::string& name, const std::vector<Toucan::Point3D, Allocator>& points, const ShowPoints3DSettings& settings = {}) {
	Toucan::Buffer<Toucan::Point3D> buffer = {points.data(), points.size()};
	ShowPoints3D(name, buffer, settings);
}

template <size_t N>
inline void ShowLines3D(const std::string& name, const std::array<Toucan::LineVertex3D, N>& points, const ShowLines3DSettings& settings = {}) {
	Toucan::Buffer<Toucan::LineVertex3D> buffer = {points.data(), points.size()};
	ShowLines3D(name, buffer, settings);
}

template <typename Allocator>
inline void ShowLines3D(const std::string& name, const std::vector<Toucan::LineVertex3D, Allocator>& points, const ShowLines3DSettings& settings = {}) {
	Toucan::Buffer<Toucan::LineVertex3D> buffer = {points.data(), points.size()};
	ShowLines3D(name, buffer, settings);
}

inline void ShowPrimitives3D(const std::string& name, const Toucan::Primitive3D& primitive, const ShowPrimitives3DSettings& settings = {}) {
	Toucan::Buffer<Toucan::Primitive3D> buffer = {&primitive, 1};
	ShowPrimitives3D(name, buffer, settings);
}

template <size_t N>
inline void ShowPrimitives3D(const std::string& name, const std::array<Toucan::Primitive3D, N>& primitives, const ShowPrimitives3DSettings& settings = {}) {
	Toucan::Buffer<Toucan::Primitive3D> buffer = {primitives.data(), primitives.size()};
	ShowPrimitives3D(name, buffer, settings);
}

template <typename Allocator>
inline void ShowPrimitives3D(const std::string& name, const std::vector<Toucan::Primitive3D, Allocator>& primitives, const ShowPrimitives3DSettings& settings = {}) {
	Toucan::Buffer<Toucan::Primitive3D> buffer = {primitives.data(), primitives.size()};
	ShowPrimitives3D(name, buffer, settings);
}
} // namespace Toucan
