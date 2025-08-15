#pragma once

namespace engine::graphics {

#define ENABLE_GPU_SPRITES 1
#define ENABLE_BINDLESS 1
#define ENABLE_SPRITE_MULTIPASS 1
#define ENABLE_SPRITE_PROFILING 1

struct SpriteRenderingCapabilities {
    bool descriptorIndexing;
    bool variableDescriptorCount;
    bool partiallyBound;
    bool updateAfterBind;
    bool bindlessSupported;
    bool gpuSpritesSupported;
    unsigned int maxDescriptorSetSampledImages;
    unsigned int maxPerStageDescriptorSampledImages;
    unsigned int maxPushConstantsSize;
    bool useBindlessFallback;
    bool timestampQuerySupported;
    
    SpriteRenderingCapabilities();
};

extern SpriteRenderingCapabilities g_spriteCapabilities;

}