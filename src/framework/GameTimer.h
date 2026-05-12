#pragma once

#include <Windows.h>

namespace Framework
{
    class GameTimer
    {
    public:
        GameTimer();

        void Reset();
        void Tick();

        float DeltaSeconds() const { return m_deltaSeconds; }
        double TotalSeconds() const;

    private:
        LARGE_INTEGER m_frequency{};
        LARGE_INTEGER m_start{};
        LARGE_INTEGER m_previous{};
        LARGE_INTEGER m_current{};
        float m_deltaSeconds = 0.0f;
    };
}
