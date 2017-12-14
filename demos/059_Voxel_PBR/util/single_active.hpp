#pragma once
#include <memory>

// Template class for set up a single instance of certain class as active, differentiating it from the rest.
template<class T> struct SingleActive
{
    static void ResetActive();
    virtual void SetAsActive();                                 // Sets this instance as active.
    bool IsActive();                                            // Return whether this instance is the one marked as active.
    static std::unique_ptr<T> &Active();                        // Returns the active instance
    static std::unique_ptr<T> current;                          // The current instance marked as active
    ~SingleActive();
};

template<class T> std::unique_ptr<T> SingleActive<T>::current = nullptr;

template<class T> SingleActive<T>::~SingleActive()
{    
    if (static_cast<T *>(this) == current.get())                // release ownership but don't delete
        current.release();
}

template <class T> void SingleActive<T>::ResetActive()
{
    current.release();
    current.reset(static_cast<T*>(nullptr));
}

template<class T> void SingleActive<T>::SetAsActive()
{
    current.release();
    current.reset(static_cast<T*>(this));
}

template <class T> bool SingleActive<T>::IsActive()
    { return current.get() == static_cast<T *>(this); }

template<class T> std::unique_ptr<T> &SingleActive<T>::Active()
    { return current; }
