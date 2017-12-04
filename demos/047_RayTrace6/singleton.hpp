#ifndef SINGLETON
#define SINGLETON

#include <iostream>

template <typename T> struct singleton_t
{
    singleton_t () { }

    ~singleton_t ()
        { Destroy(); }

    static T* Instance_Ptr ()
    {
        if (!FInstance)
            FInstance = new T;
        return (static_cast<T*> (FInstance));
    }
    static T& Instance ()
    {
        if (!FInstance)
            FInstance = new T;
        return *(static_cast<T*> (FInstance));
    }

    static void Destroy ()
    {
        if (FInstance)
        {
            delete FInstance;
            FInstance = 0;
        }
    }

    static T* FInstance;
};

template <typename T> T* singleton_t<T>::FInstance = 0;

#endif