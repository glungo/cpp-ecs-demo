#pragma once
#include <stdint.h>

namespace engine::graphics {
    // GPU sprite structure for rendering 2D sprites with advanced features
    // This structure is designed to be used in GPU buffers for efficient rendering.
    // Probably most things will be unused for quite a while but this is a good base to start with.
struct GPUSprite {
    // Basic transform (16 bytes)
    float posX, posY;
    float scaleX, scaleY;
#ifdef USE_SPRITESHEETS   
    // Texture coordinates (16 bytes)
    float uvMinX, uvMinY;
    float uvMaxX, uvMaxY;
#else
    // just an index to the texture atlas
    uint32_t textureIndex;           // Index into the texture atlas
#endif

#ifdef USE_MULTIPASS_RENDERING
    // Appearance (16 bytes)
    float rotation;
    float depth;
    uint32_t diffuseTextureIndex;   // Base color texture
    uint32_t packedColor;
    
    // Extended data for effects (16 bytes)
    uint32_t normalTextureIndex;     // Normal map for lighting
    uint32_t emissiveTextureIndex;   // Emissive map
    float emissiveIntensity;
    uint32_t renderFlags;            // Bit flags for render passes
    
    // Motion data (16 bytes)
    float prevPosX, prevPosY;        // Previous frame position
    float velocityX, velocityY;      // Velocity for motion blur
    
    // Material properties (16 bytes)
    float roughness;
    float metallic;
    float distortionStrength;        // For heat distortion, etc.
    uint32_t materialID;             // Material type for shading
#endif
};

// Render flags
enum RenderFlags : uint32_t {
    RENDER_OPAQUE       = 1 << 0,
    RENDER_TRANSPARENT  = 1 << 1,
    RENDER_EMISSIVE     = 1 << 2,
    RENDER_DISTORTION   = 1 << 3,
    RENDER_CAST_SHADOW  = 1 << 4,
    RENDER_RECEIVE_LIGHT = 1 << 5,
    RENDER_MOTION_BLUR  = 1 << 6,
};
}