#include "vulkan_rendering_context.h"
#include <vulkan/vulkan.h>
#include "vulkan_device.h"
#include "window.h"
#include <set>
#include <vector>
#include <array>

using namespace engine::graphics;

VulkanRenderingContext::VulkanRenderingContext(const glfw_window& window) :
	m_window(window),
	m_instance(VK_NULL_HANDLE),
	m_surface(VK_NULL_HANDLE),
	m_depthStencil(),
	m_framebuffers(),
	m_shaderModules(),
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
	return true; // Currently not implemented
	//TODO: Setup IMGUI
	/*ui.device = vulkanDevice;
	ui.queue = queue;
	ui.shaders = {
		loadShader(getShadersPath() + "base/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		loadShader(getShadersPath() + "base/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
	};
	ui.prepareResources();
	ui.preparePipeline(pipelineCache, renderPass, swapChain.colorFormat, depthFormat);*/
}

VkPipelineShaderStageCreateInfo VulkanRenderingContext::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = vulkan_utils::loadShader(fileName.c_str(), m_device->logicalDevice);
	shaderStage.pName = "main";
	assert(shaderStage.module != VK_NULL_HANDLE);
	m_shaderModules.push_back(shaderStage.module);
	return shaderStage;
}