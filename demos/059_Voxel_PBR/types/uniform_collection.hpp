#pragma once
#include <vector>

// Holds collections of program uniforms of the same type these collections have to be identifier by countable identifier
template<typename T1, typename T2> struct UniformCollection
{
    std::vector<std::pair<T2, T1> *> links;
    std::vector<T2> actives;
    
    const std::vector<T2> &Actives() const;                         // Active uniforms identifiers
    void Resize(const size_t idCount);                              // Resizes the specified identifier vector
    void Save(T2 id, T1 uniform);                                   // Saves the specified identifier and related program uniform
    bool Has(const T2 &id) const;                                   // Determines whether the class holds an uniform associated with the specified identifier.        
    T1 &operator[](const T2 &id);                                   // Indexes for the class, returns the id associated uniform
};


template<typename T1, typename T2> void UniformCollection<T1, T2>::Resize(const size_t idCount)
{
    links.clear();
    links.resize(idCount);
}

template<typename T1, typename T2> void UniformCollection<T1, T2>::Save(T2 id, T1 uniform)
{
    if (static_cast<int>(id) < 0 || static_cast<size_t>(id) >= links.size())
        return;

    actives.push_back(std::move(id));
    links[static_cast<int>(id)] = new std::pair<T2, T1>(actives.back(), std::move(uniform));
}

template<typename T1, typename T2> bool UniformCollection<T1, T2>::Has(const T2 &id) const
    { return links[static_cast<int>(id)] != nullptr; }

template<typename T1, typename T2> const std::vector<T2> &UniformCollection<T1, T2>::Actives() const
    { return actives; }

template<typename T1, typename T2> T1 &UniformCollection<T1, T2>::operator[] (const T2 &id)
    { return links[static_cast<int>(id)]->second; }


