#pragma once

#include "internal.h"

extern Toucan::ToucanContext* toucan_context_ptr;

#define validate_initialized(function_name) \
if (toucan_context_ptr == nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called before Toucan was initialized. Did you forget to call 'Toucan::Initialize'?"); }

#define validate_active_figure2d(function_name) \
if (toucan_context_ptr->current_figure_2d == nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called without an active Figure2D. Did you forget to call 'Toucan::BeginFigure2D'?"); }

#define validate_inactive_figure2d(function_name) \
if (toucan_context_ptr->current_figure_2d != nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called while another Figure2D was active. Did you forget to call 'Toucan::EndFigure2D'?"); }

#define validate_active_figure3d(function_name) \
if (toucan_context_ptr->current_figure_3d == nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called without an active Figure2D. Did you forget to call 'Toucan::BeginFigure2D'?"); }

#define validate_inactive_figure3d(function_name) \
if (toucan_context_ptr->current_figure_3d != nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called while another Figure2D was active. Did you forget to call 'Toucan::EndFigure2D'?"); }

#define validate_active_input_window(function_name) \
if (toucan_context_ptr->current_input_window == nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called without an active Figure2D. Did you forget to call 'Toucan::BeginInputWindow'?"); }

#define validate_inactive_input_window(function_name) \
if (toucan_context_ptr->current_input_window != nullptr) { throw std::runtime_error("Toucan error! 'Toucan::"#function_name"' was called while another InputWindow was active. Did you forget to call 'Toucan::EndInputWindow'?"); }
