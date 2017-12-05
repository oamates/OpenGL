#ifndef DG_VECTOR_P_H
#define DG_VECTOR_P_H

#include <cstdint>
#include <exception>
#include <cassert>

#include "impl_container_common.hpp"

// DgArrays are sequence containers representing arrays that can change in size.
// Just like arrays, DgArrays use contiguous storage locations for their elements,
// which means that their elements can also be accessed using offsets on regular
// pointers to its elements, and just as efficiently as in arrays. But unlike arrays,
// their size can change dynamically, with their storage being handled automatically by
// the container.
//
// Internally, DgArrays use a dynamically allocated array to store their elements.
// This array may need to be reallocated in order to grow in size when new elements
// are inserted, which implies allocating a new array and moving all elements to it.
// This is a relatively expensive task in terms of processing time, and thus, DgArrays
// do not reallocate each time an element is added to the container.
//
// Instead, vector_p containers may allocate some extra storage to accommodate for
// possible growth, and thus the container may have an actual capacity greater than
// the storage strictly needed to contain its elements (i.e., its size).

namespace Dg
{

template<struct T> struct vector_p
{
    T* m_data;
    uint32_t m_arraySize;
    uint32_t m_currentSize;

    vector_p();                                         // constructor and destructor
    vector_p(uint32_t);                                 // construct with a set size
    ~vector_p();

    vector_p(const vector_p&);
    vector_p& operator= (const vector_p&);
    void copy_all(const vector_p& other);               // Copy both the current elements and the elements in the reserved memory.
    
    T& operator[] (uint32_t i)                          // Access element
        { return m_data[i]; }
    
    const T& operator[] (uint32_t i) const              // Accessor, no range check.
        { return m_data[i]; }

    T& back()                                           // Calling this function on an empty container causes undefined behavior.
        { return m_data[m_arraySize - 1]; }

    const T& back() const                               // Calling this function on an empty container causes undefined behavior.
        { return m_data[m_arraySize - 1]; }

    T& at(uint32_t);                                    // Accessor with range check.

    const T& at(uint32_t) const;                        // Accessor with range check.

    uint32_t size() const                               // Current size of the array
        { return m_currentSize; }
    
    bool empty() const                                  // Is the array empty
        { return m_currentSize == 0; }
    
    uint32_t max_size()	const                           // Size of the reserved memory.
        { return m_arraySize; }
    
    T* data()                                           // Get pointer to first element.
        { return m_data; }
    
    const T* Data()	const                               // Get pointer to first element.
        { return m_data; }

    void push_back(const T&);                           // Add element to the back of the array.
    void pop_back();                                    // Remove element from the back of the array.
    void push_front(const T&);                          // Add element to the back of the array.
    void pop_front();                                   // Remove element from the back of the array.
    void clear();                                       // Current size is flagged as 0. Elements are NOT destroyed.
    void resize(uint32_t new_size);                     // Set the current size to 0 and the reserve to new_size
    void extend();                                      // Exteneds the total size of the array (current + reserve) by a factor of 2
    void init(const vector_p&);
};

//--------------------------------------------------------------------------------
//		Constructor
//--------------------------------------------------------------------------------
template<class T> vector_p<T>::vector_p() 
  : m_arraySize(DG_CONTAINER_DEFAULT_SIZE), m_currentSize(0)
{
    m_data = static_cast<T*> (malloc(m_arraySize * sizeof(T)));
    if (!m_data)
        throw std::bad_alloc;
}

//--------------------------------------------------------------------------------
//		Constructor
//--------------------------------------------------------------------------------
template<class T> vector_p<T>::vector_p(uint32_t a_size)
  : m_currentSize(0), m_arraySize(a_size)
{
    assert(a_size != 0);
    m_data = static_cast<T*>(malloc(m_arraySize * sizeof(T)));
    if (!m_data)
        throw std::bad_alloc;
}

//--------------------------------------------------------------------------------
//		Destructor
//--------------------------------------------------------------------------------
template<class T> vector_p<T>::~vector_p()
    { free(m_data); }

//--------------------------------------------------------------------------------
//		Initialise array to another vector_p
//--------------------------------------------------------------------------------
template<class T> void vector_p<T>::init(const vector_p& a_other)
{
    T* tempPtr = static_cast<T*> (realloc(a_.m_arraySize * sizeof(T)));
    if (!tempPtr)
        throw std::bad_alloc;

    m_data = tempPtr;
    m_arraySize = a_other.m_arraySize;
    m_currentSize = a_other.m_currentSize;
    memcpy(m_data, a_other.m_data, a_other.m_currentSize * sizeof(T));
}

//--------------------------------------------------------------------------------
//		Copy constructor
//--------------------------------------------------------------------------------
template<class T> vector_p<T>::vector_p(const vector_p& other) : m_data(0)
    { init(other); }

//--------------------------------------------------------------------------------
//		Assignment
//--------------------------------------------------------------------------------
template<class T> vector_p<T>& vector_p<T>::operator=(const vector_p& other)
{
    if (this == &other)
        return *this;
    init(other);
    return *this;
}

//--------------------------------------------------------------------------------
//		Copies entire array
//--------------------------------------------------------------------------------
template<class T> void vector_p<T>::copy_all(const vector_p<T>& a_other)
{
    if (m_arraySize != a_other.m_arraySize)
        resize(a_other.m_arraySize);
    memcpy(m_data, a_other.m_data, m_arraySize * sizeof(T));
}

//--------------------------------------------------------------------------------
//		Accessor with range check
//--------------------------------------------------------------------------------
template<class T> T& vector_p<T>::at(uint32_t index)
{
    if (index >= m_currentSize)
        throw std::out_of_range("vector_p: range error");
    return m_data[index];
}

//--------------------------------------------------------------------------------
//		const accessor with range check
//--------------------------------------------------------------------------------
template<class T> const T& vector_p<T>::at(uint32_t index) const
{
    if (index >= m_currentSize)
        throw std::out_of_range("vector_p: range error");
    return m_data[index];
}

//--------------------------------------------------------------------------------
//		Add element to the back of the array
//--------------------------------------------------------------------------------
template<class T> void vector_p<T>::push_back(const T& a_item)
{
    if (m_currentSize == m_arraySize)
        extend();
    memcpy(&m_data[m_currentSize], &a_item, sizeof(T));
    ++m_currentSize;
}

//--------------------------------------------------------------------------------
//		Pop an element from the back of the array
//--------------------------------------------------------------------------------
template<class T> void vector_p<T>::pop_back()
{
    if (m_currentSize == 0)             //Range check
        return;
    --m_currentSize;                    //Deincrement current size
}

//--------------------------------------------------------------------------------
//		Add element to the front of the array
//--------------------------------------------------------------------------------
template<class T> void vector_p<T>::push_front(const T& a_item)
{
    if (m_currentSize == m_arraySize)           //Range check
        extend();
    memmove(&m_data[1], &m_data[0], m_currentSize * sizeof(T));
    memcpy(&m_data[0], &a_item, sizeof(T));
    ++m_currentSize;                            //increment current size
}

//--------------------------------------------------------------------------------
//		Pop an element from the front of the array
//--------------------------------------------------------------------------------
template<class T> void vector_p<T>::pop_front()
{
    if (m_currentSize == 0)                         //Range check
        return;
    memmove(&m_data[0], &m_data[1], (m_currentSize - 1) * sizeof(T));
    --m_currentSize;                                //Deincrement current size
}

//--------------------------------------------------------------------------------
//		Clear array :: Set current size to 0
//--------------------------------------------------------------------------------
template<class T> void vector_p<T>::clear()
    { m_currentSize = 0; }

//--------------------------------------------------------------------------------
//		Resize array, erases all m_data before resize.
//--------------------------------------------------------------------------------
template<class T> void vector_p<T>::resize(uint32_t a_size)
{
    assert(a_size != 0);

    T* tempPtr = static_cast<T*> (realloc(m_data, a_size * sizeof(T)));

    if (!tempPtr)
        throw std::bad_alloc;

    m_data = tempPtr;

  
    m_arraySize = a_size;                             //Set sizes
    if (a_size < m_currentSize)
        m_currentSize = a_size;
}

//--------------------------------------------------------------------------------
//		Extend the array by a factor of 2, keeps all m_data.
//--------------------------------------------------------------------------------
template<class T> void vector_p<T>::extend()
{
    uint32_t new_size = m_arraySize << 1;                 //Calculate new size 
    
    if (new_size < m_arraySize)
        throw std::overflow_error("m_arraySize");
    
    T* tempPtr = static_cast<T*> (realloc(m_data, a_size * sizeof(T)));
    
    if (!tempPtr)
        throw std::bad_alloc;
    
    m_data = tempPtr;
    m_arraySize = a_size;                               //Set sizes
    if (a_size < m_currentSize)
        m_currentSize = a_size;
}

//--------------------------------------------------------------------------------
//		Helpful functions
//--------------------------------------------------------------------------------


//--------------------------------------------------------------------------------
//		Find a value in the list, returns reference
//--------------------------------------------------------------------------------
template<class T> T* find(vector_p<T>& container, const T& val)
{
    for (uint32_t i = 0; i < container.size(); ++i)
        if (container[i] == val)
            return &container[i];
    return 0;
}

//--------------------------------------------------------------------------------
//		Fill array with value
//--------------------------------------------------------------------------------
template<class T> void fill(vector_p<T>& container, const T& val)
{
    for (uint32_t i = 0; i < container.size(); ++i)
        container[i] = val;
}

};
#endif