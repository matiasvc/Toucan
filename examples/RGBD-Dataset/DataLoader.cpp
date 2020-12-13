#include "DataLoader.h"
#include <iostream>
#include <cstdint>
#include <fstream>

#include <stb_image.h>

#include <unistd.h>

DataLoader::DataLoader(const std::experimental::filesystem::path& dataset_path, const Sophus::SE3f& transformation)
		: m_dataset_path{dataset_path}, m_transformation{transformation}, m_rgb_index{0}, m_depth_index{0}, m_groundtruth_index{0} {
	
	const std::experimental::filesystem::path rgb_file_path = dataset_path / "rgb.txt";
	
	std::ifstream rgb_file(rgb_file_path.string());
	if (!rgb_file.is_open()) { throw std::invalid_argument("Unable to open rgb.txt file"); }
	
	std::string line;
	const std::string decimal = ".";
	const std::string delimiter = " ";
	
	while (getline(rgb_file, line))
	{
		if (line[0] == '#') { continue; }
		const size_t decimal_pos = line.find(decimal);
		const size_t delimiter_pos = line.find(delimiter);
		
		const uint64_t timestamp = std::stoull(line.substr(0, decimal_pos)) * 1000000 +
		                           std::stoull(line.substr(decimal_pos+1, delimiter_pos));
		const std::string file_name = line.substr(delimiter_pos + 1, line.length());
		
		m_rgb_files.emplace_back(timestamp, file_name);
	}
	
	const std::experimental::filesystem::path depth_file_path = dataset_path / "depth.txt";
	
	std::ifstream depth_file(depth_file_path.string());
	if (!depth_file.is_open()) { throw std::invalid_argument("Unable to open depth.txt file"); }
	
	while (getline(depth_file, line))
	{
		if (line[0] == '#') { continue; }
		const size_t decimal_pos = line.find(decimal);
		const size_t delimiter_pos = line.find(delimiter);
		
		const uint64_t timestamp = std::stoull(line.substr(0, decimal_pos)) * 1000000 +
		                           std::stoull(line.substr(decimal_pos+1, delimiter_pos));
		const std::string file_name = line.substr(delimiter_pos + 1, line.length());
		
		m_depth_files.emplace_back(timestamp, file_name);
	}
	
	const std::experimental::filesystem::path groundtruth_file_path = dataset_path / "groundtruth.txt";
	
	std::ifstream groudtruth_file(groundtruth_file_path.string());
	if (!groudtruth_file.is_open()) { throw std::invalid_argument("Unable to open groundtruth.txt file"); }
	
	while (getline(groudtruth_file, line))
	{
		if (line[0] == '#') { continue; }
		
		std::stringstream ss(line);
		std::string item;
		
		std::getline(ss, item, ' ');
		
		const size_t decimal_pos = item.find(decimal);
		const uint64_t timestamp = std::stoull(item.substr(0, decimal_pos)) * 1000000 +
		                           std::stoull(item.substr(decimal_pos+1)) * 100;
		
		std::getline(ss, item, ' ');
		float tx = std::stof(item);
		
		std::getline(ss, item, ' ');
		float ty = std::stof(item);
		
		std::getline(ss, item, ' ');
		float tz = std::stof(item);
		
		std::getline(ss, item, ' ');
		float qx = std::stof(item);
		
		std::getline(ss, item, ' ');
		float qy = std::stof(item);
		
		std::getline(ss, item, ' ');
		float qz = std::stof(item);
		
		std::getline(ss, item, ' ');
		float qw = std::stof(item);
		
		const Eigen::Quaternionf orientation(qw, qx, qy, qz);
		const Eigen::Vector3f position(tx, ty, tz);
		const Sophus::SE3f transform(orientation, position);
		
		m_ground_truths.emplace_back(timestamp, transform);
	}
	
	this->next();
}

void DataLoader::next() {
	m_rgb_index++;
	
	const int64_t current_timestamp = m_rgb_files[m_rgb_index].first;
	
	while (m_depth_files[m_depth_index + 1].first < current_timestamp and m_depth_index < m_depth_files.size() - 1) {
		m_depth_index++;
	}
	
	while (m_ground_truths[m_groundtruth_index + 1].first < current_timestamp and m_groundtruth_index < m_ground_truths.size() - 1) {
		m_groundtruth_index++;
	}
	
}

bool DataLoader::has_next() const {
	return m_rgb_index < m_rgb_files.size();
}

int DataLoader::get_size() const {
	return (int)m_rgb_files.size();
}

int DataLoader::get_current_index() const {
	return m_rgb_index;
}

dl_image DataLoader::get_depth() const {
	const std::experimental::filesystem::path& depth_image_path = m_dataset_path / m_depth_files[m_depth_index].second;
	
	dl_image image;
	
	const int depthChannels = 1;
	
	image.m_data = stbi_load_16(depth_image_path.c_str(), &image.m_width, &image.m_height, &image.m_channels, depthChannels);
	image.m_pitch_x = depthChannels * sizeof(uint16_t);
	image.m_pitch_y = image.m_pitch_x * image.m_width;
	
	return image;
}

dl_image DataLoader::get_rgb() const {
	const std::experimental::filesystem::path& rgb_image_path = m_dataset_path / m_rgb_files[m_rgb_index].second;
	
	dl_image image;
	
	const int rgbChannels = 3;
	
	image.m_data = stbi_load(rgb_image_path.c_str(), &image.m_width, &image.m_height, &image.m_channels, rgbChannels);
	image.m_pitch_x = rgbChannels * sizeof(uint8_t);
	image.m_pitch_y = image.m_pitch_x * image.m_width;
	
	return image;
}

Sophus::SE3f DataLoader::get_groundtruth() const {
	return m_transformation * m_ground_truths[m_groundtruth_index].second;
}

uint64_t DataLoader::get_timestamp() const
{
	return m_rgb_files[m_rgb_index].first;
}
