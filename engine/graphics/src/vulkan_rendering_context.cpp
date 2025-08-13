#include "vulkan_rendering_context.h"
#include <vulkan/vulkan.h>
#include "vulkan_device.h"
#include "vulkan_tools.h"
#include "window.h"
#include <set>
#include <vector>
#include <array>
#include "utils.h"
#include "camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace engine::graphics;

// Define the Vertex structure
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

// Define the ShaderData structure that matches what the shaders expect
struct ShaderData {
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;
};

namespace vulkan_utils {
    struct vulkan_vertex_buffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };
    
    struct vulkan_index_buffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
        uint32_t count;
    };
    
    struct vulkan_uniform_buffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
        VkDescriptorSet descriptorSet;
        void* mapped;
    };
}

VulkanRenderingContext::VulkanRenderingContext(const glfw_window& window) :
	m_window(window),
	m_instance(VK_NULL_HANDLE),
	m_surface(VK_NULL_HANDLE),
	m_depthStencil(),
	m_framebuffers(),
	m_shaderModules(),
	m_gui(),
	m_benchmark(),
	m_swapchain(std::make_unique<vulkan_utils::VulkanSwapChain>()),
	m_currentFrame(0),
	m_currentImageIndex(0),
	m_framebufferResized(false),
	m_commandPool(VK_NULL_HANDLE) {
}

VulkanRenderingContext::~VulkanRenderingContext() {
	if (m_initialized) {
		shutdown();
	}
}

bool VulkanRenderingContext::initialize() {
	if (m_window.getWindowHandle() == nullptr) {
		return false; // No window handle available
	}
	if (m_initialized) {
		return false; // Already initialized
	}
	if (!createInstance()) {
		return false;
	}
	if (!setupDebugMessenger()) {
		return false;
	}
	if (!selectPhysicalDevice()) {
		return false;
	}
	if (!createLogicalDevice()) {
		return false;
	}
	if (!createSurface()) {
		return false;
	}
	if (!createSwapchain()) {
		return false;
	}
	if (!createCommandPool()) {
		return false;
	}
	if (!createCommandBuffers()) {
		return false;
	}
	if (!createSyncObjects()) {
		return false;
	}
	if (!setupDepthStencil()) {
		return false;
	}
	if (!setupRenderPass()) {
		return false;
	}
	if (!createPipelineCache()) {
		return false;
	}
	if (!setupFrameBuffer()) {
		return false;
	}
	if (!setupUIOverlay()) {
		return false;
	}
	if (!createVertexBuffer()) {
		return false;
	}
	if (!createUniformBuffers()) {
		return false;
	}
	if (!createDescriptorSetLayout()) {
		return false;
	}
	if (!createDescriptorPool()) {
		return false;
	}
	if (!createDescriptorSets()) {
		return false;
	}
	if (!createPipelines()) {
		return false;
	}
	m_initialized = true;
	return true;
}

