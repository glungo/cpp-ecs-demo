#pragma once
#include <span>
#include "sprite.h"
#include <vector>
#include <algorithm>

namespace engine::graphics {
	
	class SpriteManager {
    public:
        //Last element reserved for INVALID_HANDLE (UINT32_MAX)
        SpriteManager() : m_freeList(UINT32_MAX - 1), 
                          m_spriteDescriptions(UINT32_MAX - 1),
                          m_spriteActiveFlags(UINT32_MAX - 1, false),
			              m_spriteVersions(UINT32_MAX - 1, 0)
        {
			uint32_t n = 0;
			std::generate(m_freeList.begin(), m_freeList.end(), [&]() { return n++; });
        }

        SpriteHandle createSprite(const SpriteDesc& desc);
        void         destroySprite(SpriteHandle h);
        bool         setSprite(SpriteHandle h, const SpriteDesc& desc);
        bool         applyPatch(const SpritePatch& p);
        void         applyPatches(std::span<const SpritePatch> patches);
        bool         getSprite(SpriteHandle h, SpriteDesc& out) const;

        // Frame build: produce a contiguous array of GPUSpritePacked in an internal scratch buffer
        // Returned view is valid until next build call.
        struct BuildResult {
            const GPUSpritePacked* data;
            uint32_t count;
        };
        BuildResult buildFrameBatch();

        size_t liveCount() const;
    private:
        std::vector<SpriteDesc> m_spriteDescriptions;
		std::vector<bool>       m_spriteActiveFlags;
		std::vector<uint32_t>   m_spriteVersions;

		std::vector<uint32_t>   m_freeList; // Free list of sprite indices
		std::vector<uint32_t>   m_dirtiedSprites; // List of indices that need to be rebuilt
		std::vector<uint8_t>    m_dirtyMask; // Bitmask for dirty properties
	};
}  // namespace engine::graphics