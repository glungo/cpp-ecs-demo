#include "config_flags.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

namespace engine::graphics {

// Constructor implementation
SpriteRenderingCapabilities::SpriteRenderingCapabilities() :
    descriptorIndexing(false),
    variableDescriptorCount(false),
    partiallyBound(false),
    updateAfterBind(false),
    bindlessSupported(false),
    gpuSpritesSupported(false),
    maxDescriptorSetSampledImages(0),
    maxPerStageDescriptorSampledImages(0),
    maxPushConstantsSize(0),
    useBindlessFallback(false),
    timestampQuerySupported(false)
{
}

// Global capability state
SpriteRenderingCapabilities g_spriteCapabilities;

namespace vulkan_utils {

void detectSpriteRenderingCapabilities(VkPhysicalDevice physicalDevice) {
    // Reset capabilities
    g_spriteCapabilities = SpriteRenderingCapabilities();
    
    // Query descriptor indexing features
    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
    descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    
    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &descriptorIndexingFeatures;
    
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
    
    // Store descriptor indexing capabilities
    g_spriteCapabilities.descriptorIndexing = descriptorIndexingFeatures.descriptorBindingPartiallyBound && 
                                              descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending;
    g_spriteCapabilities.variableDescriptorCount = descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount;
    g_spriteCapabilities.partiallyBound = descriptorIndexingFeatures.descriptorBindingPartiallyBound;
    g_spriteCapabilities.updateAfterBind = descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending;
    
    // Query device properties for limits
    VkPhysicalDeviceDescriptorIndexingProperties descriptorIndexingProperties{};
    descriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
    
    VkPhysicalDeviceProperties2 properties2{};
    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    properties2.pNext = &descriptorIndexingProperties;
    
    vkGetPhysicalDeviceProperties2(physicalDevice, &properties2);
    
    // Store limits
    g_spriteCapabilities.maxDescriptorSetSampledImages = descriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSampledImages;
    g_spriteCapabilities.maxPerStageDescriptorSampledImages = descriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindSampledImages;
    g_spriteCapabilities.maxPushConstantsSize = properties2.properties.limits.maxPushConstantsSize;
    
    // Check for timestamp query support
    unsigned int queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    if (queueFamilyCount > 0) {
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.timestampValidBits > 0) {
                g_spriteCapabilities.timestampQuerySupported = true;
                break;
            }
        }
    }
    
    // Determine derived capabilities
    g_spriteCapabilities.bindlessSupported = g_spriteCapabilities.descriptorIndexing && 
                                              g_spriteCapabilities.variableDescriptorCount &&
                                              g_spriteCapabilities.maxDescriptorSetSampledImages >= 1024;
    
    g_spriteCapabilities.gpuSpritesSupported = g_spriteCapabilities.bindlessSupported;
    g_spriteCapabilities.useBindlessFallback = !g_spriteCapabilities.bindlessSupported;
    
    // Log capability detection results
    std::cout << "Sprite Rendering Capability Detection Results:\n";
    std::cout << "  Descriptor Indexing: " << (g_spriteCapabilities.descriptorIndexing ? "YES" : "NO") << "\n";
    std::cout << "  Variable Descriptor Count: " << (g_spriteCapabilities.variableDescriptorCount ? "YES" : "NO") << "\n";
    std::cout << "  Partially Bound: " << (g_spriteCapabilities.partiallyBound ? "YES" : "NO") << "\n";
    std::cout << "  Update After Bind: " << (g_spriteCapabilities.updateAfterBind ? "YES" : "NO") << "\n";
    std::cout << "  Bindless Supported: " << (g_spriteCapabilities.bindlessSupported ? "YES" : "NO") << "\n";
    std::cout << "  GPU Sprites Supported: " << (g_spriteCapabilities.gpuSpritesSupported ? "YES" : "NO") << "\n";
    std::cout << "  Max Descriptor Set Sampled Images: " << g_spriteCapabilities.maxDescriptorSetSampledImages << "\n";
    std::cout << "  Max Per-Stage Descriptor Sampled Images: " << g_spriteCapabilities.maxPerStageDescriptorSampledImages << "\n";
    std::cout << "  Max Push Constants Size: " << g_spriteCapabilities.maxPushConstantsSize << " bytes\n";
    std::cout << "  Timestamp Query Supported: " << (g_spriteCapabilities.timestampQuerySupported ? "YES" : "NO") << "\n";
    std::cout << "  Using Fallback Path: " << (g_spriteCapabilities.useBindlessFallback ? "YES" : "NO") << "\n";
}

std::vector<const char*> getSpriteRenderingRequiredExtensions() {
    std::vector<const char*> extensions;
    
    if (g_spriteCapabilities.descriptorIndexing) {
        extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    }
    
    return extensions;
}

void getSpriteRenderingRequiredFeatures(VkPhysicalDeviceFeatures& features, void** pNext) {
    features.samplerAnisotropy = VK_TRUE;
    features.fillModeNonSolid = VK_TRUE;
    
    if (g_spriteCapabilities.bindlessSupported) {
        static VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
        descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
        descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
        
        descriptorIndexingFeatures.pNext = *pNext;
        *pNext = &descriptorIndexingFeatures;
    }
}

} // namespace vulkan_utils

} // namespace engine::graphics