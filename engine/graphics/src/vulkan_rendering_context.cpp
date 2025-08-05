#include <vulkan/vulkan.h>
#include "vulkan_rendering_context.h"
#include "vulkan_device.h"
#include "window.h"
#include <set>
#include <vector>

using namespace engine::graphics;

VulkanRenderingContext::VulkanRenderingContext(const glfw_window& window) :
	m_window(window),
	m_instance(VK_NULL_HANDLE),
	m_surface(VK_NULL_HANDLE),
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
		vkDestroySemaphore(m_device->logicalDevice, m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_device->logicalDevice, m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_device->logicalDevice, m_inFlightFences[i], nullptr);
	}
	// Cleanup command buffers
	vkFreeCommandBuffers(m_device->logicalDevice, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
	vkDestroyCommandPool(m_device->logicalDevice, m_commandPool, nullptr);
	// Cleanup swapchain resources
	m_swapchain->cleanup();
	// Cleanup surface and instance
	if (m_surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}
	/*if (m_debugMessenger != VK_NULL_HANDLE) {
		vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	}*/
	vkDestroyInstance(m_instance, nullptr);
	m_initialized = false;
}

bool VulkanRenderingContext::beginFrame() {
	if (!m_initialized) {
		return false; // Not initialized
	}
	// Wait for the current frame to finish
	vkWaitForFences(m_device->logicalDevice, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
	// Reset the fence for the next frame
	vkResetFences(m_device->logicalDevice, 1, &m_inFlightFences[m_currentFrame]);
	// Acquire the next image from the swapchain
	VkResult result = m_swapchain->acquireNextImage(m_imageAvailableSemaphores[m_currentFrame], m_currentImageIndex);
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		return false; // Failed to acquire next image
	}
	return true; // Frame started successfully
}

void VulkanRenderingContext::endFrame() {

}

void VulkanRenderingContext::present() {

}

void VulkanRenderingContext::recreateSurface(uint32_t width, uint32_t height)
{

}

void VulkanRenderingContext::getDrawableSize(uint32_t& width, uint32_t& height) const
{
	/*if (m_swapchain) {
        m_swapchain->getDrawableSize(width, height);
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
	/*if (vkCreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
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
	// Create one command buffer for each swap chain image
	m_commandBuffers.resize(m_swapchain->images.size());
	VkCommandBufferAllocateInfo cmdBufAllocateInfo = vulkan_utils::initializers::commandBufferAllocateInfo(m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(m_commandBuffers.size()));
	return vkAllocateCommandBuffers(m_device->logicalDevice, &cmdBufAllocateInfo, m_commandBuffers.data()) == VK_SUCCESS;
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
			return false; // Failed to create synchronization objects
		}
	}
	return true; // Synchronization objects created successfully
}