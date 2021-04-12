#include <Toucan/Toucan.h>

#include <thread>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "internal.h"
#include "render.h"
#include "Utils.h"
#include "gl/error.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <renderdoc/renderdoc.h>
#include <dlfcn.h>

#include "validate.h"

Toucan::ToucanContext* toucan_context_ptr = nullptr;

namespace Toucan {

// Forward declarations
void render_loop(ToucanSettings);
static void glfw_error_callback(int error, const char* description);
static void glfw_window_close_callback(GLFWwindow* window);


void Initialize(ToucanSettings settings) {
	if (toucan_context_ptr != nullptr) { throw std::runtime_error("Toucan error! 'Initialize' was called when Toucan already was initialized. Did you call 'Initialize' multiple times?"); }
	toucan_context_ptr = new ToucanContext;
	toucan_context_ptr->render_thread = std::thread(render_loop, settings);
	
	std::unique_lock lock(toucan_context_ptr->initialized_mutex);
	toucan_context_ptr->initialized_cv.wait(lock);
}

void Destroy() {
	validate_initialized(Destroy)
	toucan_context_ptr->should_render = false;
	toucan_context_ptr->render_thread.join();
	delete toucan_context_ptr;
}

bool IsWindowOpen() {
	validate_initialized(IsWindowOpen)
	return toucan_context_ptr->window_open;
}

void SleepUntilWindowClosed() {
	validate_initialized(SleepUntilWindowClosed)
	
	if (not toucan_context_ptr->window_open) {
		return;
	}
	
	std::unique_lock lock(toucan_context_ptr->window_close_mutex);
	toucan_context_ptr->window_close_cv.wait(lock);
}

void render_loop(ToucanSettings settings) {
	
	glfwSetErrorCallback(glfw_error_callback);
	
	if (glfwInit() != GLFW_TRUE) {
		glfwTerminate();
		throw std::runtime_error("Toucan error! Unable to initialize GLFW.\n");
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, settings.resizeable ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_FLOATING, settings.floating ? GLFW_TRUE : GLFW_FALSE);
	
	toucan_context_ptr->window_ptr = glfwCreateWindow(settings.width, settings.height, "Toucan", nullptr, nullptr);
	
	if (toucan_context_ptr->window_ptr == nullptr) {
		glfwTerminate();
		throw std::runtime_error("Toucan error! Unable to create GLFW Window.\n");
	}
	
	glfwSetWindowCloseCallback(toucan_context_ptr->window_ptr, glfw_window_close_callback);
	glfwMakeContextCurrent(toucan_context_ptr->window_ptr);
	
	glfwSwapInterval(1);
	
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		glfwTerminate();
		throw std::runtime_error("Toucan error! Unable to load OpenGL.\n");
	}
	
#if !NDEBUG
	glad_set_post_callback(post_call_callback);
#endif
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(toucan_context_ptr->window_ptr, true);
	constexpr auto imgui_glsl_version = "#version 330 core";
	ImGui_ImplOpenGL3_Init(imgui_glsl_version);
	
	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	
	// Load renderdoc
#if !NDEBUG
	if (void* mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD)) {
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI) dlsym(mod, "RENDERDOC_GetAPI");
		RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_1, (void**) &toucan_context_ptr->rdoc_api);
	}
#endif
	
	toucan_context_ptr->initialized_cv.notify_all();
	
	auto& imgui_style = ImGui::GetStyle();
	imgui_style.WindowMinSize = ImVec2(200.0f, 200.0f);
	
	while (toucan_context_ptr->should_render and not glfwWindowShouldClose(toucan_context_ptr->window_ptr)) {
		const auto frame_start = std::chrono::steady_clock::now();
		
		glfwPollEvents();
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		glClearColor(.2f, .2f, .2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // NOLINT
		
		ImGui::DockSpaceOverViewport();
		
		// Draw all Figure2Ds
		draw_figure_2d_list(toucan_context_ptr->figures_2d, *toucan_context_ptr);
		
		// Draw all Figure3Ds
		draw_figure_3d_list(toucan_context_ptr->figures_3d, *toucan_context_ptr);
		
		// Draw all InputWindows
		draw_figure_input_list(toucan_context_ptr->input_windows, *toucan_context_ptr);
		
		// Call ImGui render functions
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(toucan_context_ptr->window_ptr);
		
		const auto frame_end = std::chrono::steady_clock::now();
		const auto current_frame_duration = frame_end - frame_start;
		
		using namespace std::chrono_literals;
		const auto min_frame_duration = 1000ms / settings.max_frames_per_second;
		
		if (current_frame_duration < min_frame_duration) {
			std::this_thread::sleep_for(min_frame_duration - current_frame_duration);
		}
	}
	
	ImGui::DestroyContext();
	glfwTerminate();
	
	toucan_context_ptr->window_open = false;
	toucan_context_ptr->window_close_cv.notify_all();
}

static void glfw_error_callback(int error, const char* description) {
	std::ostringstream ss;
	ss << "GLFW error! (" << error << "): " << description << '\n';
	throw std::runtime_error(ss.str());
}

static void glfw_window_close_callback(GLFWwindow* window) {
	toucan_context_ptr->window_open = false;
}

} // namespace Toucan