void VulkanRenderingContext::shutdown() {
	if (!m_initialized) {
		return; // Nothing to do
	}
	
	vkDeviceWaitIdle(m_device->logicalDevice);
	
	// Cleanup synchronization objects
	for (size_t i = 0; i < m_inFlightFences.size(); ++i) {
		if (i < m_imageAvailableSemaphores.size()) {
			vkDestroySemaphore(m_device->logicalDevice, m_imageAvailableSemaphores[i], nullptr);
		}
		if (i < m_renderFinishedSemaphores.size()) {
			vkDestroySemaphore(m_device->logicalDevice, m_renderFinishedSemaphores[i], nullptr);
		}
		vkDestroyFence(m_device->logicalDevice, m_inFlightFences[i], nullptr);
	}
	
	// Cleanup command buffers
	if (!m_commandBuffers.empty()) {
		vkFreeCommandBuffers(m_device->logicalDevice, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
	}
	
	if (m_commandPool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(m_device->logicalDevice, m_commandPool, nullptr);
	}
	
	// Cleanup swapchain resources
	if (m_swapchain) {
		m_swapchain->cleanup();
	}
	
	// Cleanup surface and instance
	if (m_surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}
	
	if (m_instance != VK_NULL_HANDLE) {
		vkDestroyInstance(m_instance, nullptr);
	}
	
	m_initialized = false;
	std::cout << "VulkanRenderingContext shutdown completed" << std::endl;
}

bool VulkanRenderingContext::beginFrame() {
	if (!m_initialized) {
		return false; // Not initialized
	}
	
	static int beginFrameCount = 0;
	bool shouldPrint =false;

	if (shouldPrint) {
		std::cout << ">>> beginFrame() " << beginFrameCount << " - m_currentFrame: " << m_currentFrame << std::endl;
	}
	
	// Wait for the current frame to finish
	VkResult waitResult = vkWaitForFences(m_device->logicalDevice, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
	if (waitResult != VK_SUCCESS) {
		std::cerr << "Failed to wait for fence! VkResult: " << waitResult << std::endl;
		return false;
	}
	
	// Reset the fence for the next frame
	VkResult resetResult = vkResetFences(m_device->logicalDevice, 1, &m_inFlightFences[m_currentFrame]);
	if (resetResult != VK_SUCCESS) {
		std::cerr << "Failed to reset fence! VkResult: " << resetResult << std::endl;
		return false;
	}
	
	// Acquire the next image from the swapchain
	VkResult result = m_swapchain->acquireNextImage(m_imageAvailableSemaphores[m_currentFrame], m_currentImageIndex);
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		std::cerr << "Failed to acquire next image! VkResult: " << result << std::endl;
		return false; // Failed to acquire next image
	}
	
	if (shouldPrint) {
		std::cout << ">>> Acquired swapchain image: " << m_currentImageIndex << std::endl;
	}
	
	beginFrameCount++;
	return true; // Frame started successfully
}

void VulkanRenderingContext::endFrame() {
	static int endFrameCount = 0;
	bool shouldPrint = (endFrameCount % 60 == 0);
	
	if (shouldPrint) {
		std::cout << "<<< endFrame() " << endFrameCount << " - Advancing from frame " << m_currentFrame;
	}
	
	// Advance to next frame
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	
	if (shouldPrint) {
		std::cout << " to frame " << m_currentFrame << std::endl;
	}
	endFrameCount++;
}

void VulkanRenderingContext::render(const Camera& camera) {
	renderGame(camera);
	renderOverlay(camera);
	//after frame began successfully present the next frame

	
}
void VulkanRenderingContext::renderOverlay(const Camera& camera) {
	// Record the command buffer for rendering the UI overlay
	
}
void VulkanRenderingContext::renderGame(const Camera& camera) {
	// Debug: Print frame information at the very beginning
	static int debugFrameCount = 0;
	bool shouldPrint = (debugFrameCount % 60 == 0);
	
	if (shouldPrint) {
		std::cout << "=== RENDER FRAME " << debugFrameCount << " ===" << std::endl;
		std::cout << "  m_currentFrame: " << m_currentFrame << std::endl;
		std::cout << "  m_currentImageIndex: " << m_currentImageIndex << std::endl;
		std::cout << "  Command buffer count: " << m_commandBuffers.size() << std::endl;
		std::cout << "  Framebuffer count: " << m_framebuffers.size() << std::endl;
	}
	
	// Update the uniform buffer for the CURRENT FRAME (not image index!)
	ShaderData shaderData{};
	// DEBUGGING: Use identity matrices to render triangle in screen space
	shaderData.projectionMatrix = glm::mat4(1.0f); // Identity matrix
	shaderData.viewMatrix = glm::mat4(1.0f);       // Identity matrix  
	shaderData.modelMatrix = glm::mat4(1.0f);      // Identity matrix

	// Debug: Print matrix values occasionally
	if (shouldPrint) {
		std::cout << "  Using IDENTITY matrices for debugging (no camera transform)" << std::endl;
		std::cout << "  Camera position: (" << camera.position.x << ", " << camera.position.y << ", " << camera.position.z << ")" << std::endl;
		std::cout << "  Triangle vertices in clip space: (-1,-1,0) to (1,1,0)" << std::endl;
	}

	// Copy to the uniform buffer that matches the descriptor set we'll bind
	memcpy(m_uniformBuffers[m_currentFrame].mapped, &shaderData, sizeof(ShaderData));

	// Use command buffer that matches the swapchain image (if we have enough)
	uint32_t commandBufferIndex = (m_commandBuffers.size() > m_currentImageIndex) ? m_currentImageIndex : (m_currentImageIndex % m_commandBuffers.size());
	
	if (shouldPrint) {
		std::cout << "  Using command buffer[" << commandBufferIndex << "] for image[" << m_currentImageIndex << "]" << std::endl;
		std::cout << "  Using uniform buffer[" << m_currentFrame << "] for frame sync" << std::endl;
	}

	// Build the command buffer - USE CORRECT INDEX
	vkResetCommandBuffer(m_commandBuffers[commandBufferIndex], 0);

	VkCommandBufferBeginInfo cmdBufInfo{};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Set clear values for all framebuffer attachments with loadOp set to clear
	VkClearValue clearValues[2]{};
	clearValues[0].color = { { 0.3f, 0.6f, 0.2f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	int width, height;
	m_window.getWindowSize(&width, &height);
	renderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>(width);
	renderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(height);
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = m_framebuffers[m_currentImageIndex];

	const VkCommandBuffer commandBuffer = m_commandBuffers[commandBufferIndex];  // FIXED!
	VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));

	// Start the first sub pass specified in our default render pass setup by the base class
	// This will clear the color and depth attachment
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	// Update dynamic viewport state
	VkViewport viewport{};
	viewport.height = static_cast<float>(height);
	viewport.width = static_cast<float>(width);
	viewport.minDepth = static_cast<float>(0.0f);
	viewport.maxDepth = static_cast<float>(1.0f);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	// Update dynamic scissor state
	VkRect2D scissor{};
	scissor.extent.width = static_cast<uint32_t>(width);
	scissor.extent.height = static_cast<uint32_t>(height);
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	// Bind descriptor set for the current frame's uniform buffer (CORRECT INDEX!)
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_uniformBuffers[m_currentFrame].descriptorSet, 0, nullptr);
	// Bind the rendering pipeline
	// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
	// Bind triangle vertex buffer (contains position and colors)
	VkDeviceSize offsets[1]{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.buffer, offsets);
	// Bind triangle index buffer
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	// Draw indexed triangle
	vkCmdDrawIndexed(commandBuffer, m_indexBuffer.count, 1, 0, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
	// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to
	// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system
	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

	// Submit the command buffer to the graphics queue
	// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// The submit info structure specifies a command buffer queue submission batch
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &waitStageMask;      // Pointer to the list of pipeline stages that the semaphore waits will occur at
	submitInfo.pCommandBuffers = &commandBuffer;		// Command buffers(s) to execute in this batch (submission)
	submitInfo.commandBufferCount = 1;                  // We submit a single command buffer

	// Semaphore to wait upon before the submitted command buffer starts executing
	submitInfo.pWaitSemaphores = &m_imageAvailableSemaphores[m_currentFrame];
	submitInfo.waitSemaphoreCount = 1;
	// Semaphore to be signaled when command buffers have completed
	submitInfo.pSignalSemaphores = &m_renderFinishedSemaphores[m_currentFrame];
	submitInfo.signalSemaphoreCount = 1;

	// Submit to the graphics queue passing a wait fence
	VK_CHECK_RESULT(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]));

	// Present the current frame buffer to the swap chain
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapchain->swapChain;
	presentInfo.pImageIndices = &m_currentImageIndex;
	VkResult result = vkQueuePresentKHR(m_graphicsQueue, &presentInfo);

	// Handle swapchain recreation if needed
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
		m_framebufferResized = false;
		std::cout << "Swapchain needs recreation (result: " << result << ")" << std::endl;
	} else if (result != VK_SUCCESS) {
		std::cerr << "Failed to present swap chain image! VkResult: " << result << std::endl;
	}

	if (shouldPrint) {
		std::cout << "  Submitted command buffer[" << commandBufferIndex << "] using fence[" << m_currentFrame << "]" << std::endl;
		std::cout << "  Presented image[" << m_currentImageIndex << "]" << std::endl;
	}
	
	debugFrameCount++;
}

