## GPU-Driven Sprite Rendering with Bindless Textures - Single Draw Architecture

Excellent choice! Let's design a modern GPU-driven sprite renderer that aims for a single draw call using bindless textures and GPU-based culling/sorting.

## Overall Architecture

### **Core Concept**
```cpp
// Everything happens on GPU:
// 1. All sprite data lives in GPU buffers
// 2. Culling/sorting via compute shaders
// 3. Single indirect draw call
// 4. Bindless textures for unlimited unique sprites

class GPUDrivenSpriteRenderer {
    // Sprite data
    VkBuffer spriteDataBuffer;        // All sprites (persistent)
    VkBuffer visibleSpritesBuffer;    // Post-culling indices
    
    // Draw commands
    VkBuffer drawIndirectBuffer;      // Single DrawIndexedIndirect command
    VkBuffer dispatchIndirectBuffer;  // For compute dispatches
    
    // Bindless resources
    VkDescriptorSet bindlessSet;      // Contains all texture descriptors
    std::vector<uint32_t> textureIndices; // Maps sprite -> texture
    
    // Pipelines
    VkPipeline cullPipeline;
    VkPipeline sortPipeline;
    VkPipeline renderPipeline;
};
```

## 1. Bindless Texture System

### **Bindless Descriptor Setup**

```cpp
class BindlessTextureManager {
    static constexpr uint32_t MAX_TEXTURES = 16384;  // Adjust based on needs
    
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout bindlessLayout;
    VkDescriptorSet bindlessSet;
    
    struct TextureHandle {
        uint32_t index;
        VkImageView view;
        VkSampler sampler;
    };
    
    std::vector<TextureHandle> textures;
    std::queue<uint32_t> freeIndices;
    
    void initialize(VkDevice device) {
        // Create descriptor set layout with large array
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = MAX_TEXTURES;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        // Enable partially bound descriptors
        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
        bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        std::vector<VkDescriptorBindingFlags> flags(1, 
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT);
        bindingFlags.bindingCount = 1;
        bindingFlags.pBindingFlags = flags.data();
        
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;
        layoutInfo.pNext = &bindingFlags;
        layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        
        vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &bindlessLayout);
        
        // Create descriptor pool with UPDATE_AFTER_BIND
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = MAX_TEXTURES;
        
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        
        vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
        
        // Allocate the single bindless set
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &bindlessLayout;
        
        vkAllocateDescriptorSets(device, &allocInfo, &bindlessSet);
    }
    
    uint32_t addTexture(VkImageView view, VkSampler sampler) {
        uint32_t index;
        
        if (!freeIndices.empty()) {
            index = freeIndices.front();
            freeIndices.pop();
        } else {
            index = textures.size();
            textures.emplace_back();
        }
        
        textures[index] = {index, view, sampler};
        
        // Update descriptor
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = view;
        imageInfo.sampler = sampler;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = bindlessSet;
        write.dstBinding = 0;
        write.dstArrayElement = index;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &imageInfo;
        
        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        
        return index;
    }
};
```

## 2. GPU Sprite Data Structure

### **Sprite Data Layout**

```cpp
// Aligned to 16 bytes for efficient GPU access
struct GPUSprite {
    // Transform (16 bytes)
    float posX, posY;           // World position
    float scaleX, scaleY;       // Scale
    
    // Texture coordinates (16 bytes)
    float uvMinX, uvMinY;       // UV min
    float uvMaxX, uvMaxY;       // UV max
    
    // Appearance (16 bytes)
    float rotation;             // Rotation in radians
    float depth;                // Z-order for sorting
    uint32_t textureIndex;      // Bindless texture index
    uint32_t packedColor;       // RGBA8 packed into uint32
    
    // Optional extras (16 bytes)
    float pivotX, pivotY;       // Rotation pivot
    uint32_t flags;             // Flip H/V, blend mode, etc.
    uint32_t userData;          // Application-specific data
};

// SSBO for all sprites
layout(set = 0, binding = 0) readonly buffer SpriteBuffer {
    GPUSprite sprites[];
} spriteBuffer;

// SSBO for visible sprite indices after culling
layout(set = 0, binding = 1) buffer VisibleBuffer {
    uint count;
    uint indices[];
} visibleBuffer;
```

