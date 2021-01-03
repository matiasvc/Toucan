#include <chrono>
#include <memory>
#include <thread>

#include <Toucan/Toucan.h>

#include "DataLoader.h"

void project_image(std::vector<Toucan::Point3D>& point_vector, const Image& image, const Image& image_depth) {
	constexpr float fx_inv = 1.0 / 525.0;
	constexpr float fy_inv = 1.0 / 525.0;
	constexpr float cx = 319.5;
	constexpr float cy = 239.5;
	
	constexpr float factor = 1.0f / 5000.0f;
	
	const uint8_t* image_ptr = reinterpret_cast<uint8_t*>(image.m_data);
	const uint16_t* depth_image_ptr = reinterpret_cast<uint16_t*>(image_depth.m_data);
	
	const int image_width = image.m_width;
	const int image_height = image.m_height;
	
	for (int image_v = 0; image_v < image_height; ++image_v) {
		for (int image_u = 0; image_u < image_width; ++image_u) {
			const uint16_t depth_value = depth_image_ptr[image_v * image_width + image_u];
			
			if (depth_value == 0) { continue; }
			
			const float r = static_cast<float>(image_ptr[image_v * image_width * 3 + image_u * 3 + 0]) / 255.0f;
			const float g = static_cast<float>(image_ptr[image_v * image_width * 3 + image_u * 3 + 1]) / 255.0f;
			const float b = static_cast<float>(image_ptr[image_v * image_width * 3 + image_u * 3 + 2]) / 255.0f;
			
			const float z = static_cast<float>(depth_value) * factor;
			const float x = (static_cast<float>(image_u) - cx) * z * fx_inv;
			const float y = (static_cast<float>(image_v) - cy) * z * fy_inv;
			
			point_vector.emplace_back(
					Toucan::Point3D{Toucan::Vector3f(x, y, z), Toucan::Color(r, g, b), 0.5f, Toucan::PointShape::Circle}
			);
		}
	}
}

int main() {
	
	std::string path = "dataset/rgbd_dataset_freiburg3_long_office_household";
	
	DataLoader data_loader(path);
	
	Toucan::ToucanSettings toucan_settings;
	toucan_settings.resizeable = false;
	Toucan::Initialize(toucan_settings);
	
	std::vector<Toucan::Point3D> depth_points;
	depth_points.reserve(640 * 480);
	
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
		
		project_image(depth_points, image, image_depth);
		
		auto gt_pose = data_loader.get_groundtruth();
		auto gt_t = gt_pose.translation();
		auto gt_q = gt_pose.unit_quaternion();
		
		pose_path.emplace_back(Toucan::LineVertex3D{Toucan::Vector3f(gt_t.x(), gt_t.y(), gt_t.z()), Toucan::Color::Blue()});
		Toucan::BeginFigure3D("Point Projection");
		{
			Toucan::PushPose3D(Toucan::RigidTransform3Df(Toucan::Quaternionf(Toucan::Vector3f::UnitX(), M_PI_2), Toucan::Vector3f::Zero()));
			{ // Static transform to align with our coordinate system
				Toucan::ShowLines3D("Pose path", pose_path);
				
				Toucan::PushPose3D(Toucan::RigidTransform3Df(Toucan::Quaternionf(gt_q.w(), gt_q.x(), gt_q.y(), gt_q.z()), Toucan::Vector3f(gt_t.x(), gt_t.y(), gt_t.z())));
				{ // The coordinate system of the camera
					Toucan::ShowAxis3D("Axis");
					Toucan::ShowPoints3D("Depth points", depth_points);
				}
				Toucan::PopPose3D();
			}
			Toucan::PopPose3D();
		}
		Toucan::EndFigure3D();
		
		depth_points.clear();
		
		pos_x_plot.emplace_back(gt_t.x());
		pos_y_plot.emplace_back(gt_t.y());
		pos_z_plot.emplace_back(gt_t.z());
		
		Toucan::BeginFigure2D("Position");
		{
			Toucan::ShowLinePlot2DSettings pos_x_setting;
			pos_x_setting.line_color = Toucan::Color::Red();
			Toucan::ShowLinePlot2D("X Position", pos_x_plot, 0, pos_x_setting);
			
			Toucan::ShowLinePlot2DSettings pos_y_setting;
			pos_y_setting.line_color = Toucan::Color::Green();
			Toucan::ShowLinePlot2D("Y Position", pos_y_plot, 0, pos_y_setting);
			
			Toucan::ShowLinePlot2DSettings pos_z_setting;
			pos_z_setting.line_color = Toucan::Color::Blue();
			Toucan::ShowLinePlot2D("Z Position", pos_z_plot, 0, pos_z_setting);
		}
		
		Toucan::EndFigure2D();
		
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20ms);
		
		data_loader.next();
	}
	
	Toucan::SleepUntilWindowClosed();
	
	Toucan::Destroy();
	
	return 0;
}
