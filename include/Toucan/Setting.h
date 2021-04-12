#pragma once

#include <Toucan/DataTypes.h>
#include <Toucan/LinAlg.h>

namespace Toucan {

struct ToucanSettings {
	unsigned int width = 1600;
	unsigned int height = 1200;
	bool resizeable = true;
	bool floating = false;
	float max_frames_per_second = 60.0f;
};


enum class YAxisDirection {
	UP, DOWN
};

struct Figure2DSettings {
	float figure_padding = 5.0f;
	float axis_x_size = 35.0f;
	float axis_y_size = 60.0f;
	float tick_width = 8.0f;
	YAxisDirection y_axis_direction = YAxisDirection::UP;
};

class Figure2DSettingsBuilder {
public:
	explicit Figure2DSettingsBuilder(Figure2DSettings* settings_ptr)
	: m_settings_ptr{settings_ptr} {}
	
	Figure2DSettingsBuilder(const Figure2DSettingsBuilder&) = delete;
	Figure2DSettingsBuilder(Figure2DSettingsBuilder&&) = delete;
	Figure2DSettingsBuilder& operator=(const Figure2DSettingsBuilder&) = delete;
	Figure2DSettingsBuilder& operator=(const Figure2DSettingsBuilder&&) = delete;
	
	Figure2DSettingsBuilder& SetFigurePadding(float padding) { m_settings_ptr->figure_padding = padding; return *this; }
	Figure2DSettingsBuilder& SetAxisXSize(float axis_size) { m_settings_ptr->axis_x_size = axis_size; return *this; }
	Figure2DSettingsBuilder& SetAxisYSize(float axis_size) { m_settings_ptr->axis_y_size = axis_size; return *this; }
	Figure2DSettingsBuilder& SetTickWidth(float tick_width) { m_settings_ptr->tick_width = tick_width; return *this; }
	Figure2DSettingsBuilder& SetYAxisDirection(YAxisDirection y_axis_direction) { m_settings_ptr->y_axis_direction = y_axis_direction; return *this; }
	
private:
	Figure2DSettings* m_settings_ptr;
};

enum class Orientation {
	X_UP, X_DOWN,
	Y_UP, Y_DOWN,
	Z_UP, Z_DOWN
};

enum class Handedness {
	RIGHT_HANDED,
	LEFT_HANDED
};

struct Figure3DSettings {
	Orientation orientation = Orientation::Z_UP;
	Handedness handedness = Handedness::RIGHT_HANDED;
	float near_clip = 0.01f;
	float far_clip = 200.0f;
	bool gizmo_enabled = true;
};

class Figure3DSettingsBuilder {
public:
	explicit Figure3DSettingsBuilder(Figure3DSettings* settings_ptr)
			: m_settings_ptr{settings_ptr} {}
	
	Figure3DSettingsBuilder(const Figure3DSettingsBuilder&) = delete;
	Figure3DSettingsBuilder(Figure3DSettingsBuilder&&) = delete;
	Figure3DSettingsBuilder& operator=(const Figure3DSettingsBuilder&) = delete;
	Figure3DSettingsBuilder& operator=(const Figure3DSettingsBuilder&&) = delete;
	
	Figure3DSettingsBuilder& SetOrientation(Orientation orientation) { m_settings_ptr->orientation = orientation; return *this; }
	Figure3DSettingsBuilder& SetHandedness(Handedness handedness) { m_settings_ptr->handedness = handedness; return *this; }
	Figure3DSettingsBuilder& SetNearClip(float near_clip) { m_settings_ptr->near_clip = near_clip; return *this; }
	Figure3DSettingsBuilder& SetFarClip(float far_clip) { m_settings_ptr->far_clip = far_clip; return *this; }
	Figure3DSettingsBuilder& SetGizmoEnabled(bool gizmo_enabled) { m_settings_ptr->gizmo_enabled = gizmo_enabled; return *this; }
private:
	Figure3DSettings* m_settings_ptr;
};

struct InputSettings {

};

class InputSettingsBuilder {
public:
	explicit InputSettingsBuilder(InputSettings* settings_ptr)
			: m_settings_ptr{settings_ptr} {}
	
	InputSettingsBuilder(const InputSettingsBuilder&) = delete;
	InputSettingsBuilder(InputSettingsBuilder&&) = delete;
	InputSettingsBuilder& operator=(const InputSettingsBuilder&) = delete;
	InputSettingsBuilder& operator=(const InputSettingsBuilder&&) = delete;
	
private:
	InputSettings* m_settings_ptr;
};

// Line Plot 2D

struct ShowLinePlot2DSettings {
	ScaledTransform2Df transform;
	Color line_color = Color::White();
	float line_width = 3.0f;
};

class ShowLinePlot2DSettingsBuilder {
public:
	explicit ShowLinePlot2DSettingsBuilder(ShowLinePlot2DSettings* settings_ptr)
	: m_settings_ptr{settings_ptr} {}
	
