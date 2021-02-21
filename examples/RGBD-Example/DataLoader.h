#pragma once

#include <vector>
#include <utility>
#include <string>
#include <cstdlib>
#include <filesystem>

#include <Toucan/DataTypes.h>

struct Image {
	~Image() {
		std::free(m_data);
	}
	
	void* m_data = nullptr;
	int m_width = 0;
	int m_height = 0;
	int m_channels = 0;
	int m_pitch_x = 0;
	int m_pitch_y = 0;
};

class DataLoader {
public:
	explicit DataLoader(const std::filesystem::path& dataset_path);
	
	void next();
	[[nodiscard]] bool has_next() const;
	[[nodiscard]] Image get_rgb() const;
	[[nodiscard]] Image get_depth() const;
	[[nodiscard]] Toucan::RigidTransform3Df get_groundtruth() const;
	[[nodiscard]] uint64_t get_timestamp() const;
	
	[[nodiscard]] int get_size() const;
	[[nodiscard]] int get_current_index() const;

private:
	const std::filesystem::path m_dataset_path;
	
	int m_rgb_index;
	int m_depth_index;
	int m_groundtruth_index;
	
	std::vector<std::pair<uint64_t, const std::string>> m_rgb_files;
	std::vector<std::pair<uint64_t, const std::string>> m_depth_files;
	std::vector<std::pair<uint64_t, const Toucan::RigidTransform3Df>> m_ground_truths;
};
