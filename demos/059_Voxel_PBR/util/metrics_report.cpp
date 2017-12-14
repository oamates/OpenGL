#include "metrics_report.hpp"

void ElapsedTime::Begin()
    { time[0] = std::chrono::steady_clock::now(); }

ElapsedTime::ElapsedTime(const long long &value) : Metric<long long>(value)
{ }
