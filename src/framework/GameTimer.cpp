#include "GameTimer.h"

namespace Framework
{
    GameTimer::GameTimer()
    {
        QueryPerformanceFrequency(&m_frequency);
        Reset();
    }

    void GameTimer::Reset()
    {
        QueryPerformanceCounter(&m_start);
        m_previous = m_start;
        m_current = m_start;
        m_deltaSeconds = 0.0f;
    }

    void GameTimer::Tick()
    {
        m_previous = m_current;
        QueryPerformanceCounter(&m_current);

        const double delta = static_cast<double>(m_current.QuadPart - m_previous.QuadPart) /
                             static_cast<double>(m_frequency.QuadPart);
        m_deltaSeconds = static_cast<float>(delta > 0.0 ? delta : 0.0);
    }

    double GameTimer::TotalSeconds() const
    {
        return static_cast<double>(m_current.QuadPart - m_start.QuadPart) /
               static_cast<double>(m_frequency.QuadPart);
    }
}
