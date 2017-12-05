#ifndef DG_MAP_P_H
#define DG_MAP_P_H

#include <exception>
#include <stdexcept>
#include <cassert>

#include "impl_container_common.hpp"

namespace Dg
{

// Ordered map_pped list. 
// Assumed types are POD, so no construction / assignment operators called

template<typename U, typename T> struct map_p
{
    struct Container
    {
        U key;
        T item;
    };

    Container* m_data;
    int m_arraySize;
    int m_currentSize;

    map_p();                                                // If the constructor fails to allocate the map_p, the function throws a bad_alloc exception.
    map_p(unsigned int);
    ~map_p();
    
    map_p(map_p const &);                                   // Copy constructor.
    map_p& operator = (const map_p&);                       // Assigns new contents to the container, replacing its current content.

    
    T& operator [] (unsigned int i)                         // These function do not perform a range check.
        { return m_data[i].item; }
    const T& operator [] (unsigned int i) const
        { return m_data[i].item; }
    
    int size() const                                        // Return number of elements in the map_p.
        { return m_currentSize; }
    
    bool empty() const                                      // Returns whether the map_p is empty.
        { return m_currentSize == 0; }
    
    int max_size() const                                    // Returns number of elements the map_p can hold before resizing.
        { return m_arraySize; }
    
    U query_key(int i) const                                // This function does not perform a range check.
        { return m_data[i].key; }

    // Searches the map_p for an element with a key equivalent to k.
    // Returns true if the element was found with index being set to the index of the element inthe map_p.
    // Returns false if not found with \a index set to one lower to where k would be.
    // lower : sets a low bound to the search sublist. Uses binary search
    bool find(U k, int& index, int lower = 0) const;

    // Searches the map_p for an element with a key equivalent to k.
    // Returns true if the element was found with index being set to the index of the element inthe map_p. 
    // Returns false if not found with index set to one lower to where k would be.
    // lower : sets a low bound to the search sublist.
    // upper : sets an upper bound to the search sublist. Uses binary search
    bool find(U k, int& index, int lower, int upper) const;

    // Extends the container by inserting new elements, effectively increasing the container size by the number of elements inserted.
    // If the function fails to allocate memory, the function throws a bad_alloc exception.
    // Returns false if key already exists in the map_p.
    bool insert(U k, T const & t);
    
    bool set(U k, T t);                                     // Set element with key k, with value t. Returns true if key found.
    void erase(U k);                                        // Removes the item in the map_p with key k.
    void erase_at_position(int);                            // Removes the item in the map_p at position i.
    void clear();                                           // Clear all items from the map_p, retains allocated memory.

    // Resize the map_p. The content of the map_p is preserved up to the lesser of the new and old sizes.
    // If the function fails to allocate memory, the function throws a bad_alloc exception.
    void resize(int);

    void reset();                                           // Clears the map_p, reallocates memory to the map_p.

    // Doubles the memory allocated to the map_p. Retains all data.
    // If the function fails to allocate memory, the function throws a bad_alloc exception.
    void extend();
    void init(map_p const &);
};


//--------------------------------------------------------------------------------
//    @   map_p<U,T>::map_p()
//--------------------------------------------------------------------------------
template<typename U, typename T> map_p<U, T>::map_p()
    : m_data(0), m_arraySize(0), m_currentSize(0)
    { resize(DG_CONTAINER_DEFAULT_SIZE); }

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::map_p()
//--------------------------------------------------------------------------------
template<typename U, typename T> map_p<U, T>::map_p(unsigned int a_size)
    : m_data(nullptr) , m_arraySize(0) , m_currentSize(0)
{
    assert(a_size > 0);

    Container* tempPtr = static_cast<Container *> (malloc(sizeof(Container) * a_size));
    if (!tempPtr) throw std::bad_alloc();

    m_data = tempPtr;
    m_arraySize = a_size;
    m_currentSize = 0;
}

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::~map_p()
//--------------------------------------------------------------------------------
template<typename U, typename T> map_p<U, T>::~map_p()
    { free(m_data); }

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::init()
//--------------------------------------------------------------------------------
template<typename U, typename T> void map_p<U, T>::init(map_p const & a_other)
{
    resize(a_other.m_arraySize);
    memcpy(m_data, a_other.m_data, a_other.m_currentSize * sizeof(Container));
    m_currentSize = a_other.m_currentSize;
}

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::map_p()
//--------------------------------------------------------------------------------
template<typename U, typename T> map_p<U, T>::map_p(map_p const & a_other)
    : m_data(0), m_arraySize(0), m_currentSize(0)
    { init(a_other); }

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::operator=()
//--------------------------------------------------------------------------------
template<typename U, typename T> map_p<U, T>& map_p<U, T>::operator=(map_p const & a_other)
{
    if (this == &a_other)
        return *this;
    init(a_other);
    return *this;
}

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::resize()
//--------------------------------------------------------------------------------
template<typename U, typename T> void map_p<U, T>::resize(int a_newSize)
{
    assert(a_newSize > 0);
    Container* tempPtr = static_cast<Container *>(realloc(m_data, sizeof(Container) * a_newSize));
    if (!tempPtr)
        throw std::bad_alloc();
    m_data = tempPtr;
    m_arraySize = a_newSize;
    if (a_newSize < m_currentSize)
        m_currentSize = a_newSize;
}

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::find()
//--------------------------------------------------------------------------------
template<typename U, typename T> bool map_p<U, T>::find(U a_key, int& a_index, int a_lower) const
    { return find(a_key, a_index, a_lower, (m_currentSize - 1)); }

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::find()
//--------------------------------------------------------------------------------
template<typename U, typename T> bool map_p<U, T>::find(U a_key, int& a_index, int a_lower, int a_upper) const
{
    a_lower = (a_lower > 0) ? a_lower : 0;                  // Check bounds
    a_upper = (a_upper < m_currentSize - 1) ? a_upper : m_currentSize - 1;

    while (a_lower <= a_upper)
    {
        a_index = ((a_upper + a_lower) >> 1);               // calculate the midpoint for roughly equal partition
        if (m_data[a_index].key < a_key)                    // determine which subarray to search
            a_lower = a_index + 1;                          // change min index to search upper subarray
        else if (m_data[a_index].key > a_key)
            a_upper = a_index - 1;                          // change max index to search lower subarray
        else
            return true;                                    // key found at index index
    }
    a_index = a_lower - 1;                                  // Set index closest (but lower) to key
    return false;
}

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::extend()
//--------------------------------------------------------------------------------
template<typename U, typename T> void map_p<U, T>::extend()
{
    int new_size = (m_arraySize << 1);                      // calculate new size
    if (new_size <= m_arraySize)                            // overflow, map_p full
        throw std::overflow_error("m_arraySize");

    Container * tempPtr = static_cast<Container*>(realloc(m_data, sizeof(Container) * new_size));
    if (!tempPtr)
        throw std::bad_alloc();
    m_data = tempPtr;
    m_arraySize = new_size;
}

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::insert()
//--------------------------------------------------------------------------------
template<typename U, typename T> bool map_p<U, T>::insert(U a_key, T const & a_item)
{
    int index;                                              // find the index to insert to
    if (find(a_key, index))
        return false;                                       // element already exists
    if (m_currentSize == m_arraySize)                       // range check
        extend();

    // shift all RHS objects to the right by one.
    memmove(&m_data[index + 2], &m_data[index + 1], (m_currentSize - index - 1) * sizeof(Container));
    index++;
    memcpy(&m_data[index].key, &a_key, sizeof(a_key));
    memcpy(&m_data[index].item, &a_item, sizeof(a_item));
    m_currentSize++;
    return true;
}

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::erase()
//--------------------------------------------------------------------------------
template<typename U, typename T> void map_p<U, T>::erase(U a_key)
{
    int index;                                              // find the index
    if (!find(a_key, index))
        return;
    memmove(&m_data[index], &m_data[index + 1], (m_currentSize - index - 1) * sizeof(Container));
    m_currentSize--;
}

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::erase_at_position()
//--------------------------------------------------------------------------------
template<typename U, typename T> void map_p<U, T>::erase_at_position(int a_i)
{
    if (a_i > 0 && a_i < m_currentSize)
    {
        memmove(&m_data[a_i], &m_data[a_i + 1], (m_currentSize - a_i - 1) * sizeof(Container));
        m_currentSize--;
    }
}

//--------------------------------------------------------------------------------
//    @   map_p<U,T>::set()
//--------------------------------------------------------------------------------
template<typename U, typename T> bool map_p<U, T>::set(U a_key, T a_item)
{
    int index;                                              // find the index to insert to
    if (!find(a_key, index))
        return false;                                       // element does not exist
    memcpy(&m_data[index].item, &a_item, sizeof(a_item));
    return true;
}

//--------------------------------------------------------------------------------
//    @   Dgmap_p<U,T>::reset()
//--------------------------------------------------------------------------------
template<typename U, typename T> void map_p<U, T>::reset()
{
    clear();
    resize(DG_CONTAINER_DEFAULT_SIZE);
}

//--------------------------------------------------------------------------------
//    @   Dgmap_p<U,T>::clear()
//--------------------------------------------------------------------------------
template<typename U, typename T> void map_p<U, T>::clear()
    { m_currentSize = 0; }

};

#endif