void VulkanRenderingContext::recreateSurface(uint32_t width, uint32_t height)
{

}

void VulkanRenderingContext::getDrawableSize(uint32_t& width, uint32_t& height) const
{
	//TODO: DO WE NEED THIS?
	/*if (m_swapchain) {
		
		m_swapchain->images[0]->getSize(width, height);0
    } else {
        width = 0;
        height = 0;
    }*/
}

// private methods
bool VulkanRenderingContext::createInstance() {
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(&createInfo.enabledExtensionCount);
	if (m_enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
		createInfo.ppEnabledLayerNames = m_validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}
	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
		return false;
	}
	return true;
}

bool VulkanRenderingContext::setupDebugMessenger() {
	if (!m_enableValidationLayers) {
		return true; // Debug messenger not needed
	}
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	//createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional
	/*if (vkCreateDebugUtilsMessenger_EXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
		return false;
	}*/
	return true;
}

bool VulkanRenderingContext::createSurface() {
	if (!m_window.getWindowHandle()->handle) {
		return false; // No window to create surface for
	}

	glfwCreateWindowSurface(m_instance, m_window.getWindowHandle()->handle, nullptr, &m_surface);
	if (m_surface == VK_NULL_HANDLE) {
		std::cerr << "Failed to create Vulkan surface." << std::endl;
		return false; // Surface creation failed
	}
	//currently only supporting windows and glfw
	m_swapchain->initSurface(m_instance, m_device->physicalDevice, m_device->logicalDevice, m_surface);
	return true;
}

bool VulkanRenderingContext::selectPhysicalDevice() {
	uint32_t deviceCount = 0;
	VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceMemoryProperties selectedDeviceMemoryProps;

	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		return false; // No Vulkan-capable devices found
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
	//cache for the memory properties of the device
	VkPhysicalDeviceMemoryProperties memoryProperties;
	for (const auto& device : devices) {
		vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
		if (selectedDevice == VK_NULL_HANDLE) {
			// First device found, set it as the physical device
			selectedDevice = device;
			selectedDeviceMemoryProps = memoryProperties; // Cache the memory properties
			continue; // Continue to check other devices
		}
		
		// Compare memory properties to find the best device
		if (selectedDeviceMemoryProps.memoryHeapCount < memoryProperties.memoryHeapCount) {
			selectedDevice = device;
			selectedDeviceMemoryProps = memoryProperties; // Update to the new device's memory properties
		}
	}

	if (selectedDevice != VK_NULL_HANDLE) {
		//get the properties and features of the selected physical device
		m_device = std::make_unique<vulkan_utils::VulkanDevice>(selectedDevice);
		return true; // A suitable physical device was found
	}
	return false;
}

