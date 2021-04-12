#include <Toucan/Toucan.h>

#include "internal.h"
#include "validate.h"

namespace Toucan {

ElementInput& get_or_create_element_input(FigureInput& input_window, const std::string& name, ElementInputType type) {
	// Does the Element3D object with that name already exist?
	ElementInput* current_element_ptr = nullptr;
	for (auto& element : input_window.elements) {
		if (element.name == name) {
			current_element_ptr = &element;
			break;
		}
	}
	
	// If it does not exist, we must create a new one.
	if (current_element_ptr == nullptr) {
		
		ElementInput new_element_3d = {};
		new_element_3d.name = name;
		new_element_3d.type = type;
		
		auto& inserted_element = input_window.elements.emplace_back(new_element_3d);
		current_element_ptr = &inserted_element;
	}
	
	return *current_element_ptr;
}

InputSettingsBuilder BeginInputWindow(const std::string& name) {
	validate_initialized(BeginInputWindow)
	auto& toucan_context = *toucan_context_ptr;
	validate_inactive_input_window(BeginFigure3D)
	
	FigureInput* input_window_ptr = nullptr;
	for (auto& input_window : toucan_context.input_windows) { // Does the InputWindow already exists
		if (input_window.name == name) {
			input_window_ptr = &input_window;
			break;
		}
	}
	
	if (input_window_ptr == nullptr) { // Do we need to create a new InputWindow
		auto& input_window = toucan_context.input_windows.emplace_back();
		input_window.name = name;
		input_window_ptr = &input_window;
	}
	
	input_window_ptr->mutex.lock();
	
	toucan_context.current_input_window = input_window_ptr;
	return InputSettingsBuilder(&input_window_ptr->settings);
}

void EndInputWindow() {
	validate_initialized(BeginInputWindow)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(BeginInputWindow)
	
	toucan_context.current_input_window->mutex.unlock();
	toucan_context.current_input_window = nullptr;
}

ShowButtonsSettingsBuilder ShowButton(const std::string& name) {
	validate_initialized(ShowButton)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowButton)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::BUTTON);
	
	if (current_element.show_button_metadata.number_of_click_events > 0) {
		current_element.show_button_metadata.number_of_click_events--;
		return ShowButtonsSettingsBuilder(true, &current_element.show_button_metadata.settings);
	} else {
		return ShowButtonsSettingsBuilder(false, &current_element.show_button_metadata.settings);
	}
}

ShowCheckboxSettingsBuilder ShowCheckbox(const std::string& name, bool& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::CHECKBOX);
	
	if (current_element.show_checkbox_metadata.value_changed) {
		value = current_element.show_checkbox_metadata.value;
		current_element.show_checkbox_metadata.value_changed = false;
		return ShowCheckboxSettingsBuilder(true, &current_element.show_checkbox_metadata.settings);
	} else {
		current_element.show_checkbox_metadata.value = value;
		return ShowCheckboxSettingsBuilder(false, &current_element.show_checkbox_metadata.settings);
	}
}

ShowSliderFloatSettingsBuilder ShowSliderFloat(const std::string& name, float& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::SLIDER_FLOAT);
	
	if (current_element.show_slider_float_metadata.value_changed) {
		value = current_element.show_slider_float_metadata.value;
		current_element.show_slider_float_metadata.value_changed = false;
		return ShowSliderFloatSettingsBuilder(true, &current_element.show_slider_float_metadata.settings);
	} else {
		current_element.show_slider_float_metadata.value = value;
		return ShowSliderFloatSettingsBuilder(false, &current_element.show_slider_float_metadata.settings);
	}
}

ShowSliderFloatSettingsBuilder ShowSliderFloat2(const std::string& name, Vector2f& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::SLIDER_FLOAT2);
	
	if (current_element.show_slider_float2_metadata.value_changed) {
		value = current_element.show_slider_float2_metadata.value;
		current_element.show_slider_float2_metadata.value_changed = false;
		return ShowSliderFloatSettingsBuilder(true, &current_element.show_slider_float2_metadata.settings);
	} else {
		current_element.show_slider_float2_metadata.value = value;
		return ShowSliderFloatSettingsBuilder(false, &current_element.show_slider_float2_metadata.settings);
	}
}

