#pragma once

#include <d3dx9.h>

namespace Framework
{
    // Small D3DX-based camera helper for the D3D9 sandbox.
    //
    // This is intentionally simple and keeps D3DXMATRIX/D3DXVECTOR3 because the
    // legacy renderer already uses D3DX everywhere. The goal is to stop app code
    // from manually rebuilding view/projection matrices in random places.
    class Camera
    {
    public:
        Camera();

        void LookAt(
            const D3DXVECTOR3& eye,
            const D3DXVECTOR3& target,
            const D3DXVECTOR3& up = D3DXVECTOR3(0.0f, 1.0f, 0.0f));

        void SetPerspective(
            float verticalFovRadians,
            float aspectRatio,
            float nearPlane,
            float farPlane);

        void SetPerspectiveDegrees(
            float verticalFovDegrees,
            float aspectRatio,
            float nearPlane,
            float farPlane);

        const D3DXVECTOR3& Eye() const { return m_eye; }
        const D3DXVECTOR3& Target() const { return m_target; }
        const D3DXVECTOR3& Up() const { return m_up; }

        const D3DXMATRIX& View() const { return m_view; }
        const D3DXMATRIX& Projection() const { return m_projection; }
        const D3DXMATRIX& ViewProjection() const { return m_viewProjection; }

        float NearPlane() const { return m_nearPlane; }
        float FarPlane() const { return m_farPlane; }
        float AspectRatio() const { return m_aspectRatio; }
        float VerticalFovRadians() const { return m_verticalFovRadians; }

    private:
        void RebuildView();
        void RebuildProjection();
        void RebuildViewProjection();

        D3DXVECTOR3 m_eye;
        D3DXVECTOR3 m_target;
        D3DXVECTOR3 m_up;

        float m_verticalFovRadians = D3DXToRadian(70.0f);
        float m_aspectRatio = 4.0f / 3.0f;
        float m_nearPlane = 1.0f;
        float m_farPlane = 1000.0f;

        D3DXMATRIX m_view;
        D3DXMATRIX m_projection;
        D3DXMATRIX m_viewProjection;
    };
}
