#include "window.h"

// GLFW-specific window implementation
// Ensure that the Vulkan header is included before GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "platform_utils.h"
#include <memory>
#include "LogMacros.h"

namespace engine {

    glfw_window::glfw_window() : Window(), m_windowHandle(std::make_unique<platform::WindowHandle<GLFWwindow>>()) {
        // Initialize GLFW and create a window
        WindowConfig config;

        if (!initialize(config)) {
			LOG_ERROR << "Failed to initialize GLFW window system" << LOG_END;
        }
    }

    glfw_window::~glfw_window(){
        shutdown();
    }

    bool glfw_window::initialize(const WindowConfig& config) {
        m_config = config;

        if (!glfwInit()) {
            return false;
        }

        m_windowHandle = std::make_shared<platform::WindowHandle<GLFWwindow>>();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // ENABLE RESIZING

        m_windowHandle->handle = glfwCreateWindow(m_config.width, m_config.height, m_config.title.c_str(), nullptr, nullptr);
        if (!m_windowHandle->handle) {
            glfwTerminate();
            return false;
        }
        //set up the window abstraction so we can hook into the callbacks
		glfwSetWindowUserPointer(m_windowHandle->handle, this);

		hookCursorPosCallback();
		hookMouseButtonCallback();
		hookKeyCallback();
		hookResizeCallback();
        
        return true;
    }

    void glfw_window::shutdown() {
        if (m_windowHandle && m_windowHandle->handle) {
            glfwDestroyWindow(m_windowHandle->handle);
            m_windowHandle.reset();
        }
        glfwTerminate();
    }
}