### **Sprite Management**

```cpp
class GPUSpriteManager {
    VkBuffer spriteBuffer;
    VkDeviceMemory spriteMemory;
    GPUSprite* mappedSprites;
    
    uint32_t maxSprites;
    uint32_t activeSprites;
    
    // Free list for sprite allocation
    std::vector<uint32_t> freeList;
    
    void initialize(VkDevice device, uint32_t maxSprites) {
        this->maxSprites = maxSprites;
        
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(GPUSprite) * maxSprites;
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        vkCreateBuffer(device, &bufferInfo, nullptr, &spriteBuffer);
        
        // Allocate with HOST_VISIBLE for CPU updates
        // Or use device-local with staging for better performance
        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device, spriteBuffer, &memReqs);
        
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        vkAllocateMemory(device, &allocInfo, nullptr, &spriteMemory);
        vkBindBufferMemory(device, spriteBuffer, spriteMemory, 0);
        
        // Persistent mapping
        vkMapMemory(device, spriteMemory, 0, bufferInfo.size, 0, 
                   (void**)&mappedSprites);
    }
    
    uint32_t allocateSprite() {
        uint32_t index;
        if (!freeList.empty()) {
            index = freeList.back();
            freeList.pop_back();
        } else {
            index = activeSprites++;
        }
        return index;
    }
    
    void updateSprite(uint32_t index, const SpriteData& data) {
        GPUSprite& sprite = mappedSprites[index];
        sprite.posX = data.position.x;
        sprite.posY = data.position.y;
        sprite.scaleX = data.scale.x;
        sprite.scaleY = data.scale.y;
        sprite.rotation = data.rotation;
        sprite.depth = data.depth;
        sprite.textureIndex = data.textureHandle;
        sprite.packedColor = packColor(data.color);
        // ... etc
    }
};
```

## 3. GPU Culling & Sorting Pipeline

### **Culling Compute Shader**

```glsl
#version 450
#extension GL_ARB_gpu_shader_int64 : enable

layout(local_size_x = 64) in;

// Input sprites
layout(set = 0, binding = 0) readonly buffer SpriteBuffer {
    GPUSprite sprites[];
} spriteBuffer;

// Output visible indices
layout(set = 0, binding = 1) buffer VisibleBuffer {
    uint count;
    uint indices[];
} visibleBuffer;

// Indirect draw buffer
layout(set = 0, binding = 2) buffer DrawIndirectBuffer {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
} drawCmd;

// Uniforms for culling
layout(set = 0, binding = 3) uniform CullData {
    vec4 frustumPlanes[6];
    vec2 screenMin;
    vec2 screenMax;
    mat4 viewProj;
} cullData;

// Atomic counter for output position
layout(set = 0, binding = 4) buffer AtomicBuffer {
    uint visibleCount;
} atomicBuffer;

bool isVisible(GPUSprite sprite) {
    // Transform sprite corners to screen space
    vec2 corners[4] = {
        vec2(-0.5, -0.5),
        vec2( 0.5, -0.5),
        vec2( 0.5,  0.5),
        vec2(-0.5,  0.5)
    };
    
    // Apply rotation and scale
    float c = cos(sprite.rotation);
    float s = sin(sprite.rotation);
    mat2 rot = mat2(c, -s, s, c);
    
    vec2 minScreen = vec2(1e9);
    vec2 maxScreen = vec2(-1e9);
    
    for (int i = 0; i < 4; i++) {
        vec2 corner = rot * (corners[i] * vec2(sprite.scaleX, sprite.scaleY));
        corner += vec2(sprite.posX, sprite.posY);
        
        vec4 clipPos = cullData.viewProj * vec4(corner, sprite.depth, 1.0);
        vec2 screenPos = clipPos.xy / clipPos.w;
        
        minScreen = min(minScreen, screenPos);
        maxScreen = max(maxScreen, screenPos);
    }
    
    // Check screen bounds
    return !(maxScreen.x < cullData.screenMin.x || 
             minScreen.x > cullData.screenMax.x ||
             maxScreen.y < cullData.screenMin.y || 
             minScreen.y > cullData.screenMax.y);
}

void main() {
    uint spriteIndex = gl_GlobalInvocationID.x;
    
    if (spriteIndex >= spriteBuffer.sprites.length()) {
        return;
    }
    
    GPUSprite sprite = spriteBuffer.sprites[spriteIndex];
    
    if (isVisible(sprite)) {
        uint outputIndex = atomicAdd(atomicBuffer.visibleCount, 1);
        visibleBuffer.indices[outputIndex] = spriteIndex;
    }
    
    // Update draw command on last thread
    if (spriteIndex == 0) {
        barrier();
        drawCmd.instanceCount = atomicBuffer.visibleCount;
        drawCmd.indexCount = 6;  // Quad indices
        drawCmd.firstIndex = 0;
        drawCmd.vertexOffset = 0;
        drawCmd.firstInstance = 0;
    }
}
```

