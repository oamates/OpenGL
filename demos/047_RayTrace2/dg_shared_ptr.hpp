#ifndef DG_SHARED_PTR
#define DG_SHARED_PTR

namespace Dg
{

struct RefCounter
{
    unsigned int count;

    RefCounter() : count(0) {}

    void AddRef()
        { count++; }
    int Release()
        { return --count; }
};

template <typename T> struct shared_ptr
{
    T* pData;
    RefCounter* reference;

    shared_ptr() : pData(0), reference()
    {
        reference = new RefCounter();
        reference->AddRef();
    }

    shared_ptr(T* pValue) : pData(pValue), reference()
    {
        reference = new RefCounter();
        reference->AddRef();
    }

    shared_ptr(const shared_ptr<T>& sp) : pData(sp.pData), reference(sp.reference)
        { reference->AddRef(); }

    ~shared_ptr()
    {
        if (reference->Release() == 0)
        {
            delete pData;
            delete reference;
        }
    }

    T& operator * ()
        { return *pData; }

    T* operator -> ()
        { return pData; }

    shared_ptr<T>& operator = (const shared_ptr<T>& sp)
    {
        if (this != &sp)
        {
            if (reference->Release() == 0)  // decrement the old reference count if reference become zero delete the old data
            {
                delete pData;
                delete reference;
            }
            // Copy the data and reference pointer and increment the reference count
            pData = sp.pData;
            reference = sp.reference;
            reference->AddRef();
        }
        return *this;
    }
};

}
#endif