	ShowLinePlot2DSettingsBuilder(const ShowLinePlot2DSettingsBuilder&) = delete;
	ShowLinePlot2DSettingsBuilder(ShowLinePlot2DSettingsBuilder&&) = delete;
	ShowLinePlot2DSettingsBuilder& operator=(const ShowLinePlot2DSettingsBuilder&) = delete;
	ShowLinePlot2DSettingsBuilder& operator=(const ShowLinePlot2DSettingsBuilder&&) = delete;
	
	ShowLinePlot2DSettingsBuilder& SetTransform(const ScaledTransform2Df& transform) { m_settings_ptr->transform = transform; return *this; }
	ShowLinePlot2DSettingsBuilder& SetLineColor(const Color& color) { m_settings_ptr->line_color = color; return *this; }
	ShowLinePlot2DSettingsBuilder& SetLineWidth(float width) { m_settings_ptr->line_width = width; return *this; }
	
private:
	ShowLinePlot2DSettings* m_settings_ptr;
};

// Points 2D

struct ShowPoints2DSettings {
	ScaledTransform2Df transform;
};

class ShowPoints2DSettingsBuilder {
public:
	explicit ShowPoints2DSettingsBuilder(ShowPoints2DSettings* settings_ptr)
	: m_settings_ptr{settings_ptr} {}
	
	ShowPoints2DSettingsBuilder(const ShowPoints2DSettingsBuilder&) = delete;
	ShowPoints2DSettingsBuilder(ShowPoints2DSettingsBuilder&&) = delete;
	ShowPoints2DSettingsBuilder& operator=(const ShowPoints2DSettingsBuilder&) = delete;
	ShowPoints2DSettingsBuilder& operator=(const ShowPoints2DSettingsBuilder&&) = delete;
	
	ShowPoints2DSettingsBuilder& SetTransform(const ScaledTransform2Df& transform) { m_settings_ptr->transform = transform; return *this; }
private:
	ShowPoints2DSettings* m_settings_ptr;
};

// Image 2D

struct ShowImage2DSettings {
	ScaledTransform2Df transform;
	unsigned int image_display_width = 0;
	unsigned int image_display_height = 0;
};

class ShowImage2DSettingsBuilder {
public:
	explicit ShowImage2DSettingsBuilder(ShowImage2DSettings* settings_ptr)
	: m_settings_ptr{settings_ptr} {}
	
	ShowImage2DSettingsBuilder(const ShowImage2DSettingsBuilder&) = delete;
	ShowImage2DSettingsBuilder(ShowImage2DSettingsBuilder&&) = delete;
	ShowImage2DSettingsBuilder& operator=(const ShowImage2DSettingsBuilder&) = delete;
	ShowImage2DSettingsBuilder& operator=(const ShowImage2DSettingsBuilder&&) = delete;
	
	ShowImage2DSettingsBuilder& SetTransform(const ScaledTransform2Df& transform) { m_settings_ptr->transform = transform; return *this; }
	ShowImage2DSettingsBuilder& SetDisplayWidth(unsigned int width) { m_settings_ptr->image_display_width = width; return *this; }
	ShowImage2DSettingsBuilder& SetDisplayHeight(unsigned int height) { m_settings_ptr->image_display_height = height; return *this; }
private:
	ShowImage2DSettings* m_settings_ptr;
};

// Axis 3D

struct ShowAxis3DSettings {
	float size = 0.5f;
};

class ShowAxis3DSettingsBuilder {
public:
	explicit ShowAxis3DSettingsBuilder(ShowAxis3DSettings* settings_ptr)
	: m_settings_ptr{settings_ptr} {}
	
	ShowAxis3DSettingsBuilder(const ShowAxis3DSettingsBuilder&) = delete;
	ShowAxis3DSettingsBuilder(ShowAxis3DSettingsBuilder&&) = delete;
	ShowAxis3DSettingsBuilder& operator=(const ShowAxis3DSettingsBuilder&) = delete;
	ShowAxis3DSettingsBuilder& operator=(const ShowAxis3DSettingsBuilder&&) = delete;
	
