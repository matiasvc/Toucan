#pragma once

#include <array>
#include <optional>
#include <ostream>
#include <iomanip>
#include <cmath>
#include <iostream>

#include "LinAlg.h"

namespace Toucan {

template <typename T>
struct Buffer {
	constexpr Buffer() :
	data_ptr{nullptr}, number_of_elements{0} { }
	
	constexpr Buffer(const T* data_ptr, size_t number_of_elements) :
	data_ptr{data_ptr}, number_of_elements{number_of_elements} { }
	
	const T* data_ptr;
	size_t number_of_elements;
	//size_t pitch; // TODO: Support pitch between elements
};

struct Color {
	constexpr Color() :
	r{1.0f}, g{1.0f}, b{1.0f} { }
	
	constexpr Color(float r, float g, float b) :
	r{r}, g{g}, b{b} { }
	
	constexpr static Color Red()     { return Color(1.0f, 0.0f, 0.0); }
	constexpr static Color Green()   { return Color(0.0f, 1.0f, 0.0); }
	constexpr static Color Blue()    { return Color(0.0f, 0.0f, 1.0); }
	constexpr static Color Black()   { return Color(0.0f, 0.0f, 0.0); }
	constexpr static Color White()   { return Color(1.0f, 1.0f, 1.0); }
	constexpr static Color Gray()    { return Color(0.5f, 0.5f, 0.5); }
	constexpr static Color Yellow()  { return Color(1.0f, 1.0f, 0.0); }
	constexpr static Color Cyan()    { return Color(0.0f, 1.0f, 1.0); }
	constexpr static Color Magenta() { return Color(1.0f, 0.0f, 1.0); }
	
	float r;
	float g;
	float b;
};

struct Rectangle {
	Rectangle() :
	min{Vector2f::Zero()}, max{Vector2f::Ones()} { }
	
	Rectangle(const Vector2f& min, const Vector2f& max) :
	min{min}, max{max} { }
	
	[[nodiscard]] float width() const { return max.x() - min.x(); }
	[[nodiscard]] float height() const { return max.y() - min.y(); }
	[[nodiscard]] Vector2f size() const { return Vector2f(width(), height()); }
	
	Vector2f min;
	Vector2f max;
};

inline Rectangle get_resized_rectangle(const Rectangle& rect, Vector2f change) {
	return Rectangle{
		Vector2f(rect.min.x() - change.x(), rect.min.y() - change.y()),
		Vector2f(rect.max.x() + change.x(), rect.max.y() + change.y()),
	};
}

inline Rectangle get_union_of_rectangles(const Rectangle& rect1, const Rectangle& rect2) {
	return Rectangle{
			Vector2f(std::min(rect1.min.x(), rect2.min.x()), std::min(rect1.min.y(), rect2.min.y())),
			Vector2f(std::max(rect1.max.x(), rect2.max.x()), std::max(rect1.max.y(), rect2.max.y()))
	};
}

inline std::ostream& operator<<(std::ostream& out, const Rectangle& rect) {
	out << "Rectangle:\n";
	out << "\t Min: (" << rect.min.x() << ", " << rect.min.y() << ")\n";
	out << "\t Max: (" << rect.max.x() << ", " << rect.max.y() << ")\n";
	
	return out;
}

// Drawable

enum class ImageFormat : uint8_t {
	GRAY_U8 = 0,
	GRAY_U16, GRAY_S16,
	RG_U8, RG_U16, RG_U32, RG_F32,
	RGB_U8, RGB_U16, RGB_U32, RGB_F32,
	BGR_U8, BGR_U16, BGR_U32, BGR_F32,
	};

inline constexpr size_t get_bytes_per_pixel(ImageFormat format) {
	switch (format) {
		case ImageFormat::GRAY_U8: {
			return 1*sizeof(uint8_t);
		}
		case ImageFormat::GRAY_U16:
		case ImageFormat::GRAY_S16: {
			return 1*sizeof(uint16_t);
		}
		case ImageFormat::RG_U8: {
			return 2*sizeof(uint8_t);
		}
		case ImageFormat::RG_U16: {
			return 2*sizeof(uint16_t);
		}
		case ImageFormat::RG_U32: {
			return 2*sizeof(uint32_t);
		}
		case ImageFormat::RG_F32: {
			return 2*sizeof(float);
		}
		case ImageFormat::RGB_U8:
		case ImageFormat::BGR_U8: {
			return 3*sizeof(uint8_t);
		}
		case ImageFormat::RGB_U16:
		case ImageFormat::BGR_U16: {
			return 3*sizeof(uint16_t);
		}
		case ImageFormat::RGB_U32:
		case ImageFormat::BGR_U32: {
			return 3*sizeof(uint32_t);
		}
		case ImageFormat::RGB_F32:
		case ImageFormat::BGR_F32: {
			return 3*sizeof(float);
		}
		default: {
			throw std::runtime_error("ERROR! Not implemented");
		}
	}
}

struct Image2D {
	constexpr Image2D() :
	image_buffer_ptr{nullptr}, width{0}, height{0}, format{ImageFormat::GRAY_U8} { }
	
