#pragma once

#include <vector>
#include <utility>
#include <string>

std::pair<std::vector<float>, std::vector<std::string>> get_axis_ticks(float from_value, float to_value, int max_number_of_ticks);