	ShowAxis3DSettingsBuilder& SetSize(float size) { m_settings_ptr->size = size; return *this; }
private:
	ShowAxis3DSettings* m_settings_ptr;
};

// Points 3D

struct ShowPoints3DSettings {
	ScaledTransform3Df transform;
};

class ShowPoints3DSettingsBuilder {
public:
	explicit ShowPoints3DSettingsBuilder(ShowPoints3DSettings* settings_ptr)
	: m_settings_ptr{settings_ptr} {}
	
	ShowPoints3DSettingsBuilder(const ShowPoints3DSettingsBuilder&) = delete;
	ShowPoints3DSettingsBuilder(ShowPoints3DSettingsBuilder&&) = delete;
	ShowPoints3DSettingsBuilder& operator=(const ShowPoints3DSettingsBuilder&) = delete;
	ShowPoints3DSettingsBuilder& operator=(const ShowPoints3DSettingsBuilder&&) = delete;
	
	ShowPoints3DSettingsBuilder& SetTransform(const ScaledTransform3Df& transform) { m_settings_ptr->transform = transform; return *this; }
private:
	ShowPoints3DSettings* m_settings_ptr;
};

// Lines 3D

enum class LineType {
	LINE_SEGMENTS, LINE_STRIP, LINE_LOOP
};

struct ShowLines3DSettings {
	ScaledTransform3Df transform;
	LineType line_type = LineType::LINE_STRIP;
	float line_width = 1.0f;
};

class ShowLines3DSettingsBuilder {
public:
	explicit ShowLines3DSettingsBuilder(ShowLines3DSettings* settings_ptr)
	: m_settings_ptr{settings_ptr} {}
	
	ShowLines3DSettingsBuilder(const ShowLines3DSettingsBuilder&) = delete;
	ShowLines3DSettingsBuilder(ShowLines3DSettingsBuilder&&) = delete;
	ShowLines3DSettingsBuilder& operator=(const ShowLines3DSettingsBuilder&) = delete;
	ShowLines3DSettingsBuilder& operator=(const ShowLines3DSettingsBuilder&&) = delete;
	
	ShowLines3DSettingsBuilder& SetTransform(const ScaledTransform3Df& transform) { m_settings_ptr->transform = transform; return *this; }
	ShowLines3DSettingsBuilder& SetLineType(LineType line_type) { m_settings_ptr->line_type = line_type; return *this; }
	ShowLines3DSettingsBuilder& SetLineWidth(float line_width) { m_settings_ptr->line_width = line_width; return *this; }
private:
	ShowLines3DSettings* m_settings_ptr;
};

// Primitives 3D

struct ShowPrimitives3DSettings {
	ScaledTransform3Df transform;
	Vector3f light_vector = Vector3f(1.0f, 1.25f, 1.5f).normalized();
};

class ShowPrimitives3DSettingsBuilder {
public:
	explicit ShowPrimitives3DSettingsBuilder(ShowPrimitives3DSettings* settings_ptr)
	: m_settings_ptr{settings_ptr} {}
	
	ShowPrimitives3DSettingsBuilder(const ShowPrimitives3DSettingsBuilder&) = delete;
	ShowPrimitives3DSettingsBuilder(ShowPrimitives3DSettingsBuilder&&) = delete;
	ShowPrimitives3DSettingsBuilder& operator=(const ShowPrimitives3DSettingsBuilder&) = delete;
	ShowPrimitives3DSettingsBuilder& operator=(const ShowPrimitives3DSettingsBuilder&&) = delete;
	
	ShowPrimitives3DSettingsBuilder& SetTransform(const ScaledTransform3Df& transform) { m_settings_ptr->transform = transform; return *this; }
	ShowPrimitives3DSettingsBuilder& SetLightVector(const Vector3f& light_vector) { m_settings_ptr->light_vector = light_vector; return *this; }
private:
	ShowPrimitives3DSettings* m_settings_ptr;
};

// Button

struct ShowButtonSettings {

};

class ShowButtonsSettingsBuilder {
public:
	ShowButtonsSettingsBuilder(bool input_changed, ShowButtonSettings* settings_ptr)
	: m_input_changed{input_changed}, m_settings_ptr{settings_ptr} {}
	
	ShowButtonsSettingsBuilder(const ShowButtonsSettingsBuilder&) = delete;
	ShowButtonsSettingsBuilder(ShowButtonsSettingsBuilder&&) = delete;
	ShowButtonsSettingsBuilder& operator=(const ShowButtonsSettingsBuilder&) = delete;
	ShowButtonsSettingsBuilder& operator=(const ShowButtonsSettingsBuilder&&) = delete;
	