	constexpr Image2D(void* image_buffer_ptr, int width, int height, ImageFormat format) :
	image_buffer_ptr{image_buffer_ptr}, width{width}, height{height}, format{format} { }
	
	void* image_buffer_ptr;
	int width;
	int height;
	ImageFormat format;
};

enum class PointShape : uint8_t {Square = 0, Circle = 1, Diamond = 2, Cross = 3, Ring = 4};


struct Point2D {
	constexpr Point2D() :
	position{Vector2f::Zero()}, color{Color::White()}, size{8.0f}, shape{PointShape::Circle} { }
	
	constexpr Point2D(const Vector2f& position, const Color& color, float size, PointShape shape) :
	position{position}, color{color}, size{size}, shape{shape} { }
	
	Vector2f position;
	Color color;
	float size;
	PointShape shape;
};


struct Point3D {
	constexpr Point3D() :
	position{Vector3f::Zero()}, color{Color::White()}, size{8.0f}, shape{PointShape::Circle} { }
	
	constexpr Point3D(const Vector3f& position, const Color& color, float size, PointShape shape) :
	position{position}, color{color}, size{size}, shape{shape} { }
	
	Vector3f position;
	Color color;
	float size;
	PointShape shape;
};

struct LineVertex3D {
	LineVertex3D() :
	position{Vector3f::Zero()}, color{Color::White()} { };
	
	LineVertex3D(const Vector3f& position, const Color& color) :
	position{position}, color{color} { }
	
	Vector3f position;
	Color color;
};

enum class PrimitiveType {Sphere = 0, Cube = 1, Cylinder = 2};

struct Primitive3D {
	Primitive3D() :
	type{PrimitiveType::Sphere}, scaled_transform{}, color{Color::Red()} { }
	
	Primitive3D(PrimitiveType type, const ScaledTransform3Df& scaled_transform, const Color& color) :
	type{type}, scaled_transform{scaled_transform}, color{color} { }
	
	
	PrimitiveType type = PrimitiveType::Sphere;
	ScaledTransform3Df scaled_transform;
	Color color = Color::Red();
};

struct OrbitCamera {
	OrbitCamera() :
	pitch{4*M_PI/3}, yaw{M_PI/6}, distance{3.5}, orbit_center{Vector3f::Zero()} { }
	
	void orbit(const Vector2f& delta) { yaw += delta.x(); pitch += delta.y(); }
	void move(const Vector2f& delta) {
		orbit_center.x() += (-std::cos(yaw) * delta.x() - std::sin(yaw) * delta.y()) * distance;
		orbit_center.y() += (-std::sin(yaw) * delta.x() + std::cos(yaw) * delta.y()) * distance;
	}
	void change_distance(float delta) {
		distance += static_cast<float>(std::log1p(distance)*delta/std::log(5)); // TODO(Matias): Make distance curve (log base) user editable
		distance = std::max(distance, 0.0f);
	}
	
	[[nodiscard]] RigidTransform3Df get_pose() const {
		const Quaternionf orientation =
				Quaternionf(Vector3f::UnitZ(), yaw) *
				Quaternionf(Vector3f::UnitX(), pitch);
		const Vector3f translation = orientation * Vector3f(0.0f, 0.0f, -distance) + orbit_center;
		return RigidTransform3Df(orientation, translation);
	}
	
	[[nodiscard]] RigidTransform3Df get_orbit_pose(float distance) const {
		const Quaternionf orientation =
				Quaternionf(Vector3f::UnitZ(), yaw) *
				Quaternionf(Vector3f::UnitX(), pitch);
		const Vector3f translation = orientation * Vector3f(0.0f, 0.0f, -distance);
		return RigidTransform3Df(orientation, translation);
	}
	
	float pitch;
	float yaw;
	float distance;
	Vector3f orbit_center;
};

} // namespace Toucan
