#pragma once
#include <cstdint>
#include <unordered_map>
#include <functional>
#include <glm/vec2.hpp>

namespace engine {
    enum class KeyAction : uint8_t { Press, Release, Repeat };
    enum class MouseButtonAction : uint8_t { Press, Release };

    struct KeyEvent { int key; int scancode; KeyAction action; int mods; };
    struct MouseButtonEvent { int button; MouseButtonAction action; int mods; };
    struct ScrollEvent { double xoffset; double yoffset; };

    class InputManager {
    public:
        using KeyCallback = std::function<void(const KeyEvent&)>;
        using MouseButtonCallback = std::function<void(const MouseButtonEvent&)>;
        using CursorPosCallback = std::function<void(double,double)>;
        using ScrollCallback = std::function<void(const ScrollEvent&)>;

        void setKeyCallback(KeyCallback cb) { m_keyCallback = std::move(cb); }
        void setMouseButtonCallback(MouseButtonCallback cb) { m_mouseButtonCallback = std::move(cb); }
        void setCursorPosCallback(CursorPosCallback cb) { m_cursorPosCallback = std::move(cb); }
        void setScrollCallback(ScrollCallback cb) { m_scrollCallback = std::move(cb); }

        bool isKeyDown(int key) const { auto it = m_keyStates.find(key); return it != m_keyStates.end() && it->second; }
        bool isMouseButtonDown(int button) const { auto it = m_mouseButtonStates.find(button); return it != m_mouseButtonStates.end() && it->second; }
        glm::vec2 cursorPosition() const { return m_cursorPos; }
        glm::vec2 cursorDelta() const { return m_cursorDelta; }
        glm::vec2 scrollDelta() const { return m_scrollAccum; }

        void handleKey(int key, int scancode, int action, int mods);
        void handleMouseButton(int button, int action, int mods);
        void handleCursorPos(double x, double y);
        void handleScroll(double xoff, double yoff);
        void endFrame();

    private:
        std::unordered_map<int,bool> m_keyStates;
        std::unordered_map<int,bool> m_mouseButtonStates;
        glm::vec2 m_cursorPos{0.0f};
        glm::vec2 m_prevCursorPos{0.0f};
        glm::vec2 m_cursorDelta{0.0f};
        glm::vec2 m_scrollAccum{0.0f};
        KeyCallback m_keyCallback; MouseButtonCallback m_mouseButtonCallback; CursorPosCallback m_cursorPosCallback; ScrollCallback m_scrollCallback;
    };
}
