#pragma once
#include <cstdint>
#include <glm/vec2.hpp>

namespace engine::graphics {
    
    /// <summary>
	/// versioned identifier for a sprite instance.
    /// </summary>
    struct SpriteHandle {
        uint32_t id = 0;
        uint32_t version = 0;
    };
    constexpr SpriteHandle InvalidSpriteHandle{ -1, -1 };

    /// <summary>
    /// Describes the properties and rendering parameters of a 2D sprite.
	/// Packed to fit into 64 bytes for efficient GPU upload.
    /// </summary>
    struct SpriteDesc {
        float posX, posY;
        float scaleX, scaleY;
        float uvMinX, uvMinY;
        float uvMaxX, uvMaxY;
        float rotation;
        float depth;
        uint32_t textureIndex;
        uint32_t colorRGBA;
    };

    /// <summary>
    /// Represents a set of changes (delta) to a sprite's properties,
    /// </summary>
    //TODO: consider using std::variant for SpritePatch data to improve type safety.
    struct SpritePatch {
        enum class Kind : uint8_t {
            Position= 1 << 0,
            Scale= 1 << 1,
            Rotation= 1 << 2,
            Depth= 1 << 3,
            UV= 1 << 4,
            TextureIndex= 1 << 5,
            Color= 1 << 6,
            Pivot= 1 << 7,
            Flags= 1 << 8,
            UserData= 1 << 9
        } kind{ Kind::Position };
        SpriteHandle handle{};
        union {
            glm::vec2 vec2v;                   // Position, Scale, Pivot
            struct { glm::vec2 uvMin, uvMax; } uv; // UV
            uint32_t u32v;                     // TextureIndex, Color, Flags, UserData
            float f32v;                        // Rotation, Depth (used separately)
            struct { float rotation, depth; } rotDepthPair; // (Reserved if we later allow combined rot+depth)
        } data{};
    };
}

