#pragma once
#include <vulkan/vulkan.h>

namespace engine::graphics::vulkan_utils {
    // Vertex buffer and attributes
	struct vulkan_vertex_buffer {
		VkDeviceMemory memory{ VK_NULL_HANDLE }; // Handle to the device memory for this buffer
		VkBuffer buffer{ VK_NULL_HANDLE }; // Handle to the Vulkan buffer object that the memory is bound to
	};

	// Index buffer
	struct vulkan_index_buffer {
		VkDeviceMemory memory{ VK_NULL_HANDLE };
		VkBuffer buffer{ VK_NULL_HANDLE };
		uint32_t count{ 0 };
	};

	// Uniform buffer block object
	struct vulkan_uniform_buffer {
		VkDeviceMemory memory{ VK_NULL_HANDLE };
		VkBuffer buffer{ VK_NULL_HANDLE };
		// The descriptor set stores the resources bound to the binding points in a shader
		// It connects the binding points of the different shaders with the buffers and images used for those bindings
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
		// We keep a pointer to the mapped buffer, so we can easily update it's contents via a memcpy
		uint8_t* mapped{ nullptr };
	};
} // namespace engine::graphics::vulkan_utils