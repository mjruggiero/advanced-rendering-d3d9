#pragma once

#include <d3dx9math.h>

namespace Framework
{
    // Simple model-orbit controller used by the CharacterEngine viewer.
    //
    // Left mouse drag rotates the model.
    // Right mouse drag zooms in/out.
    //
    // This replaces the old SGI C-style virtual trackball code with a small
    // C++ class that owns its state explicitly.
    class TrackballController
    {
    public:
        TrackballController();
        void Reset();

        void SetViewport(int width, int height);
        void SetRotationSpeed(float radiansPerPixel);
        void SetZoomSpeed(float unitsPerPixel);

        void SetDistance(float distance);
        float Distance() const { return m_distance; }

        void OnMouseButtonDown(unsigned button, int x, int y);
        void OnMouseButtonUp(unsigned button, int x, int y);
        void OnMouseMove(int x, int y, int dx, int dy);

        const D3DXMATRIX& WorldMatrix() const { return m_world; }

    private:
        void UpdateWorldMatrix();

        int m_width = 1;
        int m_height = 1;

        bool m_rotating = false;
        bool m_zooming = false;

        int m_lastX = 0;
        int m_lastY = 0;

        float m_yaw = 0.0f;
        float m_pitch = 0.0f;
        float m_distance = -120.0f;

        float m_rotationSpeed = 0.01f;
        float m_zoomSpeed = 1.0f;

        float m_defaultPitch = -D3DX_PI * 0.5f;

        D3DXMATRIX m_world = {};
    };
}
