#include "vulkan_rendering_context.h"
#include "window.h"
#include "camera.h"
#include <chrono>

namespace engine {

    class Renderer {
    public:
            Renderer() = default;
            ~Renderer() = default;

            bool initialize(const glfw_window& window) {
                m_renderingContext = std::make_unique<graphics::VulkanRenderingContext>(window);
				m_camera = std::make_unique<engine::Camera>();
                
                // Initialize the camera properly
                setupCamera(window);
                
                return m_renderingContext->initialize();
            }

            void shutdown() {
                if (m_renderingContext) {
                    m_renderingContext->shutdown();
                }
            }

            void render() {
                if (m_renderingContext) {
					auto tStart = std::chrono::high_resolution_clock::now();
                    
                    // Begin frame and check if we should render
                    if (!m_renderingContext->beginFrame()) {
                        return; // Skip this frame if beginFrame fails
                    }
                    
                    // Render the frame
                    m_renderingContext->render(*m_camera);
					
					// Calculate frame timing
					auto tEnd = std::chrono::high_resolution_clock::now();
					auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
					float frameTimer = (float)tDiff / 1000.0f;
					
					// Update camera
					m_camera->update(frameTimer);

                    // End frame
                    m_renderingContext->endFrame();
                }
            }

    private:
        std::unique_ptr<graphics::VulkanRenderingContext> m_renderingContext;
        std::unique_ptr<engine::Camera> m_camera;
        
        void setupCamera(const glfw_window& window) {
            // Get window dimensions
            int width, height;
            window.getWindowSize(&width, &height);
            
            // Set camera type to lookat for simple triangle viewing
            m_camera->type = engine::Camera::CameraType::lookat;
            
            // Set up perspective projection
            float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
            m_camera->setPerspective(45.0f, aspectRatio, 0.1f, 10.0f);
            
            // Position camera to see the triangle
            // Triangle vertices are at -1 to 1, so place camera at z=3 to see it
            m_camera->setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
            m_camera->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
            
            // Ensure flipY is set correctly for Vulkan
            m_camera->flipY = true;
            
            std::cout << "Camera initialized:" << std::endl;
            std::cout << "  Position: (0, 0, 3)" << std::endl;
            std::cout << "  Rotation: (0, 0, 0)" << std::endl;
            std::cout << "  FOV: 45 degrees" << std::endl;
            std::cout << "  Aspect: " << aspectRatio << std::endl;
            std::cout << "  Near/Far: 0.1/10.0" << std::endl;
            std::cout << "  FlipY: true (Vulkan)" << std::endl;
        }
    };
}
