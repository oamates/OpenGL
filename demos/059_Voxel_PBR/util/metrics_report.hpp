#pragma once
#include <chrono>
#include <map>

struct IMetric { };

template<typename T> struct Metric : IMetric
{
    T value;
    explicit Metric(const T &value);
};

struct ElapsedTime : public Metric<long long>
{
    std::chrono::time_point<std::chrono::steady_clock> time[2];

    void Begin();                                                           // Initializes the beginning clock time      
    template<class _Duration = std::chrono::nanoseconds> void End();        // Ends the clock and stores the elapsed time from begin to end.

    explicit ElapsedTime(const long long &value);
};

struct MetricsReport
{
    static std::map<std::string, IMetric *> storage;

    MetricsReport() = default;
    ~MetricsReport() = default;
        
    template<typename T> static void Write(const std::string id, const Metric<T> &value);   // Writes the given metric value and maps it to the id.
    template<typename T> static const T &Read(const std::string id);                        // Returns the metric value for the given metrics id
};

template <typename T> Metric<T>::Metric(const T &value)
    { this->value = value; }

template<class _Duration = std::chrono::nanoseconds> void ElapsedTime::End()
{
    time[1] = std::chrono::steady_clock::now();
    value = std::chrono::duration_cast<_Duration>(time[1] - time[0]).count();
}

template <typename T> void MetricsReport::Write(const std::string id, const Metric<T> &value)
    { storage[id] = &value; }

template <typename T> const T& MetricsReport::Read(const std::string id)
{
    static_assert(storage.find(id) != storage.end(), "Id not found");
    return static_cast<Metric<T> *>(storage[id])->value;
}