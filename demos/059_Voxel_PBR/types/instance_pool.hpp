#pragma once
#include <vector>
#include <map>

// Stores all instaces of the inheriting class
template<typename T> struct InstancePool
{
    static std::vector<T *> instances;                          // The instance pool        
    static std::map<long long, long long> location;             // The location of each instance in the instances collection
    unsigned long long instanceId;
    unsigned long long priorityIndex;

    void Priority(const long long priority);                    // Sets this instance priority. Updating its position in the instances collection.
    T &GetInstance(const long long id);                         // Gets the instance with the given identifier
    InstancePool<T> &operator=(const InstancePool<T> &rhs);
    InstancePool();
    virtual ~InstancePool();
};

template<typename T> std::map<long long, long long> InstancePool<T>::location;

template<typename T> std::vector<T *> InstancePool<T>::instances;

template <typename T> InstancePool<T>::InstancePool()
{
    instanceId = static_cast<unsigned int>(instances.size());
    priorityIndex = instanceId;
    instances.push_back(static_cast<T *>(this));
    location[instanceId] = priorityIndex;
}

template <typename T> InstancePool<T>::~InstancePool()
{
    // delete self from collections
    auto index = location[instanceId];
    location.erase(instanceId);
    instances.erase(instances.begin() + index);

    // update locations
    for (size_t i = 0; i < instances.size(); ++i)
    {
        location[instances[i]->instanceId] = i;
    }
}

template <typename T>
void InstancePool<T>::Priority(const long long priority)
{
    // keep reference
    auto &instance = instances[priorityIndex];
    // remove from position
    instances.erase(instances.begin() + priorityIndex);
    // move to new place
    instances.insert(priority, instance);
    // update location
    location[instanceId] = priority;
}

template <typename T>
T &InstancePool<T>::GetInstance(const long long id)
{
    return instances[location[id]];
}

template <typename T>
InstancePool<T> &InstancePool<T>::operator=(const InstancePool<T> &rhs)
{
    return instanceId == rhs.instanceId;
}