bool VulkanRenderingContext::createLogicalDevice() {
	assert(m_device != nullptr);
	if (m_device->physicalDevice == VK_NULL_HANDLE) {
		return false; // No physical device selected
	}

	auto result = m_device->createLogicalDevice();
	if (result != VK_SUCCESS) {
		std::cerr << "Could not create Vulkan device: \n" << result << std::endl;
		return false;
	}

	// Get a graphics queue from the device
	vkGetDeviceQueue(m_device->logicalDevice, m_device->queueFamilyIndices.graphics, 0, &m_graphicsQueue);
	
	// Samples that make use of stencil will require a depth + stencil format, so we select from a different list
	VkBool32 validFormat = vulkan_utils::getSupportedDepthStencilFormat(m_device->physicalDevice, &m_depthFormat);
	if (!validFormat) {
		std::cerr << "Failed to find a supported depth format." << std::endl;
		return false; // No valid depth format found
	}
	return true; // Logical device created successfully
}

bool VulkanRenderingContext::createSwapchain() {  
   
    int width, height;  
    m_window.getWindowSize(&width, &height);  

    uint32_t swapchainWidth = static_cast<uint32_t>(width);  
    uint32_t swapchainHeight = static_cast<uint32_t>(height);  

    // Fix for E0140: Adjust the arguments to match the function signature  
    m_swapchain->create(swapchainWidth, swapchainHeight, m_device->logicalDevice);

    return true; // Ensure the function returns a value  
}

bool VulkanRenderingContext::createCommandPool() {
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = m_swapchain->queueNodeIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	return vkCreateCommandPool(m_device->logicalDevice, &cmdPoolInfo, nullptr, &m_commandPool) == VK_SUCCESS;
}

bool VulkanRenderingContext::createCommandBuffers() {
	// Create one command buffer for each frame in flight (not each swap chain image)
	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	VkCommandBufferAllocateInfo cmdBufAllocateInfo = vulkan_utils::initializers::commandBufferAllocateInfo(m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(m_commandBuffers.size()));
	VkResult result = vkAllocateCommandBuffers(m_device->logicalDevice, &cmdBufAllocateInfo, m_commandBuffers.data());
	
	if (result == VK_SUCCESS) {
		std::cout << "Created " << m_commandBuffers.size() << " command buffers for frames in flight" << std::endl;
	} else {
		std::cerr << "Failed to allocate command buffers. VkResult: " << result << std::endl;
	}
	
	return result == VK_SUCCESS;
}

bool VulkanRenderingContext::createSyncObjects() {
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start in signaled state
	
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (vkCreateSemaphore(m_device->logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device->logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_device->logicalDevice, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
			std::cerr << "Failed to create synchronization objects for frame " << i << std::endl;
			return false; // Failed to create synchronization objects
		}
	}
	
	std::cout << "Created synchronization objects for " << MAX_FRAMES_IN_FLIGHT << " frames in flight" << std::endl;
	return true; // Synchronization objects created successfully
}

bool VulkanRenderingContext::setupDepthStencil()
{
	VkImageCreateInfo imageCI{};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = m_depthFormat;
	int height, width;
	m_window.getWindowSize(&width, &height);
	imageCI.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VK_CHECK(vkCreateImage(m_device->logicalDevice, &imageCI, nullptr, &m_depthStencil.image))

	VkMemoryRequirements memReqs{};
	vkGetImageMemoryRequirements(m_device->logicalDevice, m_depthStencil.image, &memReqs);

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = m_device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK(vkAllocateMemory(m_device->logicalDevice, &memAlloc, nullptr, &m_depthStencil.memory));

	VK_CHECK(vkBindImageMemory(m_device->logicalDevice, m_depthStencil.image, m_depthStencil.memory, 0));

	VkImageViewCreateInfo imageViewCI{};
	imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCI.image = m_depthStencil.image;
	imageViewCI.format = m_depthFormat;
	imageViewCI.subresourceRange.baseMipLevel = 0;
	imageViewCI.subresourceRange.levelCount = 1;
	imageViewCI.subresourceRange.baseArrayLayer = 0;
	imageViewCI.subresourceRange.layerCount = 1;
	imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
	if (m_depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
		imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	VK_CHECK(vkCreateImageView(m_device->logicalDevice, &imageViewCI, nullptr, &m_depthStencil.view));
	
	return true;
}

bool VulkanRenderingContext::setupRenderPass()
{
	std::array<VkAttachmentDescription, 2> attachments = {};
	// Color attachment
	attachments[0].format = m_swapchain->colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// Depth attachment
	attachments[1].format = m_depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	// Subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies{};

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	dependencies[0].dependencyFlags = 0;

	dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].dstSubpass = 0;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = 0;
	dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	dependencies[1].dependencyFlags = 0;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK(vkCreateRenderPass(m_device->logicalDevice, &renderPassInfo, nullptr, &m_renderPass));
	return true;
}

bool VulkanRenderingContext::createPipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECK(vkCreatePipelineCache(m_device->logicalDevice, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));
	return true;
}

