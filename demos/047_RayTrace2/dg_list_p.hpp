//================================================================================
//
// Pre-allocated Linked list. Similar to std::list with similarly named methods
// and functionality. The underlying arrays are preallocated and only change in
// size if extending list past that allocated, or manually resizing. This makes
// for fast insertion/erasing of elements.
//
//================================================================================

#ifndef DG_LIST_S_H
#define DG_LIST_S_H

#include <exception>

#include "impl_container_common.hpp"

namespace Dg
{

//--------------------------------------------------------------------------------
// list_p<T>:  T: m_data type
//--------------------------------------------------------------------------------
template<typename T> struct list_p
{      
    DataContainer* m_data;                          // preallocated block of memory to hold elements
    DataContainer m_rootContainer;                  // root and end objects, and pointers
    DataContainer m_endContainer;
    DataContainer* m_nextFree;                      // next free element in the list;
      
    size_t m_currentSize;                           // sizes
    size_t m_arraySize;

    //--------------------------------------------------------------------------------
    //        Container to hold the object and pointers
    //--------------------------------------------------------------------------------
    struct DataContainer
    {
        DataContainer()
            : next(nullptr), previous(nullptr) {}
        
        DataContainer* next;
        DataContainer* previous;
        T element;
    };

    //--------------------------------------------------------------------------------
    //        Constant Iterator
    //--------------------------------------------------------------------------------
    struct const_iterator
    {
        const DataContainer* ptr;
        const_iterator(const DataContainer* _ptr)                       // special constructor
            { ptr = _ptr; }

        const_iterator(){}
        ~const_iterator(){}

        const_iterator(const const_iterator& it): ptr(it.ptr) {}        // copy operations
        const_iterator& operator = (const const_iterator&);
        
        bool operator == (const const_iterator& it) const               // comparison
            { return ptr == it.ptr; }
        bool operator != (const const_iterator& it) const
            { return ptr != it.ptr; }

        const_iterator& operator ++ ();
        const_iterator  operator ++ (int);
        const_iterator& operator -- ();
        const_iterator  operator -- (int);

        const T* operator -> () const
            { return &(ptr->element); }
        const T& operator * () const
            { return ptr->element; }
    };

    //--------------------------------------------------------------------------------
    //        Iterator
    //--------------------------------------------------------------------------------
    struct iterator
    {
        DataContainer* ptr;

        iterator(DataContainer* _ptr)
            { ptr = _ptr; }

        iterator() {}
        ~iterator() {}

        iterator(const iterator& it): ptr(it.ptr) {}                // copy operations
        iterator& operator = (const iterator&);
          
        bool operator == (const iterator& it) const                 // comparison operators
            { return ptr == it.ptr; }
        bool operator != (const iterator& it) const
            { return ptr != it.ptr; }
          
        iterator& operator ++ ();                                   // operators
        iterator  operator ++ (int);
        iterator& operator -- ();
        iterator  operator -- (int);
          
        operator const_iterator() const                             // conversion
            { return const_iterator(ptr); }
        T* operator -> () const
            { return &(ptr->element); }
        T& operator * () const
            { return ptr->element; }

    };

    list_p(size_t a_size = 1);                                      // constructor and destructor
    ~list_p();
      
    list_p(const list_p&);                                          // copy operations
    list_p& operator = (const list_p&);

    iterator begin() const
        { return iterator(m_rootContainer.next); }
    iterator end() const
        { return iterator(const_cast<DataContainer*> (&m_endContainer)); }
    const_iterator cbegin() const
        { return const_iterator(m_rootContainer.next); }
    const_iterator cend() const
        { return const_iterator(const_cast<DataContainer*> (&m_endContainer)); }
    size_t size() const
        { return m_currentSize; }
    size_t max_size() const
        { return m_arraySize; }
    bool empty() const
        { return m_currentSize == 0; }
    T& back() const
        { return m_endContainer.previous->element; }
    T& front() const
        { return m_rootContainer.next->element; }
      
    void push_back(const T&);                                       // manipulators
    bool push_back();
    void push_front(const T&);
    bool push_front();
    void insert(const iterator&, const T&);
    void pop_back();
    void pop_front();
    void erase(iterator&);                                          // needs to return iterator
    void clear();
    void resize(size_t);
    void extend();                                                  // functions
    void init(size_t new_size);
};

//--------------------------------------------------------------------------------
//        iterator Assignment
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::iterator& list_p<T>::iterator::operator = (const typename list_p<T>::iterator& other)
{
    ptr = other.ptr;
    return *this;
}

//--------------------------------------------------------------------------------
//        iterator pre increment
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::iterator& list_p<T>::iterator::operator++()
{
    ptr = ptr->next;
    return *this;
}

//--------------------------------------------------------------------------------
//        iterator post increment
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::iterator list_p<T>::iterator::operator++(int)
{                                                                   
    iterator result(*this);                                         // make a copy for result
    ++(*this);                                                      // Now use the prefix version to do the work
    return result;                                                  // return the copy (the old) value.

}

//--------------------------------------------------------------------------------
//        iterator pre decrement
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::iterator& list_p<T>::iterator::operator--()
{
    ptr = ptr->previous;
    return *this;
}

//--------------------------------------------------------------------------------
//        iterator post decrement
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::iterator list_p<T>::iterator::operator--(int)
{
    iterator result(*this);                                         // make a copy for result
    --(*this);                                                      // Now use the prefix version to do the work
    return result;                                                  // return the copy (the old) value.

}

//--------------------------------------------------------------------------------
//        const_iterator Assignment
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::const_iterator& list_p<T>::const_iterator::operator = (const typename list_p<T>::const_iterator& other)
{
    ptr = other.ptr;
    return *this;
}

//--------------------------------------------------------------------------------
//        const_iterator pre increment
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::const_iterator& list_p<T>::const_iterator::operator++()
{
    ptr = ptr->next;
    return *this;

}

//--------------------------------------------------------------------------------
//        const_iterator post increment
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::const_iterator list_p<T>::const_iterator::operator++(int)
{
    const_iterator result(*this);                                   // make a copy for result
    ++(*this);                                                      // Now use the prefix version to do the work
    return result;                                                  // return the copy (the old) value.
}

//--------------------------------------------------------------------------------
//        const_iterator pre decrement
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::const_iterator& list_p<T>::const_iterator::operator -- ()
{
    ptr = ptr->previous;
    return *this;
}

//--------------------------------------------------------------------------------
//        const_iterator post decrement
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::const_iterator list_p<T>::const_iterator::operator -- (int)
{
    const_iterator result(*this);                                   // make a copy for result
    --(*this);                                                      // Now use the prefix version to do the work
    return result;                                                  // return the copy (the old) value.
}

//--------------------------------------------------------------------------------
//        General initialise function
//--------------------------------------------------------------------------------
template<struct T> void list_p<T>::init(size_t a_size)
{
    assert(a_size > 0);
    T* tempPtr = static_cast<DataContainer *> (realloc(m_data, a_size * sizeof(DataContainer)));
    if (tempPtr == nullptr)
        throw std::bad_alloc;
    m_data = tempPtr;
    m_arraySize = a_size;                                           // Assign sizes
    m_currentSize = 0;
    m_nextFree = &m_data[0];                                        // Initialise m_data
    m_rootContainer.next = &m_endContainer;                         // Set outer container pointers
    m_endContainer.previous = &m_rootContainer;
    for (size_t i = 0; i < m_arraySize-1; i++)                      // Only need to assign forward pointers
        m_data[i].next = &m_data[i + 1];
}

//--------------------------------------------------------------------------------
//        Constructor
//--------------------------------------------------------------------------------
template<struct T> list_p<T>::list_p() : m_data(0), m_nextFree(0)
    { init(DG_CONTAINER_DEFAULT_SIZE); }

//--------------------------------------------------------------------------------
//        Constructor
//--------------------------------------------------------------------------------
template<struct T> list_p<T>::list_p(size_t a_size): m_data(nullptr), m_nextFree(nullptr)
{
    assert(a_size > 0);                                             // Size must be at least 1
    init(a_size);                                                   // Set up the list
}

//--------------------------------------------------------------------------------
//        Destructor
//--------------------------------------------------------------------------------
template<struct T> list_p<T>::~list_p()
    { free(m_data); }

//--------------------------------------------------------------------------------
//        Copy constructor
//--------------------------------------------------------------------------------
template<struct T> list_p<T>::list_p(const list_p& other)
{
    init(other.m_arraySize);                                        // Initialise m_data
    for (list_p<T>::const_iterator it = other.begin(); it != other.end(); ++it)
        push_back(*it);
}

//--------------------------------------------------------------------------------
//        Assignment
//--------------------------------------------------------------------------------
template<struct T> list_p<T>& list_p<T>::operator = (const list_p& other)
{
    if (this == &other)
        return *this;
    resize(other.m_arraySize);                                      // resize array
    for (list_p<T>::const_iterator it = other.begin();; it != other.end(); ++it)
        push_back(*it);
    return *this;
}

//--------------------------------------------------------------------------------
//        Clear the list, retains allocated memory.
//--------------------------------------------------------------------------------
template<struct T> void list_p<T>::clear()
{
    m_nextFree = &m_data[0];                                        // Reset next free
    m_rootContainer.next = &m_endContainer;                         // Set outer container pointers
    m_endContainer.previous = &m_rootContainer;
    m_data[m_arraySize - 1].next = 0;                               // close the last element in the list
    for (size_t i = 0; i < m_arraySize - 1; i++)                    // Assign pointers
        m_data[i].next = &m_data[i+1];
    m_currentSize = 0;
}

//--------------------------------------------------------------------------------
//        Resize the list, wipes all m_data.
//--------------------------------------------------------------------------------
template<struct T> void list_p<T>::resize(size_t a_newSize)
{
    assert(a_newSize > 0);                                          // Size must be at least 1
    init(new_size);                                                 // Initialise m_data
}

//--------------------------------------------------------------------------------
//        Add an element to the back of the list
//--------------------------------------------------------------------------------
template<struct T> void list_p<T>::push_back(const T& a_item)
{
    if (m_currentSize == m_arraySize)                               // Is the list full?
        extend();
    DataContainer *new_element = m_nextFree;                        // Get the list node to work on
    m_nextFree = m_nextFree->next;                                  // Move m_nextFree pointer to the next DataContainer
    memcpy(new_element->element, &a_item, sizeof(T));               // Assign the element
    m_endContainer.previous->next = new_element;                    // Add the current element to the back of the active list
    new_element->previous = m_endContainer.previous;
    new_element->next = &m_endContainer;
    m_endContainer.previous = new_element;
    m_currentSize++;                                                // Increment m_currentSize
}

//--------------------------------------------------------------------------------
// Add an element to the back of the list, but does not assign, nor resize the array.
//--------------------------------------------------------------------------------
template<struct T> bool list_p<T>::push_back()
{
    if (m_currentSize == m_arraySize)                               // Is the list full?
        return false;
    DataContainer *new_element = m_nextFree;                        // Get the list node to work on
    m_nextFree = m_nextFree->next;                                  // Move m_nextFree pointer to the next DataContainer
    m_endContainer.previous->next = new_element;                    // Add the current element to the back of the active list
    new_element->previous = m_endContainer.previous;
    new_element->next = &m_endContainer;
    m_endContainer.previous = new_element;
    m_currentSize++;                                                // Increment m_currentSize
    return true;
}

//--------------------------------------------------------------------------------
// Add an element to the front of the list, but does not assign, nor resize the array.
//--------------------------------------------------------------------------------
template<struct T> bool list_p<T>::push_front()
{
    if (m_currentSize == m_arraySize)                               // Is the list full?
        return false;
    DataContainer *new_element = m_nextFree;                        // Get the list node to work on
    m_nextFree = m_nextFree->next;                                  // Move m_nextFree pointer to the next DataContainer
    m_rootContainer.next->previous = new_element;                   // Add the current element to the back of the active list
    new_element->previous = &m_rootContainer;
    new_element->next = m_rootContainer.next;
    m_rootContainer.next = new_element;
    m_currentSize++;                                                // Increment m_currentSize
    return true;
}

//--------------------------------------------------------------------------------
//        Add an element to the front of the list
//--------------------------------------------------------------------------------
template<struct T> void list_p<T>::push_front(const T& val)
{
    if (m_currentSize == m_arraySize)                               // Is the list full?
        extend();
    DataContainer *new_element = m_nextFree;                        // Get the list node to work on
    m_nextFree = m_nextFree->next;                                  // Move m_nextFree pointer to the next DataContainer
    memcpy(new_element->element, &a_item, sizeof(T));               // Assign the element
    m_rootContainer.next->previous = new_element;                   // Add the current element to the back of the active list
    new_element->previous = &m_rootContainer;
    new_element->next = m_rootContainer.next;
    m_rootContainer.next = new_element;
    m_currentSize++;                                                // Increment m_currentSize
}

//--------------------------------------------------------------------------------
//        Erase last element
//--------------------------------------------------------------------------------
template<struct T> void list_p<T>::pop_back()
{
    assert(m_currentSize != 0);                                     // Range check
    DataContainer* last = m_endContainer.previous->previous;        // Get new last element
    m_endContainer.previous->next = m_nextFree;                     // Assign next free
    m_nextFree = m_endContainer.previous;
    last->next = &m_endContainer;                                   // Extract element from chain, prev points to next
    m_endContainer.previous = last;                                 // next points to previous
    m_currentSize--;                                                // Decrement m_currentSize
}

//--------------------------------------------------------------------------------
//        Erase first element
//--------------------------------------------------------------------------------
template<struct T> void list_p<T>::pop_front()
{  
    assert(m_currentSize != 0);                                     // Range check
    DataContainer* first = m_rootContainer.next->next;              // Get new first element
    m_rootContainer.next->next = m_nextFree;                        // Assign next free
    m_nextFree = m_rootContainer.next;
    first->previous = &m_rootContainer;                             // extract element from chain, prev points to next
    m_rootContainer.next = first;                                   // next points to previous
    m_currentSize--;                                                // Decrement m_currentSize
}

//--------------------------------------------------------------------------------
//        Add an element to the list at position. 
//--------------------------------------------------------------------------------
template<struct T> void list_p<T>::insert(iterator const & it, const T& a_item)
{
    assert(it.ptr != &m_rootContainer);
    if (m_currentSize == m_arraySize)                               // Is the list full?
        extend();

    DataContainer *new_element = m_nextFree;                        // Get the list node to work on
    m_nextFree = m_nextFree->next;                                  // Move m_nextFree pointer to the next DataContainer
    it.ptr->previous->next = new_element;                           // Insert next free
    new_element->previous = it.ptr->previous;
    it.ptr->previous = new_element;
    new_element->next = it.ptr;
    memcpy(new_element->element, &a_item, sizeof(T));               // Set the element
    m_currentSize++;                                                // Increment m_currentSize
}

//--------------------------------------------------------------------------------
//        Erase an element from the list
//--------------------------------------------------------------------------------
template<struct T> void list_p<T>::erase(iterator& it)
{  
    DataContainer* next = it.ptr->next;                             // remember previous element    
    it.ptr->previous->next = it.ptr->next;                          // extract element from chain, prev points to next
    it.ptr->next->previous = it.ptr->previous;                      // next points to previous
    it.ptr->next = m_nextFree;                                      // add this broken item to the begining of the free list
    m_nextFree = it.ptr;                                            // put item in between m_nextFree and end of the list and reset m_nextFree
    m_currentSize--;                                                // decrement m_currentSize      
    it = iterator(next);                                            // return iterator to the next container
}

//--------------------------------------------------------------------------------
//        Increases the size of the underlying arrays by a factor of 1.5
//--------------------------------------------------------------------------------
template<struct T> void list_p<T>::extend()
{
    size_t new_size = m_arraySize << 1;                             // Calculate new size

    if (new_size < m_arraySize)
        throw std::overflow_error("m_arraySize");

    DataContainer* new_data = static_cast<DataContainer *>(malloc(new_size * sizeof(DataContainer)));

    if (new_data == nullptr)
        throw std::bad_alloc;

    for (size_t i = 0; i < new_size - 1; i++)                       // Assign pointers
        new_data[i].next = &new_data[i + 1];
    for (size_t i = 1; i < m_currentSize; i++)
        new_data[i].previous = &new_data[i - 1];
    
    iterator it = begin();                                          // Assign values
    for (size_t i = 0; it != end(); ++it, ++i)
        new_data[i].element = *it;

    free(m_data);                                                   // Assign m_data pointer
    m_data = new_data;
    m_nextFree = &new_data[m_currentSize];                          // Assign next free pointer
    m_arraySize = new_size;                                         // Adjust sizes

    if (m_currentSize == 0)                                         // Determine root and end pointers
    {
        m_rootContainer.next = &m_endContainer;
        m_endContainer.previous = &m_rootContainer;
    }
    else
    {
        m_rootContainer.next = &m_data[0];
        m_endContainer.previous = &m_data[m_currentSize - 1];
        new_data[0].previous = &m_rootContainer;
        new_data[m_currentSize - 1].next = &m_endContainer;
    }
}

//--------------------------------------------------------------------------------
//        Helpful functions
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
//        Find a value in the list, returns iterator
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::iterator find (typename list_p<T>::iterator first, typename list_p<T>::iterator last, const T& val)
{
    while (first!=last) 
    {
        if (*first == val) return first;
        ++first;
    }
    return last;
}

//--------------------------------------------------------------------------------
//        Find a value in the list, returns const_iterator
//--------------------------------------------------------------------------------
template<struct T> typename list_p<T>::const_iterator find (typename list_p<T>::const_iterator first, typename list_p<T>::const_iterator last, const T& val)
{
    while (first != last) 
    {
        if (*first == val) return first;
        ++first;
    }
    return last;
}

};

#endif