	explicit operator bool() const { return m_input_changed; }
	
private:
	bool m_input_changed;
	ShowButtonSettings* m_settings_ptr;
};

// Checkbox

struct ShowCheckboxSettings {

};

class ShowCheckboxSettingsBuilder {
public:
	ShowCheckboxSettingsBuilder(bool input_changed, ShowCheckboxSettings* settings_ptr)
	: m_input_changed{input_changed}, m_settings_ptr{settings_ptr} {}
	
	ShowCheckboxSettingsBuilder(const ShowCheckboxSettingsBuilder&) = delete;
	ShowCheckboxSettingsBuilder(ShowCheckboxSettingsBuilder&&) = delete;
	ShowCheckboxSettingsBuilder& operator=(const ShowCheckboxSettingsBuilder&) = delete;
	ShowCheckboxSettingsBuilder& operator=(const ShowCheckboxSettingsBuilder&&) = delete;
	
	explicit operator bool() const { return m_input_changed; }
	
private:
	bool m_input_changed;
	ShowCheckboxSettings* m_settings_ptr;
};

// Slider Float

struct ShowSliderFloatSettings {
	float min_value = -5.0f;
	float max_value = 5.0f;
};

class ShowSliderFloatSettingsBuilder {
public:
	ShowSliderFloatSettingsBuilder(bool input_changed, ShowSliderFloatSettings* settings_ptr)
	: m_input_changed{input_changed}, m_settings_ptr{settings_ptr} {}
	
	ShowSliderFloatSettingsBuilder(const ShowSliderFloatSettingsBuilder&) = delete;
	ShowSliderFloatSettingsBuilder(ShowSliderFloatSettingsBuilder&&) = delete;
	ShowSliderFloatSettingsBuilder& operator=(const ShowSliderFloatSettingsBuilder&) = delete;
	ShowSliderFloatSettingsBuilder& operator=(const ShowSliderFloatSettingsBuilder&&) = delete;
	
	explicit operator bool() const { return m_input_changed; }
	ShowSliderFloatSettingsBuilder& SetMinValue(float value) { m_settings_ptr->min_value = value; return *this; }
	ShowSliderFloatSettingsBuilder& SetMaxValue(float value) { m_settings_ptr->max_value = value; return *this; }
	
private:
	bool m_input_changed;
	ShowSliderFloatSettings* m_settings_ptr;
};

// Slider Int

struct ShowSliderIntSettings {
	int min_value = -5;
	int max_value = 5;
};

class ShowSliderIntSettingsBuilder {
public:
	ShowSliderIntSettingsBuilder(bool input_changed, ShowSliderIntSettings* settings_ptr)
	: m_input_changed{input_changed}, m_settings_ptr{settings_ptr} {}
	
	ShowSliderIntSettingsBuilder(const ShowSliderIntSettingsBuilder&) = delete;
	ShowSliderIntSettingsBuilder(ShowSliderIntSettingsBuilder&&) = delete;
	ShowSliderIntSettingsBuilder& operator=(const ShowSliderIntSettingsBuilder&) = delete;
	ShowSliderIntSettingsBuilder& operator=(const ShowSliderIntSettingsBuilder&&) = delete;
	
	explicit operator bool() const { return m_input_changed; }
	ShowSliderIntSettingsBuilder& SetMinValue(int value) { m_settings_ptr->min_value = value; return *this; }
	ShowSliderIntSettingsBuilder& SetMaxValue(int value) { m_settings_ptr->max_value = value; return *this; }
	
private:
	bool m_input_changed;
	ShowSliderIntSettings* m_settings_ptr;
};

// Color Picker

struct ShowColorPickerSettings {

};

class ShowColorPickerSettingsBuilder {
public:
	ShowColorPickerSettingsBuilder(bool input_changed, ShowColorPickerSettings* settings_ptr)
	: m_input_changed{input_changed}, m_settings_ptr{settings_ptr} {}
	
	ShowColorPickerSettingsBuilder(const ShowColorPickerSettingsBuilder&) = delete;
	ShowColorPickerSettingsBuilder(ShowColorPickerSettingsBuilder&&) = delete;
	ShowColorPickerSettingsBuilder& operator=(const ShowColorPickerSettingsBuilder&) = delete;
	ShowColorPickerSettingsBuilder& operator=(const ShowColorPickerSettingsBuilder&&) = delete;
	
	explicit operator bool() const { return m_input_changed; }
	
private:
	bool m_input_changed;
	ShowColorPickerSettings* m_settings_ptr;
};

} // namespace Toucan