bool VulkanRenderingContext::setupFrameBuffer()
{
	// Create frame buffers for every swap chain image
	m_framebuffers.resize(m_swapchain->images.size());
	for (uint32_t i = 0; i < m_framebuffers.size(); i++)
	{
		const VkImageView attachments[2] = {
			m_swapchain->imageViews[i],
			// Depth/Stencil attachment is the same for all frame buffers
			m_depthStencil.view
		};
		VkFramebufferCreateInfo frameBufferCreateInfo{};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass = m_renderPass;
		frameBufferCreateInfo.attachmentCount = 2;
		frameBufferCreateInfo.pAttachments = attachments;
		int width, height;
		m_window.getWindowSize(&width, &height);
		frameBufferCreateInfo.width = width;
		frameBufferCreateInfo.height = height;
		frameBufferCreateInfo.layers = 1;
		VK_CHECK(vkCreateFramebuffer(m_device->logicalDevice, &frameBufferCreateInfo, nullptr, &m_framebuffers[i]));
	}
	return true;
}

bool VulkanRenderingContext::setupUIOverlay()
{
	// Provide the UI overlay with the VulkanDevice pointer for resource creation
	m_gui.device = m_device.get();
	m_gui.queue = m_graphicsQueue;

	m_gui.shaders = {
		loadShader("uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		loadShader("uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
	};
	m_gui.prepareResources();
	m_gui.preparePipeline(m_pipelineCache, m_renderPass, m_swapchain->colorFormat, m_depthFormat);
	return true;
}

VkPipelineShaderStageCreateInfo VulkanRenderingContext::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	std::string shaderBasePath = getShaderBasePath();
	fileName = shaderBasePath + fileName;
	shaderStage.module = vulkan_utils::loadShader(fileName.c_str(), m_device->logicalDevice);
	shaderStage.pName = "main";
	assert(shaderStage.module != VK_NULL_HANDLE);
	m_shaderModules.push_back(shaderStage.module);
	return shaderStage;
}

