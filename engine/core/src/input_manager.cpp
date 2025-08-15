#include "input_manager.h"
#include <GLFW/glfw3.h>

namespace engine {

void InputManager::handleKey(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) m_keyStates[key] = true;
    else if (action == GLFW_RELEASE) m_keyStates[key] = false;

    if (m_keyCallback) {
        KeyAction ka = (action == GLFW_PRESS) ? KeyAction::Press : (action == GLFW_RELEASE ? KeyAction::Release : KeyAction::Repeat);
        m_keyCallback(KeyEvent{key, scancode, ka, mods});
    }
}

void InputManager::handleMouseButton(int button, int action, int mods) {
    if (action == GLFW_PRESS) m_mouseButtonStates[button] = true; 
    else if (action == GLFW_RELEASE) m_mouseButtonStates[button] = false;
    if (m_mouseButtonCallback) {
        MouseButtonAction mba = (action == GLFW_PRESS) ? MouseButtonAction::Press : MouseButtonAction::Release;
        m_mouseButtonCallback(MouseButtonEvent{button, mba, mods});
    }
}

void InputManager::handleCursorPos(double x, double y) {
    m_prevCursorPos = m_cursorPos;
    m_cursorPos = {static_cast<float>(x), static_cast<float>(y)};
    m_cursorDelta = m_cursorPos - m_prevCursorPos;
    if (m_cursorPosCallback) m_cursorPosCallback(x,y);
}

void InputManager::handleScroll(double xoff, double yoff) {
    m_scrollAccum.x += static_cast<float>(xoff);
    m_scrollAccum.y += static_cast<float>(yoff);
    if (m_scrollCallback) m_scrollCallback(ScrollEvent{xoff,yoff});
}

void InputManager::endFrame() {
    m_scrollAccum = {0.0f, 0.0f};
    m_cursorDelta = {0.0f, 0.0f};
}

} // namespace engine
