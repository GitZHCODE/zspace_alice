#include "InputManager.h"
#ifdef _WIN32
    #include <freeglut.h>
#else
    #include <GL/freeglut.h>
#endif

namespace alice2 {

    InputManager::InputManager()
        : m_modifiers(0)
    {
        // Initialize mouse state
        m_mouseState.position = Vec3(0, 0, 0);
        m_mouseState.lastPosition = Vec3(0, 0, 0);
        m_mouseState.delta = Vec3(0, 0, 0);
        m_mouseState.wheelDelta = 0.0f;
        
        for (int i = 0; i < 3; i++) {
            m_mouseState.buttons[i] = false;
            m_mouseState.lastButtons[i] = false;
        }
    }

    void InputManager::update() {
        // Update last states
        m_lastKeyStates = m_keyStates;
        
        for (int i = 0; i < 3; i++) {
            m_mouseState.lastButtons[i] = m_mouseState.buttons[i];
        }
        
        // Update mouse delta
        updateMouseDelta();
        
        // Reset wheel delta
        m_mouseState.wheelDelta = 0.0f;
    }

    void InputManager::setKeyState(unsigned char key, KeyState state) {
        m_keyStates[key] = state;
    }

    bool InputManager::isKeyPressed(unsigned char key) const {
        auto it = m_keyStates.find(key);
        auto lastIt = m_lastKeyStates.find(key);
        
        bool currentPressed = (it != m_keyStates.end() && it->second == KeyState::Pressed);
        bool lastPressed = (lastIt != m_lastKeyStates.end() && lastIt->second == KeyState::Pressed);
        
        return currentPressed && !lastPressed;
    }

    bool InputManager::isKeyDown(unsigned char key) const {
        auto it = m_keyStates.find(key);
        return (it != m_keyStates.end() && it->second == KeyState::Pressed);
    }

    bool InputManager::isKeyReleased(unsigned char key) const {
        auto it = m_keyStates.find(key);
        auto lastIt = m_lastKeyStates.find(key);
        
        bool currentPressed = (it != m_keyStates.end() && it->second == KeyState::Pressed);
        bool lastPressed = (lastIt != m_lastKeyStates.end() && lastIt->second == KeyState::Pressed);
        
        return !currentPressed && lastPressed;
    }

    void InputManager::setMousePosition(int x, int y) {
        m_mouseState.lastPosition = m_mouseState.position;
        m_mouseState.position = Vec3(static_cast<float>(x), static_cast<float>(y), 0);
    }

    void InputManager::setMouseButton(MouseButton button, KeyState state) {
        int index = static_cast<int>(button);
        if (index >= 0 && index < 3) {
            m_mouseState.buttons[index] = (state == KeyState::Pressed);
        }
    }

    void InputManager::setMouseWheel(float delta) {
        m_mouseState.wheelDelta = delta;
    }

    bool InputManager::isMouseButtonPressed(MouseButton button) const {
        int index = static_cast<int>(button);
        if (index >= 0 && index < 3) {
            return m_mouseState.buttons[index] && !m_mouseState.lastButtons[index];
        }
        return false;
    }

    bool InputManager::isMouseButtonDown(MouseButton button) const {
        int index = static_cast<int>(button);
        if (index >= 0 && index < 3) {
            return m_mouseState.buttons[index];
        }
        return false;
    }

    bool InputManager::isMouseButtonReleased(MouseButton button) const {
        int index = static_cast<int>(button);
        if (index >= 0 && index < 3) {
            return !m_mouseState.buttons[index] && m_mouseState.lastButtons[index];
        }
        return false;
    }

    bool InputManager::isShiftPressed() const {
        return (m_modifiers & GLUT_ACTIVE_SHIFT) != 0;
    }

    bool InputManager::isCtrlPressed() const {
        return (m_modifiers & GLUT_ACTIVE_CTRL) != 0;
    }

    bool InputManager::isAltPressed() const {
        return (m_modifiers & GLUT_ACTIVE_ALT) != 0;
    }

    void InputManager::processKeyboard(unsigned char key, int x, int y) {
        m_modifiers = glutGetModifiers();
        setKeyState(key, KeyState::Pressed);
        
        if (m_keyCallback) {
            m_keyCallback(key, x, y);
        }
    }

    void InputManager::processMouseButton(int button, int state, int x, int y) {
        m_modifiers = glutGetModifiers();
        setMousePosition(x, y);
        
        MouseButton mb;
        switch (button) {
            case GLUT_LEFT_BUTTON: mb = MouseButton::Left; break;
            case GLUT_MIDDLE_BUTTON: mb = MouseButton::Middle; break;
            case GLUT_RIGHT_BUTTON: mb = MouseButton::Right; break;
            default: return;
        }
        
        KeyState ks = (state == GLUT_DOWN) ? KeyState::Pressed : KeyState::Released;
        setMouseButton(mb, ks);
        
        if (m_mouseButtonCallback) {
            m_mouseButtonCallback(mb, ks, x, y);
        }
    }

    void InputManager::processMouseMotion(int x, int y) {
        setMousePosition(x, y);
        
        if (m_mouseMoveCallback) {
            m_mouseMoveCallback(x, y);
        }
    }

    void InputManager::processMouseWheel(float delta) {
        setMouseWheel(delta);
        
        if (m_mouseWheelCallback) {
            m_mouseWheelCallback(delta);
        }
    }

    void InputManager::updateMouseDelta() {
        m_mouseState.delta = m_mouseState.position - m_mouseState.lastPosition;
    }

} // namespace alice2