bool VulkanRenderingContext::createVertexBuffer()
{
	// Setup vertices - MUCH LARGER triangle for debugging visibility
	std::vector<Vertex> vertexBuffer{
			{ {  .5f,  .5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -.5f,  .5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.0f, -.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
	};
	uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

	// Setup indices
	std::vector<uint32_t> indexBuffer{ 0, 1, 2 };
	m_indexBuffer.count = static_cast<uint32_t>(indexBuffer.size());
	uint32_t indexBufferSize = m_indexBuffer.count * sizeof(uint32_t);

	std::cout << "Creating LARGE triangle with vertices:" << std::endl;
	for (size_t i = 0; i < vertexBuffer.size(); ++i) {
		const Vertex& vertex = vertexBuffer[i];
		std::cout << "  Vertex " << i << ": pos(" << vertex.position[0] << ", " << vertex.position[1] << ", " << vertex.position[2]
				  << ") color(" << vertex.color[0] << ", " << vertex.color[1] << ", " << vertex.color[2] << ")" << std::endl;
	}

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	// Static data like vertex and index buffer should be stored on the device memory for optimal (and fastest) access by the GPU
	//
	// To achieve this we use so-called "staging buffers" :
	// - Create a buffer that's visible to the host (and can be mapped)
	// - Copy the data to this buffer
	// - Create another buffer that's local on the device (VRAM) with the same size
	// - Copy the data from the host to the device using a command buffer
	// - Delete the host visible (staging) buffer
	// - Use the device local buffers for rendering
	//
	// Note: On unified memory architectures where host (CPU) and GPU share the same memory, staging is not necessary
	// To keep this sample easy to follow, there is no check for that in place

	struct StagingBuffer {
		VkDeviceMemory memory;
		VkBuffer buffer;
	};

	struct {
		StagingBuffer vertices;
		StagingBuffer indices;
	} stagingBuffers{};

	void* data;

	// Vertex buffer
	VkBufferCreateInfo vertexBufferInfoCI{};
	vertexBufferInfoCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfoCI.size = vertexBufferSize;
	// Buffer is used as the copy source
	vertexBufferInfoCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	// Create a host-visible buffer to copy the vertex data to (staging buffer)
	VK_CHECK(vkCreateBuffer(m_device->logicalDevice, &vertexBufferInfoCI, nullptr, &stagingBuffers.vertices.buffer));
	vkGetBufferMemoryRequirements(m_device->logicalDevice, stagingBuffers.vertices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	// Request a host visible memory type that can be used to copy our data to
	// Also request it to be coherent, so that writes are visible to the GPU right after unmapping the buffer
	memAlloc.memoryTypeIndex = vulkan_utils::getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_device->memoryProperties);
	VK_CHECK(vkAllocateMemory(m_device->logicalDevice, &memAlloc, nullptr, &stagingBuffers.vertices.memory));
	// Map and copy
	VK_CHECK(vkMapMemory(m_device->logicalDevice, stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, 0, &data));
	memcpy(data, vertexBuffer.data(), vertexBufferSize);
	vkUnmapMemory(m_device->logicalDevice, stagingBuffers.vertices.memory);
	VK_CHECK(vkBindBufferMemory(m_device->logicalDevice, stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0));

	// Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
	vertexBufferInfoCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VK_CHECK(vkCreateBuffer(m_device->logicalDevice, &vertexBufferInfoCI, nullptr, &m_vertexBuffer.buffer));
	vkGetBufferMemoryRequirements(m_device->logicalDevice, m_vertexBuffer.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkan_utils::getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_device->memoryProperties);
	VK_CHECK(vkAllocateMemory(m_device->logicalDevice, &memAlloc, nullptr, &m_vertexBuffer.memory));
	VK_CHECK(vkBindBufferMemory(m_device->logicalDevice, m_vertexBuffer.buffer, m_vertexBuffer.memory, 0));

	// Index buffer
	VkBufferCreateInfo indexbufferCI{};
	indexbufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexbufferCI.size = indexBufferSize;
	indexbufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	// Copy index data to a buffer visible to the host (staging buffer)
	VK_CHECK(vkCreateBuffer(m_device->logicalDevice, &indexbufferCI, nullptr, &stagingBuffers.indices.buffer));
	vkGetBufferMemoryRequirements(m_device->logicalDevice, stagingBuffers.indices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkan_utils::getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_device->memoryProperties);
	VK_CHECK(vkAllocateMemory(m_device->logicalDevice, &memAlloc, nullptr, &stagingBuffers.indices.memory));
	VK_CHECK(vkMapMemory(m_device->logicalDevice, stagingBuffers.indices.memory, 0, indexBufferSize, 0, &data));
	memcpy(data, indexBuffer.data(), indexBufferSize);
	vkUnmapMemory(m_device->logicalDevice, stagingBuffers.indices.memory);
	VK_CHECK(vkBindBufferMemory(m_device->logicalDevice, stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0));

	// Create destination buffer with device only visibility
	indexbufferCI.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VK_CHECK(vkCreateBuffer(m_device->logicalDevice, &indexbufferCI, nullptr, &m_indexBuffer.buffer));
	vkGetBufferMemoryRequirements(m_device->logicalDevice, m_indexBuffer.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkan_utils::getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_device->memoryProperties);
	VK_CHECK(vkAllocateMemory(m_device->logicalDevice, &memAlloc, nullptr, &m_indexBuffer.memory));
	VK_CHECK(vkBindBufferMemory(m_device->logicalDevice, m_indexBuffer.buffer, m_indexBuffer.memory, 0));

	// Buffer copies have to be submitted to a queue, so we need a command buffer for them
	// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies
	VkCommandBuffer copyCmd;

	VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = m_commandPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;
	VK_CHECK(vkAllocateCommandBuffers(m_device->logicalDevice, &cmdBufAllocateInfo, &copyCmd));

	VkCommandBufferBeginInfo cmdBufInfo = vulkan_utils::initializers::commandBufferBeginInfo();
	VK_CHECK(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));
	// Put buffer region copies into command buffer
	VkBufferCopy copyRegion{};
	// Vertex buffer
	copyRegion.size = vertexBufferSize;
	vkCmdCopyBuffer(copyCmd, stagingBuffers.vertices.buffer, m_vertexBuffer.buffer, 1, &copyRegion);
	// Index buffer
	copyRegion.size = indexBufferSize;
	vkCmdCopyBuffer(copyCmd, stagingBuffers.indices.buffer, m_indexBuffer.buffer,	1, &copyRegion);
	VK_CHECK(vkEndCommandBuffer(copyCmd));

	// Submit the command buffer to the queue to finish the copy
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCI{};
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.flags = 0;
	VkFence fence;
	VK_CHECK(vkCreateFence(m_device->logicalDevice, &fenceCI, nullptr, &fence));

	// Submit to the queue
	VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, fence));
	// Wait for the fence to signal that command buffer has finished executing
	VK_CHECK(vkWaitForFences(m_device->logicalDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

	vkDestroyFence(m_device->logicalDevice, fence, nullptr);
	vkFreeCommandBuffers(m_device->logicalDevice, m_commandPool, 1, &copyCmd);

	// Destroy staging buffers
	// Note: Staging buffer must not be deleted before the copies have been submitted and executed
	vkDestroyBuffer(m_device->logicalDevice, stagingBuffers.vertices.buffer, nullptr);
	vkFreeMemory(m_device->logicalDevice, stagingBuffers.vertices.memory, nullptr);
	vkDestroyBuffer(m_device->logicalDevice, stagingBuffers.indices.buffer, nullptr);
	vkFreeMemory(m_device->logicalDevice, stagingBuffers.indices.memory, nullptr);
	
	std::cout << "LARGE vertex and index buffers created successfully" << std::endl;
	return true;
}

bool VulkanRenderingContext::createUniformBuffers()
{
	// Prepare and initialize the per-frame uniform buffer blocks containing shader uniforms
	// Single uniforms like in OpenGL are no longer present in Vulkan. All hader uniforms are passed via uniform buffer blocks
	VkMemoryRequirements memReqs;

	// Vertex shader uniform buffer block
	VkBufferCreateInfo bufferInfo{};
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = 0;
	allocInfo.memoryTypeIndex = 0;

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(ShaderData);
	// This buffer will be used as a uniform buffer
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		// Create the buffers
	for (uint32_t i = 0; i < MAX_CONCURRENT_FRAMES; i++) {
		VK_CHECK(vkCreateBuffer(m_device->logicalDevice, &bufferInfo, nullptr, &m_uniformBuffers[i].buffer));
		// Get memory requirements including size, alignment and memory type
		vkGetBufferMemoryRequirements(m_device->logicalDevice, m_uniformBuffers[i].buffer, &memReqs);
		allocInfo.allocationSize = memReqs.size;
		// Get the memory type index that supports host visible memory access
		// Most implementations offer multiple memory types and selecting the correct one to allocate memory from is crucial
		// We also want the buffer to be host coherent so we don't have to flush (or sync after every update.
		// Note: This may affect performance so you might not want to do this in a real world application that updates buffers on a regular base
		allocInfo.memoryTypeIndex = vulkan_utils::getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_device->memoryProperties);
		// Allocate memory for the uniform buffer
		VK_CHECK(vkAllocateMemory(m_device->logicalDevice, &allocInfo, nullptr, &m_uniformBuffers[i].memory));
		// Bind memory to buffer
		VK_CHECK(vkBindBufferMemory(m_device->logicalDevice, m_uniformBuffers[i].buffer, m_uniformBuffers[i].memory, 0));
		// We map the buffer once, so we can update it without having to map it again
		VK_CHECK(vkMapMemory(m_device->logicalDevice, m_uniformBuffers[i].memory, 0, sizeof(ShaderData), 0, (void**)&m_uniformBuffers[i].mapped));
	}
	return true;
}

