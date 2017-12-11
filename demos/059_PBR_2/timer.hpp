#pragma once

#include <chrono>

struct Timer
{
    explicit Timer();

    void start();
    void stop();
    
    double elapsedMilliseconds();
    double elapsedSeconds();
    
    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;
    bool                                               m_bRunning = false;
};

struct FPSTimer : public Timer
{
	FPSTimer();
    
    bool update();
    
    double getFPS()
    {
        return m_CurrentFPS;
    }
    
    void setRefreshRate(double refreshRate)
    {
        m_RefreshRate = refreshRate;
    }
    
    Timer m_Timer;
    
    double m_RefreshRate   = 1000;
    int    m_FrameCount    = 0;
    double m_CurrentFPS    = 0;
};