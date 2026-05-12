#include "TrackballController.h"

#include <algorithm>

namespace Framework
{
	namespace
	{
		constexpr unsigned LeftMouseButton = 0;
		constexpr unsigned RightMouseButton = 1;
	}

	TrackballController::TrackballController()
	{
		Reset();
	}

	void TrackballController::Reset()
	{
		m_rotating = false;
		m_zooming = false;
		m_lastX = 0;
		m_lastY = 0;
		m_yaw = 0.0f;
		m_pitch = m_defaultPitch;
		UpdateWorldMatrix();
	}

	void TrackballController::SetViewport(int width, int height)
	{
		m_width = std::max(width, 1);
		m_height = std::max(height, 1);
	}

	void TrackballController::SetRotationSpeed(float radiansPerPixel)
	{
		m_rotationSpeed = radiansPerPixel;
	}

	void TrackballController::SetZoomSpeed(float unitsPerPixel)
	{
		m_zoomSpeed = unitsPerPixel;
	}

	void TrackballController::SetDistance(float distance)
	{
		m_distance = distance;
	}

	void TrackballController::OnMouseButtonDown(unsigned button, int x, int y)
	{
		m_lastX = x;
		m_lastY = y;

		if (button == LeftMouseButton)
			m_rotating = true;
		else if (button == RightMouseButton)
			m_zooming = true;
	}

	void TrackballController::OnMouseButtonUp(unsigned button, int x, int y)
	{
		(void)x;
		(void)y;

		if (button == LeftMouseButton)
			m_rotating = false;
		else if (button == RightMouseButton)
			m_zooming = false;
	}

	void TrackballController::OnMouseMove(int x, int y, int dx, int dy)
	{
		(void)dx;
		(void)dy;

		const int deltaX = x - m_lastX;
		const int deltaY = y - m_lastY;

		if (m_rotating)
		{
			m_yaw += static_cast<float>(deltaX) * m_rotationSpeed;
			m_pitch += static_cast<float>(deltaY) * m_rotationSpeed;

			// Keep pitch sane. The old SGI trackball could spin freely, but
			// this keeps the viewer predictable.
			const float limit = D3DX_PI * 0.49f;
			m_pitch = std::max(-limit, std::min(limit, m_pitch));

			UpdateWorldMatrix();
		}

		if (m_zooming)
		{
			m_distance += static_cast<float>(deltaY) * m_zoomSpeed;
		}

		m_lastX = x;
		m_lastY = y;
	}

	void TrackballController::UpdateWorldMatrix()
	{
		D3DXMATRIX yaw;
		D3DXMATRIX pitch;

		D3DXMatrixRotationY(&yaw, m_yaw);
		D3DXMatrixRotationX(&pitch, m_pitch);

		// Match the old viewer expectation: this matrix rotates the model, not
		// the camera.
		m_world = pitch * yaw;
	}
}
