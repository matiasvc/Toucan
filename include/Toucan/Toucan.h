#pragma once

#include <cstddef>
#include <vector>
#include <string>
#include <optional>

#include <Toucan/Setting.h>
#include <Toucan/DataTypes.h>

namespace Toucan {

void Initialize(ToucanSettings settings = {});
void Destroy();
bool IsWindowOpen();

void SleepUntilWindowClosed();

// ***** Figure 2D *****
Figure2DSettingsBuilder BeginFigure2D(const std::string& name);
void EndFigure2D();

// ***** Elements 2D *****
void PushPose2D(const RigidTransform2Df& pose);
void PopPose2D();
void ClearPose2D();

ShowLinePlot2DSettingsBuilder ShowLinePlot2D(const std::string& name, const Buffer<Vector2f>& line_buffer, int draw_layer = 0);
ShowPoints2DSettingsBuilder   ShowPoints2D(const std::string& name, const Buffer<Point2D>& points_buffer, int draw_layer = 0);
ShowImage2DSettingsBuilder    ShowImage2D(const std::string& name, const Image2D& image, int draw_layer = 0);

// Helper functions
template <size_t N>
inline ShowPoints2DSettingsBuilder ShowPoints2D(const std::string& name, const std::array<Point2D, N>& points, int draw_layer = 0) {
	Buffer<Point2D> buffer = {points.data(), points.size()};
	return ShowPoints2D(name, buffer, draw_layer);
}

template <typename Allocator>
inline ShowPoints2DSettingsBuilder ShowPoints2D(const std::string& name, const std::vector<Point2D, Allocator>& points, int draw_layer = 0) {
	Buffer<Point2D> buffer = {points.data(), points.size()};
	return ShowPoints2D(name, buffer, draw_layer);
}

// ***** Figure 3D *****
Figure3DSettingsBuilder BeginFigure3D(const std::string& name);
void EndFigure3D();

// ***** Elements 3D *****
void PushPose3D(const RigidTransform3Df& pose);
void PopPose3D();
void ClearPose3D();

ShowAxis3DSettingsBuilder       ShowAxis3D(const std::string& name); // TODO(Matias): Replace with setting in PushPose3D
ShowPoints3DSettingsBuilder     ShowPoints3D(const std::string& name, const Buffer<Point3D>& points_buffer);
ShowLines3DSettingsBuilder      ShowLines3D(const std::string& name, const Buffer<LineVertex3D>& lines_buffer);
ShowPrimitives3DSettingsBuilder ShowPrimitives3D(const std::string& name, const Buffer<Primitive3D>& primitives_buffer);

// ***** Input *****
InputSettingsBuilder BeginInputWindow(const std::string& name);
void EndInputWindow();

// ***** Elements Input *****
ShowButtonsSettingsBuilder ShowButton(const std::string& name);

ShowCheckboxSettingsBuilder ShowCheckbox(const std::string& name, bool& value);

ShowSliderFloatSettingsBuilder ShowSliderFloat(const std::string& name, float& value);
ShowSliderFloatSettingsBuilder ShowSliderFloat2(const std::string& name, Vector2f& value);
ShowSliderFloatSettingsBuilder ShowSliderFloat3(const std::string& name, Vector3f& value);
ShowSliderFloatSettingsBuilder ShowSliderFloat4(const std::string& name, Vector4f& value);

ShowSliderIntSettingsBuilder ShowSliderInt(const std::string& name, int& value);
ShowSliderIntSettingsBuilder ShowSliderInt2(const std::string& name, Vector2i& value);
ShowSliderIntSettingsBuilder ShowSliderInt3(const std::string& name, Vector3i& value);
ShowSliderIntSettingsBuilder ShowSliderInt4(const std::string& name, Vector4i& value);

ShowColorPickerSettingsBuilder ShowColorPicker(const std::string& name, Color& value);

// Helper functions
template <size_t N>
inline ShowLinePlot2DSettingsBuilder ShowLinePlot2D(const std::string& name, const std::array<Vector2f, N>& points, int draw_layer = 0) {
	Buffer<Vector2f> buffer = {points.data(), points.size()};
	return ShowLinePlot2D(name, buffer, draw_layer);
}

template <typename Allocator>
inline ShowLinePlot2DSettingsBuilder ShowLinePlot2D(const std::string& name, const std::vector<Vector2f, Allocator>& points, int draw_layer = 0) {
	Buffer<Vector2f> buffer = {points.data(), points.size()};
	return ShowLinePlot2D(name, buffer, draw_layer);
}

template <size_t N>
inline ShowLinePlot2DSettingsBuilder ShowLinePlot2D(const std::string& name, const std::array<float, N>& values, int draw_layer = 0) {
	std::array<Vector2f, N> points;
	
	for (int point_index = 0; point_index < N; ++point_index) {
		points[point_index] = Vector2f(static_cast<float>(point_index), values[point_index]);
	}
	
	Buffer<Vector2f> buffer = {points.data(), points.size()};
	return ShowLinePlot2D(name, buffer, draw_layer);
}

template <typename Allocator>
inline ShowLinePlot2DSettingsBuilder ShowLinePlot2D(const std::string& name, const std::vector<float, Allocator>& values, int draw_layer = 0) {
	const auto number_of_points = values.size();
	std::vector<Vector2f> points;
	points.reserve(number_of_points);
	
	for (int point_index = 0; point_index < number_of_points; ++point_index) {
		points.template emplace_back(static_cast<float>(point_index), values[point_index]);
	}
	
	Buffer<Vector2f> buffer = {points.data(), points.size()};
	return ShowLinePlot2D(name, buffer, draw_layer);
}

template <size_t N>
inline ShowPoints3DSettingsBuilder ShowPoints3D(const std::string& name, const std::array<Point3D, N>& points) {
	Buffer<Point3D> buffer = {points.data(), points.size()};
	return ShowPoints3D(name, buffer);
}

template <typename Allocator>
inline ShowPoints3DSettingsBuilder ShowPoints3D(const std::string& name, const std::vector<Point3D, Allocator>& points) {
	Buffer<Point3D> buffer = {points.data(), points.size()};
	return ShowPoints3D(name, buffer);
}

template <size_t N>
inline ShowLines3DSettingsBuilder ShowLines3D(const std::string& name, const std::array<LineVertex3D, N>& points) {
	Buffer<LineVertex3D> buffer = {points.data(), points.size()};
	return ShowLines3D(name, buffer);
}

template <typename Allocator>
inline ShowLines3DSettingsBuilder ShowLines3D(const std::string& name, const std::vector<LineVertex3D, Allocator>& points) {
	Buffer<LineVertex3D> buffer = {points.data(), points.size()};
	return ShowLines3D(name, buffer);
}

inline ShowPrimitives3DSettingsBuilder ShowPrimitives3D(const std::string& name, const Primitive3D& primitive) {
	Buffer<Primitive3D> buffer = {&primitive, 1};
	return ShowPrimitives3D(name, buffer);
}

template <size_t N>
inline ShowPrimitives3DSettingsBuilder ShowPrimitives3D(const std::string& name, const std::array<Primitive3D, N>& primitives) {
	Buffer<Primitive3D> buffer = {primitives.data(), primitives.size()};
	return ShowPrimitives3D(name, buffer);
}

template <typename Allocator>
inline ShowPrimitives3DSettingsBuilder ShowPrimitives3D(const std::string& name, const std::vector<Primitive3D, Allocator>& primitives) {
	Buffer<Primitive3D> buffer = {primitives.data(), primitives.size()};
	return ShowPrimitives3D(name, buffer);
}
} // namespace Toucan
