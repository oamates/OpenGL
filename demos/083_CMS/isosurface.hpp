#ifndef _cms_isosurface_included_8910256243875680146357816347561387459184375678
#define _cms_isosurface_included_8910256243875680146357816347561387459184375678

namespace cms
{

struct Isosurface
{
    Isosurface() {}

    virtual ~Isosurface() {};
    virtual float operator()(float x, float y, float z) const = 0;
};


// The templated isosurface class, which should be used to extend Isosurface

template<typename Function>
struct Isosurface_t : public Isosurface
{
    const Function* m_fn;

    // The default construtor - sets function to NULL (should be set)
    Isosurface_t()
        : m_fn(0) 
    {}

    // The constructor taking in a Function
    Isosurface_t(const Function* i_fn)
        : m_fn(i_fn) 
    {}

    // The overloaded operator used for sampling the function
    float operator()(float x, float y, float z) const
        { return (*m_fn)(x, y, z); }
};

} // namespace cms

#endif // _cms_isosurface_included_8910256243875680146357816347561387459184375678
