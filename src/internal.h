#pragma once

#include <mutex>
#include <vector>
#include <list>
#include <atomic>
#include <thread>
#include <condition_variable>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <renderdoc/renderdoc.h>

#include "Toucan/DataTypes.h"

#include "asset.h"

namespace Toucan {

enum class ElementType2D { LinePlot2D, Point2D, Image2D };

struct LinePlot2DMetadata {
	unsigned int vao;
	unsigned int vbo;
	
	int number_of_points;
	
	ShowLinePlot2DSettings settings;
};

struct Point2DMetadata{
	unsigned int vao;
	unsigned int vbo;
	
	int number_of_points;
	
	ShowPoints2DSettings settings;
};

struct Image2DMetadata {
	unsigned int texture;
	
	int width;
	int height;
	ImageFormat format;
	
	ShowImage2DSettings settings;
};

struct Element2D {
	std::string name;
	RigidTransform2Df pose;
	ElementType2D type = {};
	int draw_layer = 0;
	
	Rectangle data_bounds_cache;
	
	void* data_buffer_ptr = nullptr; // non-null if new data should be uploaded to device
	union {
		LinePlot2DMetadata line_plot_2d_metadata;
		Point2DMetadata point_2d_metadata;
		Image2DMetadata image_2d_metadata;
	};
};

struct Figure2D {
	std::string name;
	Figure2DSettings settings = {};
	
	std::mutex mutex;
	std::vector<Element2D> elements;
	
	Rectangle view;
	bool user_changed_view = false;
	bool user_dragging = false;
	std::vector<RigidTransform2Df> pose_stack;
	
	unsigned int framebuffer = 0;
	unsigned int framebuffer_color_texture = 0;
	Vector2i framebuffer_size = Vector2i(128, 128);
};

enum class ElementType3D { Grid3D, Axis3D, Point3D, Line3D, Primitive3D };

struct Grid3DMetadata {
	unsigned int vao_major;
	unsigned int vao_minor;
	unsigned int vbo_major;
	unsigned int vbo_minor;
	
	unsigned int number_of_minor_vertices;
	unsigned int number_of_major_vertices;
	
	float spacing;
	int lines;
};

struct Axis3DMetadata {
	ShowAxis3DSettings settings;
};

struct Point3DMetadata {
	unsigned int vao;
	unsigned int vbo;
	
	int number_of_points;
	
	ShowPoints3DSettings settings;
};

struct Line3DMetadata {
	unsigned int vao;
	unsigned int vbo;
	
	int number_of_line_vertices;
	
	ShowLines3DSettings settings;
};

struct Primitive3DMetadata {
	Primitive3D* vertex_data_ptr;
	int number_of_primitives;
	
	ShowPrimitives3DSettings settings;
};

struct Element3D {
	std::string name;
	RigidTransform3Df pose;
	ElementType3D type = {};
	
	void* data_buffer_ptr = nullptr; // non-null if new data should be uploaded to device
	union {
		Grid3DMetadata grid_3d_metadata;
		Axis3DMetadata axis_3d_metadata;
		Point3DMetadata point_3d_metadata;
		Line3DMetadata line_3d_metadata;
		Primitive3DMetadata primitive_3d_metadata;
	};
};

struct Figure3D {
	std::string name;
	Figure3DSettings settings = {};
	
	std::mutex mutex;
	std::vector<Element3D> elements;
	
	OrbitCamera camera;
	std::vector<RigidTransform3Df> pose_stack;
	
	bool dragging = false;
	
	unsigned int framebuffer = 0;
	unsigned int framebuffer_color_texture = 0;
	unsigned int framebuffer_depth_texture = 0;
	Vector2i framebuffer_size = Vector2i(128, 128);
};

enum class ElementInputType {
	BUTTON, CHECKBOX,
	SLIDER_FLOAT, SLIDER_FLOAT2, SLIDER_FLOAT3, SLIDER_FLOAT4,
	SLIDER_INT, SLIDER_INT2, SLIDER_INT3, SLIDER_INT4,
	COLOR_PICKER
};

struct ShowButtonMetadata {
	int number_of_click_events = 0;
	ShowButtonSettings settings;
};

struct ShowCheckboxMetadata {
	bool value;
	bool value_changed;
	ShowCheckboxSettings settings;
};

struct ShowSliderFloatMetadata {
	float value;
	bool value_changed;
	ShowSliderFloatSettings settings;
};

struct ShowSliderFloat2Metadata {
	Vector2f value;
	bool value_changed;
	ShowSliderFloatSettings settings;
};

struct ShowSliderFloat3Metadata {
	Vector3f value;
	bool value_changed;
	ShowSliderFloatSettings settings;
};

struct ShowSliderFloat4Metadata {
	Vector4f value;
	bool value_changed;
	ShowSliderFloatSettings settings;
};

struct ShowSliderIntMetadata {
	int value;
	bool value_changed;
	ShowSliderIntSettings settings;
};

struct ShowSliderInt2Metadata {
	Vector2i value;
	bool value_changed;
	ShowSliderIntSettings settings;
};

struct ShowSliderInt3Metadata {
	Vector3i value;
	bool value_changed;
	ShowSliderIntSettings settings;
};

struct ShowSliderInt4Metadata {
	Vector4i value;
	bool value_changed;
	ShowSliderIntSettings settings;
};

struct ShowColorPickerMetadata {
	Color value;
	bool value_changed;
	ShowColorPickerSettings settings;
};

struct ElementInput {
	std::string name;
	ElementInputType type;
	
	union {
		ShowButtonMetadata show_button_metadata;
		ShowCheckboxMetadata show_checkbox_metadata;
		ShowSliderFloatMetadata show_slider_float_metadata;
		ShowSliderFloat2Metadata show_slider_float2_metadata;
		ShowSliderFloat3Metadata show_slider_float3_metadata;
		ShowSliderFloat4Metadata show_slider_float4_metadata;
		ShowSliderIntMetadata show_slider_int_metadata;
		ShowSliderInt2Metadata show_slider_int2_metadata;
		ShowSliderInt3Metadata show_slider_int3_metadata;
		ShowSliderInt4Metadata show_slider_int4_metadata;
		ShowColorPickerMetadata show_color_picker_metadata;
	};
};

struct InputWindow {
	std::string name;
	InputSettings settings = {};
	
	std::mutex mutex;
	std::vector<ElementInput> elements;
};

struct ToucanContext {
	std::atomic_bool should_render = true;
	std::atomic_bool window_open = true;
	std::thread render_thread;
	
	std::mutex initialized_mutex;
	std::condition_variable initialized_cv;
	
	std::mutex window_close_mutex;
	std::condition_variable window_close_cv;
	
	GLFWwindow* window_ptr = nullptr;
	RENDERDOC_API_1_4_1* rdoc_api = nullptr;
	
	std::list<Toucan::Figure2D> figures_2d;
	Toucan::Figure2D* current_figure_2d = nullptr;
	
	std::list<Toucan::Figure3D> figures_3d;
	Toucan::Figure3D* current_figure_3d = nullptr;
	
	std::list<Toucan::InputWindow> input_windows;
	Toucan::InputWindow* current_input_window = nullptr;
	
	AssetContext asset_context = {};
};

} // namespace Toucan
