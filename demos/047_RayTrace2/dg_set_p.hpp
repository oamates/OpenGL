#ifndef DG_SET_H
#define DG_SET_H

//================================================================================
//
// An ordered array. Elements are inserted in order. Uses contiguous memory to store m_data, 
// therefore is best used for very small m_data types (i.e literals) and types with cheap assignment operators.
//
//================================================================================

#include <exception>
#include <cassert>

#include "impl_container_common.hpp"

namespace Dg
{
    //--------------------------------------------------------------------------------
    //	@	set_p<T>:	T: m_data type
    //--------------------------------------------------------------------------------
template<struct T> struct set_p
{
    T* m_data;
    int m_arraySize;
    int m_currentSize;

    set_p();                                    // If the constructor fails to allocate the set_p, the function throws a bad_alloc exception.
    set_p(unsigned int);                        // If the constructor fails to allocate the set_p, the function throws a bad_alloc exception.
    ~set_p();
    set_p(set_p const &);                       // Copy constructor.
    
    set_p& operator = (set_p const &);          // Assigns new contents to the container, replacing its current content.

    T& operator [] (unsigned int i)             // These function do not perform a range check.
        { return m_data[i]; }

    const T& operator [] (unsigned int i) const 
        { return m_data[i]; }
    
    int size() const                            // Returns the number of elements in the set.
        { return m_currentSize; }

    bool empty() const                          // Returns whether the set_p is empty.
        { return m_currentSize == 0; }

    int max_size() const                        // Returns number of elements the set_p can hold before resizing.
        { return m_arraySize; }

    // Searches the set for an element equivalent to t.
    // Returns true if the element was found with index being set_p to the index of the element in the set_p.
    // Returns false if not found with index set_p to one lower to where t would be.
    // lower : sets a low bound to the search sublist.
    bool find(const T& t, int& index, int lower = 0) const;

    // Searches the set_p for an element with a key equivalent to t.
    // Returns true if the element was found with index being set_p to the index of the element in the set_p. 
    // Returns false if not found with index set_p to one lower to where t would be.
    // lower : sets a low bound to the search sublist.
    // upper : sets an upper bound to the search sublist.
    bool find(const T& t, int& index, int lower, int upper) const;

    // Extends the container by inserting new elements, effectively increasing the container size by the number of elements inserted.
    // If the function fails to allocate memory, the function throws a bad_alloc exception.
    void insert(T const & t);

    // Extends the container by inserting new elements, effectively increasing the container size by the number of elements inserted.
    // If the function fails to allocate memory, the function throws a bad_alloc exception.
    // Returns false if t already exists in the set.
    bool insert_unique(const T& t);

    
    void erase(const T& t);                     // Removes a single element equal to t from the set.
    void erase_all(const T& t);                 // Removes all elements equal to t from the set.
    void clear();                               // Clear all items from the set_p, retains allocated memory.

    // Resize the set_p. The content of the set_p is preserved up to the lesser of the new and old sizes.
    // If the function fails to allocate memory, the function throws a bad_alloc exception.
    void resize(int);
    
    void reset();                               // Clears the set_p, reallocates memory to the set_p.

    // Doubles the memory allocated to the set_p. Retains all data.
    // If the function fails to allocate memory, the function throws a bad_alloc exception.
    void extend();
    void init(const set_p&);
};

//--------------------------------------------------------------------------------
//		Constructor
//--------------------------------------------------------------------------------
template<class T> set_p<T>::set_p()
    : m_data(0), m_arraySize(0), m_currentSize(0)
    { resize(DG_CONTAINER_DEFAULT_SIZE); }

//--------------------------------------------------------------------------------
//		Constructor
//--------------------------------------------------------------------------------
template<class T> set_p<T>::set_p(unsigned a_newSize)
    : m_data(0), m_arraySize(0), m_currentSize(0)
{
    assert(a_newSize > 0);
    resize(a_newSize);
}

//--------------------------------------------------------------------------------
//		Destructor
//--------------------------------------------------------------------------------
template<class T> set_p<T>::~set_p()
    { free( m_data ); }

//--------------------------------------------------------------------------------
//		Initialise set_p to another.
//--------------------------------------------------------------------------------
template<class T> void set_p<T>::init(const set_p& other)
{
    int sze = (other.m_arraySize>0) ? other.m_arraySize : 1;            // Resize lists
    resize(sze);
    memcpy(m_data, other.m_data, other.m_currentSize * sizeof(T));
    m_currentSize = other.m_currentSize;
}

//--------------------------------------------------------------------------------
//		Copy constructor
//--------------------------------------------------------------------------------
template<class T> set_p<T>::set_p(const set_p& other)
    : m_data(0), m_arraySize(0), m_currentSize(0)
    { init(other); }

//--------------------------------------------------------------------------------
//		Assignment
//--------------------------------------------------------------------------------
template<class T> set_p<T>& set_p<T>::operator = (const set_p& other)
{
    if (this == &other)
        return *this;
    init(other);
    return *this;
}

//--------------------------------------------------------------------------------
//		Resize map
//--------------------------------------------------------------------------------
template<class T> void set_p<T>::resize(int a_newSize)
{
    assert(a_newSize > 0);
    T* tempPtr = static_cast<T *>(realloc(m_data, sizeof(T) * a_newSize));
    if (!tempPtr)
        throw std::bad_alloc();
    m_data = tempPtr;
    m_arraySize = a_newSize;
    if (a_newSize < m_currentSize)
        m_currentSize = a_newSize;
}

//--------------------------------------------------------------------------------
//		Find a value in the list, uses a binary search algorithm
//--------------------------------------------------------------------------------
template<class T> bool set_p<T>::find(const T& a_item, int& a_index, int a_lower) const
    { return find(a_item, a_index, a_lower, m_currentSize - 1); }

//--------------------------------------------------------------------------------
//		Find a value in the list within a range, uses a binary search algorithm
//--------------------------------------------------------------------------------
template<class T> bool set_p<T>::find(const T& a_item, int& a_index, int a_lower, int a_upper) const
{
    a_lower = (a_lower > 0) ? a_lower : 0;                              // Check bounds
    a_upper = (a_upper < m_currentSize - 1) ? a_upper : m_currentSize - 1;

    while (a_lower <= a_upper)
    {
        a_index = ((a_upper + a_lower) >> 1);                           // calculate the midpoint for roughly equal partition
        if (m_data[a_index] < a_item)                                   // determine which subarray to search
            a_lower = a_index + 1;                                      // change min index to search upper subarray
        else if (m_data[a_index] > a_item)
            a_upper = a_index - 1;                                      // change max index to search lower subarray
        else
            return true;                                                // key found at index index
    }
    a_index = a_lower - 1;                                              // Set index closest (but lower) to key
    return false;
}

//--------------------------------------------------------------------------------
//	@	set_p<T>::extend()
//--------------------------------------------------------------------------------
template<class T> void set_p<T>::extend()
{
    int new_size = (m_arraySize << 1);                                  // Calculate new size

    if (new_size <= m_arraySize)                                        // overflow, map_p full
        throw std::overflow_error("m_arraySize");

    T* tempPtr = static_cast<T*>(realloc(m_data, sizeof(T) * new_size));
    if (!tempPtr)
        throw std::bad_alloc();

    m_data = tempPtr;
    m_arraySize = new_size;
}

//--------------------------------------------------------------------------------
//		Insert an element into the list
//--------------------------------------------------------------------------------
template<class T> void set_p<T>::insert(T const & a_item)
{
    int index;                                                          // Find the index to insert to
    find(a_item, index);

    if (m_currentSize == m_arraySize)                                   // Range check
        extend();

    // shift all RHS objects to the right by one.    
    memmove(&m_data[index + 2], &m_data[index + 1], (m_currentSize - index - 1) * sizeof(T));
    index++;
    memcpy(&m_data[index], &a_item, sizeof(T));
    m_currentSize++;
    return true;
}

//--------------------------------------------------------------------------------
//		Insert an element into the list, only if it does not yet exist
//--------------------------------------------------------------------------------
template<class T> bool set_p<T>::insert_unique(const T& item)
{
    int index;                                                          // Find the index to insert to
    if (find(item, index))
        return false;
    if (m_currentSize == m_arraySize)                                   // Range check
        extend();

    // shift all RHS objects to the right by one.
    memmove(&m_data[index + 2], &m_data[index + 1], (m_currentSize - index - 1) * sizeof(T));
    index++;
    memcpy(&m_data[index], &item, sizeof(T));
    m_currentSize++;
    return true;
}

//--------------------------------------------------------------------------------
//		Find and removes one of this element from the list.
//--------------------------------------------------------------------------------
template<class T> void set_p<T>::erase(const T& a_item)
{
    int index;                                                          // Find the index
    if (!find(a_item, index))
        return;	                                                        // element not found

    // Remove elements
    memmove(&m_data[index], &m_data[index + 1], (m_currentSize - index - 1) * sizeof(T));
    m_currentSize--;                                                    // Adjust m_currentSize
}

//--------------------------------------------------------------------------------
//		Find and removes all of this element from the list.
//--------------------------------------------------------------------------------
template<class T> void set_p<T>::erase_all(const T& a_item)
{
    
    int lower, upper;                                                   // Find the index
    if (!find(a_item, lower))
        return false;	                                                // element not found

    upper = lower + 1;                                                  // Initial upper bounds
    while (lower >= 0 && m_data[lower] == a_item)                       // Find lower bounds
        --lower;
    while (upper <= m_currentSize && m_data[upper] == a_item)           // Find upper bounds
      ++upper;
    
    lower++;                                                            // Number of elements to remove
    int num = upper - lower;
    
    // Remove elements
    memmove(&m_data[lower], &m_data[lower + num], (m_currentSize - index - 1 - num) * sizeof(T));
    m_currentSize -= num;                                               // Adjust m_currentSize
    return true;
}

//--------------------------------------------------------------------------------
//		Reset size to 1
//--------------------------------------------------------------------------------
template<class T> void set_p<T>::reset()
{
    clear();
    resize(DG_CONTAINER_DEFAULT_SIZE);
}

//--------------------------------------------------------------------------------
//		Set the number of elements to zero
//--------------------------------------------------------------------------------
template<class T> void set_p<T>::clear()
    { m_currentSize = 0; }

};

#endif