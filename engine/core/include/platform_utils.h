#pragma once
namespace platform {
    // Pointer for window implementation
    //==============================================
    template<typename T>
    struct WindowHandle
    {
        // memory managed elsewhere, this is just a pointer
        T* handle;
    };
    //==============================================
    
    // Graphics API types (abstracted from Vulkan)
    //============================================
    template<typename T>
    struct GraphicsInstance
    {
        // memory managed elsewhere, this is just a pointer
        T* instance;
    };

    template<typename T>
    struct GraphicsSurface
    {
        // memory managed elsewhere, this is just a pointer
        T* surface;
    };
    //============================================
}