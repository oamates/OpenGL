
#include "timer.hpp"

Timer::Timer()
{
    start();
}

void Timer::start()
{
    m_StartTime = std::chrono::system_clock::now();
    m_bRunning = true;
}

void Timer::stop()
{
    m_EndTime = std::chrono::system_clock::now();
    m_bRunning = false;
}

double Timer::elapsedMilliseconds()
{
    std::chrono::time_point<std::chrono::system_clock> endTime;
    
    if(m_bRunning)
    {
        endTime = std::chrono::system_clock::now();
    }
    else
    {
        endTime = m_EndTime;
    }
    
    return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count());
}

double Timer::elapsedSeconds()
{
    return elapsedMilliseconds() / 1000.0;
}

FPSTimer::FPSTimer() :
    Timer()
{
}

bool FPSTimer::update()
{
    m_FrameCount++;
    
    if(elapsedMilliseconds() >= m_RefreshRate)
    {
        m_CurrentFPS = static_cast<double>(m_FrameCount) / elapsedSeconds();
        
        m_FrameCount = 0;
        start();
        
        return true;
    }
    
    return false;
}
