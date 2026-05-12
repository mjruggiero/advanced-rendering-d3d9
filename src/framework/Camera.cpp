#include "Camera.h"

namespace Framework
{
    Camera::Camera()
        : m_eye(0.0f, 0.0f, -120.0f)
        , m_target(0.0f, 0.0f, 0.0f)
        , m_up(0.0f, 1.0f, 0.0f)
    {
        RebuildView();
        RebuildProjection();
        RebuildViewProjection();
    }

    void Camera::LookAt(
        const D3DXVECTOR3& eye,
        const D3DXVECTOR3& target,
        const D3DXVECTOR3& up)
    {
        m_eye = eye;
        m_target = target;
        m_up = up;

        RebuildView();
        RebuildViewProjection();
    }

    void Camera::SetPerspective(
        float verticalFovRadians,
        float aspectRatio,
        float nearPlane,
        float farPlane)
    {
        m_verticalFovRadians = verticalFovRadians;
        m_aspectRatio = aspectRatio > 0.0f ? aspectRatio : 1.0f;
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;

        RebuildProjection();
        RebuildViewProjection();
    }

    void Camera::SetPerspectiveDegrees(
        float verticalFovDegrees,
        float aspectRatio,
        float nearPlane,
        float farPlane)
    {
        SetPerspective(
            D3DXToRadian(verticalFovDegrees),
            aspectRatio,
            nearPlane,
            farPlane);
    }

    void Camera::RebuildView()
    {
        D3DXMatrixLookAtLH(&m_view, &m_eye, &m_target, &m_up);
    }

    void Camera::RebuildProjection()
    {
        D3DXMatrixPerspectiveFovLH(
            &m_projection,
            m_verticalFovRadians,
            m_aspectRatio,
            m_nearPlane,
            m_farPlane);
    }

    void Camera::RebuildViewProjection()
    {
        D3DXMatrixMultiply(&m_viewProjection, &m_view, &m_projection);
    }
}