// Descriptor set layouts define the interface between our application and the shader
// Basically connects the different shader stages to descriptors for binding uniform buffers, image samplers, etc.
// So every shader binding should map to one descriptor set layout binding
bool VulkanRenderingContext::createDescriptorSetLayout()
{
	// Binding 0: Uniform buffer (Vertex shader)
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
	descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCI.pNext = nullptr;
	descriptorLayoutCI.bindingCount = 1;
	descriptorLayoutCI.pBindings = &layoutBinding;
	VK_CHECK(vkCreateDescriptorSetLayout(m_device->logicalDevice, &descriptorLayoutCI, nullptr, &m_descriptorSetLayout));
	return true;
}

bool VulkanRenderingContext::createDescriptorPool()
{
	// We need to tell the API the number of max. requested descriptors per type
	VkDescriptorPoolSize descriptorTypeCounts[1]{};
	// This example only one descriptor type (uniform buffer)
	descriptorTypeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	// We have one buffer (and as such descriptor) per frame
	descriptorTypeCounts[0].descriptorCount = MAX_CONCURRENT_FRAMES;
	// For additional types you need to add new entries in the type count list
	// E.g. for two combined image samplers :
	// typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	// typeCounts[1].descriptorCount = 2;

	// Create the global descriptor pool
	// All descriptors used in this example are allocated from this pool
	VkDescriptorPoolCreateInfo descriptorPoolCI{};
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.pNext = nullptr;
	descriptorPoolCI.poolSizeCount = 1;
	descriptorPoolCI.pPoolSizes = descriptorTypeCounts;
	// Set the max. number of descriptor sets that can be requested from this pool (requesting beyond this limit will result in an error)
	// Our sample will create one set per uniform buffer per frame
	descriptorPoolCI.maxSets = MAX_CONCURRENT_FRAMES;
	VK_CHECK(vkCreateDescriptorPool(m_device->logicalDevice, &descriptorPoolCI, nullptr, &m_descriptorPool));
	return true;
}

// Shaders access data using descriptor sets that "point" at our uniform buffers
// The descriptor sets make use of the descriptor set layouts created above 
bool VulkanRenderingContext::createDescriptorSets()
{
	// Allocate one descriptor set per frame from the global descriptor pool
	for (uint32_t i = 0; i < MAX_CONCURRENT_FRAMES; i++) {
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_descriptorSetLayout;
		VK_CHECK(vkAllocateDescriptorSets(m_device->logicalDevice, &allocInfo, &m_uniformBuffers[i].descriptorSet));

		// Update the descriptor set determining the shader binding points
		// For every binding point used in a shader there needs to be one
		// descriptor set matching that binding point
		VkWriteDescriptorSet writeDescriptorSet{};
		
		// The buffer's information is passed using a descriptor info structure
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i].buffer;
		bufferInfo.range = sizeof(ShaderData);

		// Binding 0 : Uniform buffer
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = m_uniformBuffers[i].descriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &bufferInfo;
		writeDescriptorSet.dstBinding = 0;
		vkUpdateDescriptorSets(m_device->logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
	}
	return true; // Descriptor sets created successfully
}

