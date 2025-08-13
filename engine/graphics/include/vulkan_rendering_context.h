#pragma once

#include "rendering_context.h"
#include <vulkan/vulkan.h>
#include "window.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_tools.h"
#include "vulkan_gui.h"
#include <vector>
#include <string>
#include <memory>
#include <array>
#include "vulkan_benchmark.h"
#include "vulkan_utils.h"

namespace engine::graphics {
    // -----------------------
    // Forward declarations
    // -----------------------
    class VulkanRenderingContext : public RenderingContext {
    public:
        VulkanRenderingContext(const glfw_window& window);
        ~VulkanRenderingContext() override;
        
        // RenderingContext implementation
		bool initialize() override;
        void shutdown() override;
        bool beginFrame() override;
        void endFrame() override;
        void render(const Camera& camera) override;
        void renderOverlay(const Camera& camera);
        void renderGame(const Camera& camera);
        void recreateSurface(uint32_t width, uint32_t height) override;
        void getDrawableSize(uint32_t& width, uint32_t& height) const override;
        uint32_t getCurrentFrameIndex() const override { return m_currentFrame; }
        uint32_t getMaxFramesInFlight() const override { return MAX_FRAMES_IN_FLIGHT; }
        
        // Vulkan-specific accessors
        VkInstance getInstance() const { return m_instance; }
        const vulkan_utils::VulkanDevice& getDevice() const { return *m_device; }
        VkSurfaceKHR getSurface() const { return m_surface; }
        const vulkan_utils::VulkanSwapChain& getSwapchain() const { return *m_swapchain; }
        
        // Frame synchronization
        VkSemaphore getImageAvailableSemaphore() const { return m_imageAvailableSemaphores[m_currentFrame]; }
        VkSemaphore getRenderFinishedSemaphore() const { return m_renderFinishedSemaphores[m_currentFrame]; }
        VkFence getInFlightFence() const { return m_inFlightFences[m_currentFrame]; }
        
        // Command buffer access
        VkCommandPool getCommandPool() const { return m_commandPool; }
        VkCommandBuffer getCurrentCommandBuffer() const { return m_commandBuffers[m_currentFrame]; }
        
        // State queries
        bool isInitialized() const { return m_initialized; }
        bool isSwapchainValid() const { return m_swapchain != VK_NULL_HANDLE; }
        uint32_t getCurrentImageIndex() const { return m_currentImageIndex; }
        
        // Debug utilities
        //void setObjectName(uint64_t object, VkObjectType objectType, const char* name);
        
    private:
        static const int MAX_FRAMES_IN_FLIGHT = 3;
        static constexpr uint32_t MAX_CONCURRENT_FRAMES = MAX_FRAMES_IN_FLIGHT;
        
        // Initialization state
        bool m_initialized = false;
        const glfw_window& m_window;
        
        // Core Vulkan objects
        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        std::unique_ptr<vulkan_utils::VulkanDevice> m_device;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;
        vulkan_utils::DepthStencil m_depthStencil;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
        GUI::UIOverlay m_gui;
        vulkan_utils::Benchmark m_benchmark;
        // List of available frame buffers (same as number of swap chain images)
        std::vector<VkFramebuffer> m_framebuffers;
        std::vector<VkShaderModule> m_shaderModules;
        vulkan_utils::vulkan_vertex_buffer m_vertexBuffer;
        vulkan_utils::vulkan_index_buffer m_indexBuffer;
        std::array<vulkan_utils::vulkan_uniform_buffer, MAX_CONCURRENT_FRAMES> m_uniformBuffers;
        // The descriptor set layout describes the shader binding layout (without actually referencing descriptor)
	    // Like the pipeline layout it's pretty much a blueprint and can be used with different descriptor sets as long as their layout matches
	    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        std::unique_ptr<vulkan_utils::VulkanSwapChain> m_swapchain;
        
        // Frame management
        uint32_t m_currentFrame = 0;
        uint32_t m_currentImageIndex = 0;
        bool m_framebufferResized = false;
        
        // Command recording
        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> m_commandBuffers;
        
        // Synchronization
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        
        // Debug (if enabled)
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        bool m_enableValidationLayers = false;
        
        // Validation layers
        const std::vector<const char*> m_validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
        
        // Required device extensions
        const std::vector<const char*> m_deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        
        // Helper methods
        bool createInstance();
        bool setupDebugMessenger();
        bool createSurface();
        bool selectPhysicalDevice();
        bool createLogicalDevice();
        bool createSwapchain();
        bool createCommandPool();
        bool createCommandBuffers();
        bool createSyncObjects();
        bool setupDepthStencil();
        bool setupRenderPass();
        bool createPipelineCache();
        bool setupFrameBuffer();
        bool setupUIOverlay();
		bool createVertexBuffer();
		bool createUniformBuffers();
		bool createDescriptorSetLayout();
		bool createDescriptorPool();
		bool createDescriptorSets();
		bool createPipelines();
        VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);

        // Debug callback
       /* static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);*/
    };
    
} // namespace engine::graphics