#include "sprite_manager.h"

SpriteHandle engine::graphics::SpriteManager::createSprite(const SpriteDesc& desc)
{
	assert(!m_freeList.empty() && "No free sprite handles available");

	uint32_t index = m_freeList.back();
	m_freeList.pop_back();


	SpriteHandle handle{ index, m_spriteVersions[index]++ };
	m_spriteActiveFlags[index] = true;
	m_spriteDescriptions[index] = desc;
	
	m_dirtiedSprites.push_back(index);
	m_dirtyMask.push_back(0xFF); // mark all properties as dirty initially

	return handle;
}

void engine::graphics::SpriteManager::destroySprite(SpriteHandle h)
{
	assert(h.index < m_spriteActiveFlags.size() && "Invalid sprite handle");
	m_spriteActiveFlags[h.index] = false;
	m_freeList.push_back(h.index);

	// if we removed a sprite and has not been committed yet, we can remove it from the dirty list
	auto it = std::remove(m_dirtiedSprites.begin(), m_dirtiedSprites.end(), h.index);
	if (it != m_dirtiedSprites.end()) {
		m_dirtiedSprites.erase(it, m_dirtiedSprites.end());
		// Reset the dirty mask for this sprite
		m_dirtyMask[h.index] = 0;
	}
}

bool engine::graphics::SpriteManager::setSprite(SpriteHandle h, const SpriteDesc& desc)
{
	assert(h.index < m_spriteActiveFlags.size() && "Invalid sprite handle");
	if (!m_spriteActiveFlags[h.index]) return false;

	m_spriteDescriptions[h.index] = desc;
	return true;
}

bool engine::graphics::SpriteManager::applyPatch(const SpritePatch& p)
{
	assert(p.index < m_spriteActiveFlags.size() && "Invalid sprite patch");
	if (!m_spriteActiveFlags[p.index]) return false;

	//apply the patch to the sprite description
	switch (p.kind) {
		case SpritePatch::Kind::Position:
			m_spriteDescriptions[p.handle.id].posX = p.data.vec2v.x;
			m_spriteDescriptions[p.handle.id].posY = p.data.vec2v.y;
			break;
		case SpritePatch::Kind::Scale:
			m_spriteDescriptions[p.handle.id].scaleX = p.data.vec2v.x;
			m_spriteDescriptions[p.handle.id].scaleY = p.data.vec2v.y;
			break;
		case SpritePatch::Kind::Rotation:
			m_spriteDescriptions[p.handle.id].rotation = p.data.f32v;
			break;
		case SpritePatch::Kind::Depth:
			m_spriteDescriptions[p.handle.id].depth = p.data.f32v;
			break;
		case SpritePatch::Kind::UV:
			m_spriteDescriptions[p.handle.id].uvMinX = p.data.uv.uvMin.x;
			m_spriteDescriptions[p.handle.id].uvMinY = p.data.uv.uvMin.y;
			m_spriteDescriptions[p.handle.id].uvMaxX = p.data.uv.uvMax.x;
			m_spriteDescriptions[p.handle.id].uvMaxY = p.data.uv.uvMax.y;
			break;
		case SpritePatch::Kind::TextureIndex:
			m_spriteDescriptions[p.handle.id].textureIndex = p.data.u32v;
			break;
		case SpritePatch::Kind::Color:
			m_spriteDescriptions[p.handle.id].colorRGBA = p.data.u32v;
			break;
		default:
			return false; // Unsupported patch kind
	}
	// maybe the sprite was already dirty, update the mask with the new changes
	if (m_dirtiedSprites[p.handle.id] == 0) {
		m_dirtiedSprites.push_back(p.handle.id);
		m_dirtyMask[p.handle.id] = 0; // Reset the dirty mask for this sprite
	}
	m_dirtyMask[p.handle.id] |= p.kind; // Set the dirty mask for this sprite
	return true;
}

void engine::graphics::SpriteManager::applyPatches(std::span<const SpritePatch> patches)
{
	for (const auto& patch : patches) {
		applyPatch(patch);
	}
}

bool engine::graphics::SpriteManager::getSprite(SpriteHandle h, SpriteDesc& out) const
{
	assert(h.index < m_spriteActiveFlags.size() && "Invalid sprite handle");
	if (!m_spriteActiveFlags[h.index]) return false;

	out = m_spriteDescriptions[h.index];
	return true;
}

BuildResult engine::graphics::SpriteManager::buildFrameBatch()
{
	//gather the sprites that were marked as dirty
	BuildResult result;
	//TODO: for now 1024 sounds a reasonable size for a temporary buffer
	std::array<GPUSpritePacked, 1024> gpuSprites; // Temporary buffer for GPU sprites
	//build the result data out of the dirty sprites
	for (auto index : m_dirtiedSprites) {
		gpuSprites.push_back(GPUSpritePacked{
			.position = m_spriteDescriptions[index].position,
			.scale = m_spriteDescriptions[index].scale,
			.rotation = m_spriteDescriptions[index].rotation,
			.depth = m_spriteDescriptions[index].depth,
			.uv = m_spriteDescriptions[index].uv,
			.textureIndex = m_spriteDescriptions[index].textureIndex,
			.color = m_spriteDescriptions[index].color,
			.pivot = m_spriteDescriptions[index].pivot,
			.flags = m_spriteDescriptions[index].flags,
			.userData = m_spriteDescriptions[index].userData
		});
	}
	result.data = gpuSprites.data();
	result.count = static_cast<uint32_t>(m_dirtiedSprites.size());

	//clear the dirty list after building
	m_dirtiedSprites.clear();
	m_dirtyMask.clear();
	return result;
}

size_t engine::graphics::SpriteManager::liveCount() const
{
	return std::count(m_spriteActiveFlags.begin(), m_spriteActiveFlags.end(), true);
}
