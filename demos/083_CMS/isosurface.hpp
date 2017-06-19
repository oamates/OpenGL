#ifndef _cms_isosurface_included_8910256243875680146357816347561387459184375678
#define _cms_isosurface_included_8910256243875680146357816347561387459184375678

namespace cms
{

struct Isosurface
{
    // The iso-level (iso-value) of the surface (zero by def)
    float iso_level;

    // A variable defining the inside (and therefore outside)
    // signs of the isosurface. Allowing for changes to be made, to the functions signs negative inside by default
    bool negative_inside;
    bool loaded;

    // The default construtor - setting isolevel value to 0
    // class cannot be instantiated directly as this is a protected ctor
    Isosurface() : iso_level(0.0f), loaded(false) {}

    // The virtual destructor of the the Isosurface
    virtual ~Isosurface() {};


    
    virtual float operator()(float x, float y, float z) const = 0;              // The pure virtual overloaded operator used for sampling the function implementation is in the template derived class Isosurface_t

    // Set the isolevel of the surface
//    void setIsolevel(float i_isoLevel)
//        { m_isoLevel = i_isoLevel; }

    // Get a read-only reference to the isolevel value
//    const float& getIsolevel() const
//        { return m_isoLevel; }

    // sets the signs of the mesh based on the boolean value supplied
    // if true - negative inside and positive outside; if false - vice-versa
//    void setNegativeInside(bool _negInside)
//        { m_negativeInside = _negInside; }

    // Returns a boolean of whether the sign is negative inside the surface
    // If returns false - It is positive inside and negative outside
//    bool isNegativeInside() const
//        { return m_negativeInside; }

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

    // Sets the member function to the one supplied
    void setFunction(const Function* i_fn)
        { m_fn = i_fn; }

    // Returns the base function
    float getFuntion() const 
        { return m_fn; }
};

} // namespace cms

#endif // _cms_isosurface_included_8910256243875680146357816347561387459184375678
