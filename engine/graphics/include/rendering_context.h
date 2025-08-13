#pragma once

#include <cstdint>

namespace engine {
    class Window;
    class Camera;
}

namespace engine::graphics {
    
    // Abstract rendering context interface
    class RenderingContext {
    public:
        virtual ~RenderingContext() = default;
        
        // Core lifecycle
		virtual bool initialize() = 0;
        virtual void shutdown() = 0;
        
        // Frame operations
        virtual bool beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void render(const Camera& camera) = 0;
        
        // Surface management
        virtual void recreateSurface(uint32_t width, uint32_t height) = 0;
        virtual void getDrawableSize(uint32_t& width, uint32_t& height) const = 0;
        
        // Properties
        virtual uint32_t getCurrentFrameIndex() const = 0;
        virtual uint32_t getMaxFramesInFlight() const = 0;
    };
    
} // namespace engine::graphics