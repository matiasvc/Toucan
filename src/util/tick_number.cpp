#include "tick_number.h"

#include <cassert>
#include <cmath>
#include <string>

struct TickInfo {
	int start_index;
	int end_index;
	int multiplier;
	int exponent;
};

TickInfo compute_tick_info(float from_value, float to_value, int max_number_of_ticks) {
	assert(from_value < to_value);
	
	float width = to_value - from_value;
	float min_segment_width = width / static_cast<float>(max_number_of_ticks);
	
	float min_segment_width_log10 = std::log10(min_segment_width);
	int exponent = static_cast<int>(std::floor(min_segment_width_log10));
	
	float magnitude = std::pow(10.0f, exponent);
	
	int multiplier;
	if (width / (magnitude * 1.0f) < static_cast<float>(max_number_of_ticks)) {
		multiplier = 1;
	} else if (width / (magnitude * 2.0f) < static_cast<float>(max_number_of_ticks)) {
		multiplier = 2;
	} else {
		multiplier = 5;
	}
	
	int start_index = static_cast<int>(std::ceil(from_value / (static_cast<float>(multiplier) * magnitude)));
	int end_index = static_cast<int>(std::floor(to_value / (static_cast<float>(multiplier) * magnitude)));
	
	return {start_index, end_index, multiplier, exponent};
}
constexpr int switch_to_scientific_magnitude = 5;

std::string tick_to_string(int index, int multiplier, int exponent) {
	
	int base_value = std::abs(index * multiplier);
	
	std::string value_string;
	value_string.reserve(32); // TODO(Matias): reserve size based on number of digits
	value_string = std::to_string(base_value);
	
	if (std::abs(exponent) >= switch_to_scientific_magnitude) { // Scientific notation
		const auto num_of_digits = value_string.size();
		
		if (num_of_digits > 1) {
			value_string.insert(value_string.cbegin() + 1, '.');
		}
		else {
			std::string append_str = ".0";
			value_string.insert(value_string.cend(), append_str.cbegin(), append_str.cend());
		}
		
		if (exponent > 0) {
			std::string exponent_string = std::to_string(static_cast<size_t>(exponent) + num_of_digits - 1);
			value_string.insert(value_string.cend(), 'e');
			value_string.insert(value_string.cend(), '+');
			value_string.insert(value_string.cend(), exponent_string.cbegin(), exponent_string.cend());
		}
		else {
			std::string exponent_string = std::to_string(static_cast<size_t>(exponent) + num_of_digits - 1);
			value_string.insert(value_string.cend(), 'e');
			value_string.insert(value_string.cend(), exponent_string.cbegin(), exponent_string.cend());
		}
		
	}
	else { // Normal notation
		if (exponent > 0) {
			if (index != 0) {
				std::string following_zeros(static_cast<size_t>(exponent), '0');
				value_string.insert(value_string.cend(), following_zeros.cbegin(), following_zeros.cend());
			}
		}
		else if (exponent < 0) {
			// TODO: Could be faster by precomputing number of zeros needed and inserting all at once
			// using the "iterator insert( const_iterator pos, size_type count, CharT ch );" overload
			while (value_string.cend() + exponent <= value_string.cbegin()) {
				value_string.insert(value_string.cbegin(), '0');
			}
			
			value_string.insert(value_string.cend() + exponent, '.');
		}
	}
	
	if (index < 0) {
		value_string.insert(value_string.cbegin(), '-');
	}
	
	return value_string;
}


std::pair<std::vector<float>, std::vector<std::string>> get_axis_ticks(float from_value, float to_value, int max_number_of_ticks) {
	
	const auto tick_info = compute_tick_info(from_value, to_value, max_number_of_ticks);
	
	const auto number_of_ticks = static_cast<size_t>(tick_info.end_index - tick_info.start_index);
	
	std::vector<float> values;
	values.reserve(number_of_ticks);
	std::vector<std::string> value_strings;
	value_strings.reserve(number_of_ticks);
	
	for (int index = tick_info.start_index; index <= tick_info.end_index; ++index) {
		float value = index * tick_info.multiplier * std::pow(10.0, tick_info.exponent);
		std::string value_string = tick_to_string(index, tick_info.multiplier, tick_info.exponent);
		
		values.emplace_back(value);
		value_strings.emplace_back(std::move(value_string));
	}
	
	return {values, value_strings};
}