bool VulkanRenderingContext::createPipelines()
{
	// Create the pipeline layout that is used to generate the rendering pipelines that are based on this descriptor set layout
	// In a more complex scenario you would have different pipeline layouts for different descriptor set layouts that could be reused
	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.pNext = nullptr;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &m_descriptorSetLayout;
	VK_CHECK(vkCreatePipelineLayout(m_device->logicalDevice, &pipelineLayoutCI, nullptr, &m_pipelineLayout));

	// Create the graphics pipeline used in this example
	// Vulkan uses the concept of rendering pipelines to encapsulate fixed states, replacing OpenGL's complex state machine
	// A pipeline is then stored and hashed on the GPU making pipeline changes very fast
	// Note: There are still a few dynamic states that are not directly part of the pipeline (but the info that they are used is)

	VkGraphicsPipelineCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// The layout used for this pipeline (can be shared among multiple pipelines using the same layout)
	pipelineCI.layout = m_pipelineLayout;
	// Renderpass this pipeline is attached to
	pipelineCI.renderPass = m_renderPass;

	// Construct the different states making up the pipeline

	// Input assembly state describes how primitives are assembled
	// This pipeline will assemble vertex data as a triangle lists (though we only use one triangle)
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
	inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
	rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateCI.cullMode = VK_CULL_MODE_NONE; // NO CULLING - should be visible from both sides
	rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateCI.depthClampEnable = VK_FALSE;
	rasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCI.depthBiasEnable = VK_FALSE;
	rasterizationStateCI.lineWidth = 1.0f; // Thick lines for visibility

	// Color blend state describes how blend factors are calculated (if used)
	// We need one blend attachment state per color attachment (even if blending is not used)
	VkPipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.colorWriteMask = 0xf;
	blendAttachmentState.blendEnable = VK_FALSE;
	VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
	colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCI.attachmentCount = 1;
	colorBlendStateCI.pAttachments = &blendAttachmentState;

	// Viewport state sets the number of viewports and scissor used in this pipeline
	// Note: This is actually overridden by the dynamic states (see below)
	VkPipelineViewportStateCreateInfo viewportStateCI{};
	viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCI.viewportCount = 1;
	viewportStateCI.scissorCount = 1;

	// Enable dynamic states
	// Most states are baked into the pipeline, but there are still a few dynamic states that can be changed within a command buffer
	// To be able to change these we need do specify which dynamic states will be changed using this pipeline. Their actual states are set later on in the command buffer.
	// For this example we will set the viewport and scissor using dynamic states
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
	VkPipelineDynamicStateCreateInfo dynamicStateCI{};
	dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
	dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	// Depth and stencil state containing depth and stencil compare and test operations
	// TEMPORARILY DISABLE DEPTH TESTING to debug triangle visibility
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
	depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCI.depthTestEnable = VK_FALSE;  // DISABLED FOR DEBUGGING
	depthStencilStateCI.depthWriteEnable = VK_FALSE; // DISABLED FOR DEBUGGING
	depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateCI.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilStateCI.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilStateCI.stencilTestEnable = VK_FALSE;
	depthStencilStateCI.front = depthStencilStateCI.back;

	// Multi sampling state
	// This example does not make use of multi sampling (for anti-aliasing), the state must still be set and passed to the pipeline
	VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
	multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleStateCI.pSampleMask = nullptr;

	// Vertex input descriptions
	// Specifies the vertex input parameters for a pipeline

	// Vertex input binding
	// This example uses a single vertex input binding at binding point 0 (see vkCmdBindVertexBuffers)
	VkVertexInputBindingDescription vertexInputBinding{};
	vertexInputBinding.binding = 0;
	vertexInputBinding.stride = sizeof(Vertex);
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Input attribute bindings describe shader attribute locations and memory layouts
	std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributs{};
	// These match the following shader layout (see triangle.vert):
	//	layout (location = 0) in vec3 inPos;
	//	layout (location = 1) in vec3 inColor;
	// Attribute location 0: Position
	vertexInputAttributs[0].binding = 0;
	vertexInputAttributs[0].location = 0;
	// Position attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
	vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributs[0].offset = offsetof(Vertex, position);
	// Attribute location 1: Color
	vertexInputAttributs[1].binding = 0;
	vertexInputAttributs[1].location = 1;
	// Color attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
	vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributs[1].offset = offsetof(Vertex, color);

	// Vertex input state used for pipeline creation
	VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
	vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCI.vertexBindingDescriptionCount = 1;
	vertexInputStateCI.pVertexBindingDescriptions = &vertexInputBinding;
	vertexInputStateCI.vertexAttributeDescriptionCount = 2;
	vertexInputStateCI.pVertexAttributeDescriptions = vertexInputAttributs.data();

	// Shaders
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
		loadShader("triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		loadShader("triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)};

	// Set pipeline shader stage info
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	// Assign the pipeline states to the pipeline creation info structure
	pipelineCI.pVertexInputState = &vertexInputStateCI;
	pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
	pipelineCI.pRasterizationState = &rasterizationStateCI;
	pipelineCI.pColorBlendState = &colorBlendStateCI;
	pipelineCI.pMultisampleState = &multisampleStateCI;
	pipelineCI.pViewportState = &viewportStateCI;
	pipelineCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCI.pDynamicState = &dynamicStateCI;

	// Create rendering pipeline using the specified states
	VK_CHECK(vkCreateGraphicsPipelines(m_device->logicalDevice, m_pipelineCache, 1, &pipelineCI, nullptr, &m_pipeline));

	// Shader modules are no longer needed once the graphics pipeline has been created
	vkDestroyShaderModule(m_device->logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device->logicalDevice, shaderStages[1].module, nullptr);
	return true;
}