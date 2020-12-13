#include <chrono>
#include <memory>
#include <thread>

#include <Toucan/Toucan.h>

#include "DataLoader.h"

int main() {
	
	std::string path = "dataset/rgbd_dataset_freiburg3_long_office_household";
	
	DataLoader data_loader(path);
	
	Toucan::ToucanSettings toucan_settings;
	toucan_settings.resizeable = false;
	Toucan::Initialize(toucan_settings);
	
	std::vector<Toucan::LineVertex3D> pose_path;
	std::vector<float> pos_x_plot;
	std::vector<float> pos_y_plot;
	std::vector<float> pos_z_plot;
	
	
	while(data_loader.has_next() and Toucan::IsWindowOpen()) {
		
		auto image = data_loader.get_rgb();
		auto image_depth = data_loader.get_depth();
		
		Toucan::Figure2DSettings color_image_settings;
		color_image_settings.y_axis_direction = Toucan::YAxisDirection::DOWN;
		Toucan::BeginFigure2D("Color Image", color_image_settings);
		{
			Toucan::Image2D toucan_image;
			toucan_image.width = image.m_width;
			toucan_image.height = image.m_height;
			toucan_image.format = Toucan::ImageFormat::RGB_U8;
			toucan_image.image_buffer_ptr = image.m_data;
			Toucan::ShowImage2D("RGB Image", toucan_image, -1);
		}
		Toucan::EndFigure2D();
		
		Toucan::Figure2DSettings depth_image_settings;
		depth_image_settings.y_axis_direction = Toucan::YAxisDirection::DOWN;
		Toucan::BeginFigure2D("Depth Image", depth_image_settings);
		{
			Toucan::Image2D toucan_image;
			toucan_image.width = image.m_width;
			toucan_image.height = image.m_height;
			toucan_image.format = Toucan::ImageFormat::GRAY_U16;
			toucan_image.image_buffer_ptr = image_depth.m_data;
			
			Toucan::ShowImage2D("Depth Image", toucan_image, -1);
		}
		Toucan::EndFigure2D();
		
		Toucan::BeginFigure3D("Point Projection");
		
		constexpr int image_width = 640;
		constexpr int image_height = 480;
		
		constexpr float fx_inv = 1.0/525.0;
		constexpr float fy_inv = 1.0/525.0;
		constexpr float cx = 319.5;
		constexpr float cy = 239.5;
		
		constexpr float factor = 1.0f/5000.0f;
		
		std::vector<Toucan::Point3D> depth_points;
		depth_points.reserve(image_width*image_height);
		
		const uint8_t* image_ptr = reinterpret_cast<uint8_t*>(image.m_data);
		const uint16_t* depth_image_ptr = reinterpret_cast<uint16_t*>(image_depth.m_data);
		
		for (int image_v = 0; image_v < image_height; ++image_v) {
			for (int image_u = 0; image_u < image_width; ++image_u) {
				const uint16_t depth_value = depth_image_ptr[image_v*image_width + image_u];
				
				if (depth_value == 0) { continue; }
				
				const float r = image_ptr[image_v*image_width*3 + image_u*3 + 0] / 255.0f;
				const float g = image_ptr[image_v*image_width*3 + image_u*3 + 1] / 255.0f;
				const float b = image_ptr[image_v*image_width*3 + image_u*3 + 2] / 255.0f;
				
				const float z = depth_value * factor;
				const float x = (image_u - cx) * z * fx_inv;
				const float y = (image_v - cy) * z * fy_inv;
				
				depth_points.emplace_back(
					Toucan::Point3D{Toucan::Vector3f(x, y, z), Toucan::Color(r, g, b), 0.5f, Toucan::PointShape::Circle}
				);
			}
		}
		
		auto pose = data_loader.get_groundtruth();
		auto t = pose.translation();
		auto q = pose.unit_quaternion();
		
		pose_path.emplace_back(Toucan::LineVertex3D{Toucan::Vector3f (t.x(), t.y(), t.z()), Toucan::Color::Blue()});
		
		std::vector<Toucan::Primitive3D> primitive_array;
		primitive_array.emplace_back(Toucan::Primitive3D{
			Toucan::PrimitiveType::Cube,
			Toucan::ScaledTransform3Df(Toucan::Quaternionf(q.w(), q.x(), q.y(), q.z()), Toucan::Vector3f(t.x(), t.y(), t.z()), Toucan::Vector3f(0.2f, 0.2f, 0.01f)),
			Toucan::Color::Red()
		});
		
		Toucan::PushPose3D(Toucan::RigidTransform3Df(Toucan::Quaternionf(Toucan::Vector3f::UnitX(), M_PI_2), Toucan::Vector3f::Zero()));
		Toucan::ShowLines3D("Pose path", pose_path);
		Toucan::ShowPrimitives3D("Primitives", primitive_array);
		
		
		Toucan::PushPose3D(Toucan::RigidTransform3Df( Toucan::Quaternionf(q.w(), q.x(), q.y(), q.z()), Toucan::Vector3f(t.x(), t.y(), t.z())));
		
		Toucan::ShowPoints3D("Depth points", depth_points);
		
		Toucan::PopPose3D();
		Toucan::PopPose3D();
		
		Toucan::EndFigure3D();
		
		pos_x_plot.emplace_back(t.x());
		pos_y_plot.emplace_back(t.y());
		pos_z_plot.emplace_back(t.z());
		
		Toucan::BeginFigure2D("Position");
		
		Toucan::ShowLinePlot2DSettings pos_x_setting;
		pos_x_setting.line_color = Toucan::Color::Red();
		Toucan::ShowLinePlot2D("X Position", pos_x_plot, 0, pos_x_setting);
		Toucan::ShowLinePlot2DSettings pos_y_setting;
		pos_y_setting.line_color = Toucan::Color::Green();
		Toucan::ShowLinePlot2D("Y Position", pos_y_plot, 0, pos_y_setting);
		Toucan::ShowLinePlot2DSettings pos_z_setting;
		pos_z_setting.line_color = Toucan::Color::Blue();
		Toucan::ShowLinePlot2D("Z Position", pos_z_plot, 0, pos_z_setting);
		
		Toucan::EndFigure2D();
		
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20ms);
		
		data_loader.next();
		
	}
	
	Toucan::Destroy();
	
	return 0;
}