ShowSliderFloatSettingsBuilder ShowSliderFloat3(const std::string& name, Vector3f& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::SLIDER_FLOAT3);
	
	if (current_element.show_slider_float3_metadata.value_changed) {
		value = current_element.show_slider_float3_metadata.value;
		current_element.show_slider_float3_metadata.value_changed = false;
		return ShowSliderFloatSettingsBuilder(true, &current_element.show_slider_float3_metadata.settings);
	} else {
		current_element.show_slider_float3_metadata.value = value;
		return ShowSliderFloatSettingsBuilder(false, &current_element.show_slider_float3_metadata.settings);
	}
}

ShowSliderFloatSettingsBuilder ShowSliderFloat4(const std::string& name, Vector4f& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::SLIDER_FLOAT4);
	
	if (current_element.show_slider_float4_metadata.value_changed) {
		value = current_element.show_slider_float4_metadata.value;
		current_element.show_slider_float4_metadata.value_changed = false;
		return ShowSliderFloatSettingsBuilder(true, &current_element.show_slider_float4_metadata.settings);
	} else {
		current_element.show_slider_float4_metadata.value = value;
		return ShowSliderFloatSettingsBuilder(false, &current_element.show_slider_float4_metadata.settings);
	}
}

ShowSliderIntSettingsBuilder ShowSliderInt(const std::string& name, int& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::SLIDER_INT);
	
	if (current_element.show_slider_int_metadata.value_changed) {
		value = current_element.show_slider_int_metadata.value;
		current_element.show_slider_int_metadata.value_changed = false;
		return ShowSliderIntSettingsBuilder(true, &current_element.show_slider_int_metadata.settings);
	} else {
		current_element.show_slider_int_metadata.value = value;
		return ShowSliderIntSettingsBuilder(false, &current_element.show_slider_int_metadata.settings);
	}
}

ShowSliderIntSettingsBuilder ShowSliderInt2(const std::string& name, Vector2i& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::SLIDER_INT2);
	
	if (current_element.show_slider_int2_metadata.value_changed) {
		value = current_element.show_slider_int2_metadata.value;
		current_element.show_slider_int2_metadata.value_changed = false;
		return ShowSliderIntSettingsBuilder(true, &current_element.show_slider_int2_metadata.settings);
	} else {
		current_element.show_slider_int2_metadata.value = value;
		return ShowSliderIntSettingsBuilder(false, &current_element.show_slider_int2_metadata.settings);
	}
}

ShowSliderIntSettingsBuilder ShowSliderInt3(const std::string& name, Vector3i& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::SLIDER_INT3);
	
	if (current_element.show_slider_int3_metadata.value_changed) {
		value = current_element.show_slider_int3_metadata.value;
		current_element.show_slider_int3_metadata.value_changed = false;
		return ShowSliderIntSettingsBuilder(true, &current_element.show_slider_int3_metadata.settings);
	} else {
		current_element.show_slider_int3_metadata.value = value;
		return ShowSliderIntSettingsBuilder(false, &current_element.show_slider_int3_metadata.settings);
	}
}

ShowSliderIntSettingsBuilder ShowSliderInt4(const std::string& name, Vector4i& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::SLIDER_INT4);
	
	if (current_element.show_slider_int4_metadata.value_changed) {
		value = current_element.show_slider_int4_metadata.value;
		current_element.show_slider_int4_metadata.value_changed = false;
		return ShowSliderIntSettingsBuilder(true, &current_element.show_slider_int4_metadata.settings);
	} else {
		current_element.show_slider_int4_metadata.value = value;
		return ShowSliderIntSettingsBuilder(false, &current_element.show_slider_int4_metadata.settings);
	}
}

ShowColorPickerSettingsBuilder ShowColorPicker(const std::string& name, Color& value) {
	validate_initialized(ShowSliderFloat)
	auto& toucan_context = *toucan_context_ptr;
	validate_active_input_window(ShowSliderFloat)
	auto& current_input_window = *toucan_context.current_input_window;
	
	auto& current_element = get_or_create_element_input(current_input_window, name, ElementInputType::COLOR_PICKER);
	
	if (current_element.show_color_picker_metadata.value_changed) {
		value = current_element.show_color_picker_metadata.value;
		current_element.show_color_picker_metadata.value_changed = false;
		return ShowColorPickerSettingsBuilder(true, &current_element.show_color_picker_metadata.settings);
	} else {
		current_element.show_color_picker_metadata.value = value;
		return ShowColorPickerSettingsBuilder(false, &current_element.show_color_picker_metadata.settings);
	}
}

} // namespace Toucan