### **Sorting Compute Shader (Bitonic Sort)**

```glsl
#version 450

layout(local_size_x = 256) in;

layout(set = 0, binding = 0) readonly buffer SpriteBuffer {
    GPUSprite sprites[];
} spriteBuffer;

layout(set = 0, binding = 1) buffer VisibleBuffer {
    uint count;
    uint indices[];
} visibleBuffer;

layout(push_constant) uniform PushConstants {
    uint stage;
    uint passNum;
    uint numElements;
} pc;

void bitonicSort() {
    uint tid = gl_GlobalInvocationID.x;
    
    uint pairDistance = 1u << (pc.stage - pc.passNum);
    uint blockWidth = 2u * pairDistance;
    uint leftId = (tid % pairDistance) + (tid / pairDistance) * blockWidth;
    uint rightId = leftId + pairDistance;
    
    if (rightId >= pc.numElements) return;
    
    uint leftIndex = visibleBuffer.indices[leftId];
    uint rightIndex = visibleBuffer.indices[rightId];
    
    float leftDepth = spriteBuffer.sprites[leftIndex].depth;
    float rightDepth = spriteBuffer.sprites[rightIndex].depth;
    
    bool compareResult = leftDepth > rightDepth;
    
    uint sameDirectionBlockWidth = 1u << pc.stage;
    bool sameDirection = (tid / (sameDirectionBlockWidth / 2)) % 2 == 0;
    
    if (sameDirection == compareResult) {
        // Swap
        visibleBuffer.indices[leftId] = rightIndex;
        visibleBuffer.indices[rightId] = leftIndex;
    }
}

void main() {
    bitonicSort();
}
```

## 4. Rendering Pipeline

### **Vertex Shader**

