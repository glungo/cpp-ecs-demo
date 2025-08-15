#include "vulkan_rendering_context.h"
#include "window.h"
#include "camera.h"
#include "input_manager.h"
#include <chrono>

namespace engine {

    class Renderer {
    public:
            Renderer() = default;
            ~Renderer() = default;

            bool initialize(const glfw_window& window) {
                m_renderingContext = std::make_unique<graphics::VulkanRenderingContext>(window);
				m_camera = std::make_unique<engine::Camera>();
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
					auto tEnd = std::chrono::high_resolution_clock::now();
					auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
					float frameTimer = (float)tDiff / 1000.0f;
					
					// Update camera
					m_camera->update(frameTimer);
                    m_renderingContext->endFrame();
                }
            }
            
            void handleResize(int width, int height) {
                if (m_renderingContext && width > 0 && height > 0) {
                    m_renderingContext->setFramebufferResized(true);
                    m_renderingContext->recreateSurface(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
                    if (m_camera) {
                        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
                        m_camera->updateAspectRatio(aspectRatio);
                    }
                }
            }

            Camera& getCamera() { return *m_camera; }

            void UpdateCameraInput(const engine::InputManager& inputManager) {
                if (m_camera) {
                    // Map WASD to camera keys
                    m_camera->keys.up = inputManager.isKeyDown(GLFW_KEY_W);
                    m_camera->keys.down = inputManager.isKeyDown(GLFW_KEY_S);
                    m_camera->keys.left = inputManager.isKeyDown(GLFW_KEY_A);
                    m_camera->keys.right = inputManager.isKeyDown(GLFW_KEY_D);
                }
			}
    private:
        std::unique_ptr<graphics::VulkanRenderingContext> m_renderingContext;
        std::unique_ptr<engine::Camera> m_camera;
        
        void setupCamera(const glfw_window& window) {
            int width, height; window.getWindowSize(&width, &height);
            m_camera->type = engine::Camera::CameraType::firstperson; // enable movement
            m_camera->flipY = true;
            float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
            m_camera->setPerspective(60.0f, aspectRatio, 1.0, 256.0f);
            m_camera->setPosition(glm::vec3(0.0f, 0.0f, -3.0f));
            m_camera->setRotation(glm::vec3(0.0f));
            m_camera->movementSpeed = 5.0f;
            m_camera->rotationSpeed = 1.0f;
        }
    };
}
