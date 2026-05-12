#pragma once

#include <Windows.h>
#include <array>
#include <cstdint>

namespace Framework
{
    class InputState
    {
    public:
        void BeginFrame();

        void SetKey(uint32_t vk, bool down);
        bool IsKeyDown(uint32_t vk) const;
        bool WasKeyPressed(uint32_t vk) const;
        bool WasKeyReleased(uint32_t vk) const;

        void SetMouseButton(uint32_t button, bool down);
        bool IsMouseButtonDown(uint32_t button) const;

        void SetMousePosition(int x, int y);
        void AddMouseWheelDelta(int delta);

        int MouseX() const { return m_mouseX; }
        int MouseY() const { return m_mouseY; }
        int MouseDeltaX() const { return m_mouseDeltaX; }
        int MouseDeltaY() const { return m_mouseDeltaY; }
        int MouseWheelDelta() const { return m_mouseWheelDelta; }

    private:
        std::array<bool, 256> m_keys{};
        std::array<bool, 256> m_prevKeys{};
        std::array<bool, 8> m_mouseButtons{};
        int m_mouseX = 0;
        int m_mouseY = 0;
        int m_mouseDeltaX = 0;
        int m_mouseDeltaY = 0;
        int m_mouseWheelDelta = 0;
    };
}
