#include "InputState.h"

namespace Framework
{
    void InputState::BeginFrame()
    {
        m_prevKeys = m_keys;
        m_mouseDeltaX = 0;
        m_mouseDeltaY = 0;
        m_mouseWheelDelta = 0;
    }

    void InputState::SetKey(uint32_t vk, bool down)
    {
        if (vk < m_keys.size())
            m_keys[vk] = down;
    }

    bool InputState::IsKeyDown(uint32_t vk) const
    {
        return vk < m_keys.size() && m_keys[vk];
    }

    bool InputState::WasKeyPressed(uint32_t vk) const
    {
        return vk < m_keys.size() && m_keys[vk] && !m_prevKeys[vk];
    }

    bool InputState::WasKeyReleased(uint32_t vk) const
    {
        return vk < m_keys.size() && !m_keys[vk] && m_prevKeys[vk];
    }

    void InputState::SetMouseButton(uint32_t button, bool down)
    {
        if (button < m_mouseButtons.size())
            m_mouseButtons[button] = down;
    }

    bool InputState::IsMouseButtonDown(uint32_t button) const
    {
        return button < m_mouseButtons.size() && m_mouseButtons[button];
    }

    void InputState::SetMousePosition(int x, int y)
    {
        m_mouseDeltaX += x - m_mouseX;
        m_mouseDeltaY += y - m_mouseY;
        m_mouseX = x;
        m_mouseY = y;
    }

    void InputState::AddMouseWheelDelta(int delta)
    {
        m_mouseWheelDelta += delta;
    }
}