```glsl
#version 450
#extension GL_EXT_nonuniform_qualifier : enable

// Vertex data for a quad (could be hardcoded)
const vec2 quadVertices[6] = {
    vec2(-0.5, -0.5),  // Triangle 1
    vec2( 0.5, -0.5),
    vec2( 0.5,  0.5),
    
    vec2( 0.5,  0.5),  // Triangle 2
    vec2(-0.5,  0.5),
    vec2(-0.5, -0.5)
};

const vec2 quadUVs[6] = {
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0)
};

// Sprite data
layout(set = 0, binding = 0) readonly buffer SpriteBuffer {
    GPUSprite sprites[];
} spriteBuffer;

// Visible sprite indices
layout(set = 0, binding = 1) readonly buffer VisibleBuffer {
    uint count;
    uint indices[];
} visibleBuffer;

// Uniforms
layout(set = 0, binding = 2) uniform UniformBuffer {
    mat4 viewProj;
} ubo;

// Outputs
layout(location = 0) out vec2 fragUV;
layout(location = 1) out flat uint fragTextureIndex;
layout(location = 2) out vec4 fragColor;

vec4 unpackColor(uint packed) {
    return vec4(
        float((packed >> 0) & 0xFF) / 255.0,
        float((packed >> 8) & 0xFF) / 255.0,
        float((packed >> 16) & 0xFF) / 255.0,
        float((packed >> 24) & 0xFF) / 255.0
    );
}

void main() {
    // Get sprite index from instance
    uint spriteIdx = visibleBuffer.indices[gl_InstanceIndex];
    GPUSprite sprite = spriteBuffer.sprites[spriteIdx];
    
    // Get vertex position
    vec2 vertexPos = quadVertices[gl_VertexIndex];
    vec2 vertexUV = quadUVs[gl_VertexIndex];
    
    // Apply sprite transform
    float c = cos(sprite.rotation);
    float s = sin(sprite.rotation);
    mat2 rot = mat2(c, -s, s, c);
    
    vec2 scaledPos = vertexPos * vec2(sprite.scaleX, sprite.scaleY);
    vec2 rotatedPos = rot * scaledPos;
    vec2 worldPos = rotatedPos + vec2(sprite.posX, sprite.posY);
    
    gl_Position = ubo.viewProj * vec4(worldPos, sprite.depth, 1.0);
    
    // Interpolate UVs
    fragUV = mix(
        vec2(sprite.uvMinX, sprite.uvMinY),
        vec2(sprite.uvMaxX, sprite.uvMaxY),
        vertexUV
    );
    
    fragTextureIndex = sprite.textureIndex;
    fragColor = unpackColor(sprite.packedColor);
}
```

### **Fragment Shader**

```glsl
#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 fragUV;
layout(location = 1) in flat uint fragTextureIndex;
layout(location = 2) in vec4 fragColor;

// Bindless texture array
layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(location = 0) out vec4 outColor;

void main() {
    // Sample texture using bindless index
    vec4 texColor = texture(textures[nonuniformEXT(fragTextureIndex)], fragUV);
    
    // Apply tint color
    outColor = texColor * fragColor;
    
    // Alpha test for better performance (optional)
    if (outColor.a < 0.01) {
        discard;
    }
}
```

## 5. Main Render Loop

### **Single Draw Render System**

```cpp
class SingleDrawSpriteRenderer {
    void render(VkCommandBuffer cmd) {
        // 1. Reset atomic counter
        uint32_t zero = 0;
        vkCmdUpdateBuffer(cmd, atomicBuffer, 0, sizeof(uint32_t), &zero);
        
        // Memory barrier
        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | 
                               VK_ACCESS_SHADER_WRITE_BIT;
        
        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 1, &barrier, 0, nullptr, 0, nullptr);
        
        // 2. Dispatch culling
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, cullPipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                               cullPipelineLayout, 0, 1, &computeDescSet, 0, nullptr);
        
        uint32_t groupCount = (activeSpriteCount + 63) / 64;
        vkCmdDispatch(cmd, groupCount, 1, 1);
        
        // Barrier between culling and sorting
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | 
                               VK_ACCESS_SHADER_WRITE_BIT;
        
        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 1, &barrier, 0, nullptr, 0, nullptr);
        
        // 3. Dispatch sorting (multiple passes for bitonic sort)
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, sortPipeline);
        
        uint32_t numStages = (uint32_t)ceil(log2(maxSprites));
        for (uint32_t stage = 1; stage <= numStages; stage++) {
            for (uint32_t pass = stage; pass > 0; pass--) {
                SortPushConstants pushConstants{stage, pass, activeSpriteCount};
                vkCmdPushConstants(cmd, sortPipelineLayout,
                                  VK_SHADER_STAGE_COMPUTE_BIT,
                                  0, sizeof(pushConstants), &pushConstants);
                
                uint32_t sortGroups = (activeSpriteCount + 255) / 256;
                vkCmdDispatch(cmd, sortGroups, 1, 1);
                
                // Barrier between sort passes
                vkCmdPipelineBarrier(cmd,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    0, 1, &barrier, 0, nullptr, 0, nullptr);
            }
        }
        
        // 4. Barrier before rendering
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
                               VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        
        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | 
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            0, 1, &barrier, 0, nullptr, 0, nullptr);
        
        // 5. Single indirect draw call!
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPipeline);
        
        // Bind descriptor sets
        VkDescriptorSet sets[] = {renderDescSet, bindlessTextureSet};
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                               renderPipelineLayout, 0, 2, sets, 0, nullptr);
        
        // THE SINGLE DRAW CALL
        vkCmdDrawIndirect(cmd, drawIndirectBuffer, 0, 1, 0);
    }
};
```

