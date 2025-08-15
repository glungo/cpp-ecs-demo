#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include "platform_utils.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Forward declarations for platform-specific window implementation
namespace engine {
    
    struct WindowConfig {
        int width = 1280;
        int height = 720;
        std::string title = "Engine Window";
        bool fullscreen = false;
        bool resizable = true;
        bool vsync = true;
    };

    class Window {
    public:    
        Window() = default;
        ~Window() = default;
        
        // Core functionality
        virtual void shutdown() = 0;
        virtual void pollEvents() = 0;
        virtual bool shouldClose() const = 0;
        virtual void swapBuffers() = 0;
        
        // Properties
        virtual void getFramebufferSize(int* width, int* height) const = 0;
        virtual void getWindowSize(int* width, int* height) const = 0;
        virtual float getAspectRatio() const = 0;
        
        void setResizeCallback(std::function<void(int, int)> callback) {
            m_resizeCallback = callback;
        }
        void setKeyCallback(std::function<void(int, int, int, int)> callback){
            m_keyCallback = callback;
		}
		// Mouse and cursor callbacks
        void setMouseButtonCallback(std::function<void(int, int, int)> callback){
			m_mouseButtonCallback = callback;
        }
        void setCursorPosCallback(std::function<void(double, double)> callback) {
			m_cursorPosCallback = callback;
        }
        void setScrollCallback(std::function<void(double, double)> callback) {
            m_scrollCallback = callback;
        }

    protected:
        virtual bool initialize(const WindowConfig& config) = 0;

        WindowConfig m_config;
        // Event callback storage
        std::function<void(int, int)> m_resizeCallback;
        std::function<void(int, int, int, int)> m_keyCallback;
        std::function<void(int, int, int)> m_mouseButtonCallback;
        std::function<void(double, double)> m_cursorPosCallback;
        std::function<void(double, double)> m_scrollCallback;
    };

    // Forward declaration for GLFW-specific window implementation
    // GLFW-specific window implementation
    class glfw_window : public Window
    {
    public:
        glfw_window();
        ~glfw_window();

        void shutdown() override;
        void pollEvents() override {
            glfwPollEvents();
		}
        bool shouldClose() const override { return glfwWindowShouldClose(m_windowHandle->handle); }
        void swapBuffers() override { glfwSwapBuffers(m_windowHandle->handle); }
        void getFramebufferSize(int* width, int* height) const override {
            glfwGetFramebufferSize(m_windowHandle->handle, width, height);
        }
        void getWindowSize(int* width, int* height) const override {
            glfwGetWindowSize(m_windowHandle->handle, width, height);
        }
        float getAspectRatio() const override {
            int width, height;
            getFramebufferSize(&width, &height);
            return static_cast<float>(width) / static_cast<float>(height);
        }

        void hookResizeCallback() {
 
            glfwSetFramebufferSizeCallback(m_windowHandle->handle, [](GLFWwindow* window, int width, int height) {
                //get the window pointer from user pointer
                glfw_window* self = static_cast<glfw_window*>(glfwGetWindowUserPointer(window));
                if (self->m_resizeCallback) {
                    self->m_resizeCallback(width, height);
                }
                });
        }

        void hookKeyCallback() {
            glfwSetKeyCallback(m_windowHandle->handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                //get the window pointer from user pointer
                glfw_window* self = static_cast<glfw_window*>(glfwGetWindowUserPointer(window));
                if (self->m_keyCallback) {
                    self->m_keyCallback(key, scancode, action, mods);
                }
			});
        }
        // Mouse and cursor callbacks
        void hookMouseButtonCallback() {
           
            glfwSetMouseButtonCallback(m_windowHandle->handle, [](GLFWwindow* window, int button, int action, int mods) {
				//get the window pointer from user pointer
				glfw_window* self = static_cast<glfw_window*>(glfwGetWindowUserPointer(window));
                if (self->m_mouseButtonCallback) {
                    self->m_mouseButtonCallback(button, action, mods);
                }
                });
        }

        void hookCursorPosCallback() {
            glfwSetCursorPosCallback(m_windowHandle->handle, [](GLFWwindow* window, double xpos, double ypos) {
				//get the window pointer from user pointer
				glfw_window* self = static_cast<glfw_window*>(glfwGetWindowUserPointer(window));
                if (self->m_cursorPosCallback) {
                    self->m_cursorPosCallback(xpos, ypos);
                }
                });
        }
        void hookScrollCallback() {
            glfwSetScrollCallback(m_windowHandle->handle, [](GLFWwindow* window, double xoffset, double yoffset) {
                glfw_window* self = static_cast<glfw_window*>(glfwGetWindowUserPointer(window));
                if (self->m_scrollCallback) {
                    self->m_scrollCallback(xoffset, yoffset);
                }
            });
        }

		// Window handle access
        std::shared_ptr<platform::WindowHandle<GLFWwindow>> getWindowHandle() const {
            return m_windowHandle;
		}
      
    protected:
        bool initialize(const WindowConfig& config) override;

    private:
         std::shared_ptr<platform::WindowHandle<GLFWwindow>> m_windowHandle;
    };

} // namespace engine