## 6. Advanced Optimizations

### **Multi-Draw Indirect for Batching by State**

```cpp
// If you need different blend modes or render states
struct DrawGroup {
    uint32_t blendMode;
    uint32_t startIndex;
    uint32_t count;
};

// In compute shader, output multiple draw commands
layout(set = 0, binding = 3) buffer MultiDrawBuffer {
    VkDrawIndexedIndirectCommand commands[MAX_DRAW_GROUPS];
} multiDraw;

// Then in render:
vkCmdDrawIndexedIndirect(cmd, multiDrawBuffer, 0, 
                         drawGroupCount, sizeof(VkDrawIndexedIndirectCommand));
```

### **Mesh Shaders (if available)**

```glsl
#version 450
#extension GL_NV_mesh_shader : enable

layout(local_size_x = 32) in;

// Task shader to cull groups of sprites
taskNV out Task {
    uint spriteIndices[32];
} OUT;

void main() {
    // Cull 32 sprites at once
    uint baseIndex = gl_WorkGroupID.x * 32;
    uint visibleCount = 0;
    
    for (uint i = 0; i < 32; i++) {
        if (isVisible(sprites[baseIndex + i])) {
            OUT.spriteIndices[visibleCount++] = baseIndex + i;
        }
    }
    
    // Emit mesh shader workgroups
    gl_TaskCountNV = visibleCount;
}
```

### **Streaming Updates**

```cpp
class StreamingSpriteUpdater {
    // Triple buffering for updates while GPU is rendering
    static constexpr int BUFFER_COUNT = 3;
    VkBuffer updateBuffers[BUFFER_COUNT];
    int currentBuffer = 0;
    
    void updateSprites(const std::vector<SpriteUpdate>& updates) {
        // Copy updates to staging buffer
        currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;
        VkBuffer stagingBuffer = updateBuffers[currentBuffer];
        
        // Record copy commands
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = updates.size() * sizeof(GPUSprite);
        
        vkCmdCopyBuffer(cmd, stagingBuffer, spriteBuffer, 1, &copyRegion);
    }
};
```

## Performance Tips

1. **Persistent Threads**: Keep compute shader threads alive across multiple operations
2. **Wave Intrinsics**: Use subgroup operations for faster reductions
3. **Async Compute**: Run culling/sorting on async compute queue
4. **Temporal Coherence**: Cache previous frame's visibility for prediction
5. **LOD System**: Reduce vertex count for distant sprites
6. **Texture Compression**: Use BC7/ASTC for texture memory bandwidth
7. **Early-Z**: Sort front-to-back when possible for early depth rejection

This architecture achieves:
- **Single draw call** for all sprites (or minimal with multi-draw indirect)
- **No CPU culling** - everything on GPU
- **Unlimited unique textures** via bindless
- **Minimal state changes**
- **Scales to 100k+ sprites** efficiently

The key is keeping everything on the GPU and avoiding CPU-GPU synchronization points.