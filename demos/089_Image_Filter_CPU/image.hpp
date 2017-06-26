#ifndef _image_included_0918426510235610235641723506178346501765415786328957164
#define _image_included_0918426510235610235641723506178346501765415786328957164

#include "nvimage.h"
//#include "nvcore/Debug.h"

namespace nv
{
    class Color32;

    //===================================================================================================================================================================================================================
    // 32 bit RGBA image.
    //===================================================================================================================================================================================================================
    struct image_t
    {
        unsigned int m_width;
        unsigned int m_height;
        unsigned int m_depth;
        Format m_format;
        Color32 * m_data;

        enum Format 
        {
            Format_RGB,
            Format_ARGB,
        };

        image_t();
        image_t(const image_t& img);
        ~image_t();

        const image_t& operator = (const image_t& img);


        void allocate(unsigned int w, unsigned int h, unsigned int d = 1);
        bool load(const char * name);

        void resize(unsigned int w, unsigned int h, unsigned int d = 1);

        void wrap(void * data, unsigned int w, unsigned int h, unsigned int d = 1);
        void unwrap();

        unsigned int width() const;
        unsigned int height() const;
        unsigned int depth() const;

        const Color32* scanline(unsigned int h) const;
        Color32* scanline(unsigned int h);

        const Color32 * pixels() const;
        Color32* pixels();

        const Color32& pixel(unsigned int idx) const;
        Color32& pixel(unsigned int idx);

        const Color32& pixel(unsigned int x, unsigned int y, unsigned int z = 0) const;
        Color32& pixel(unsigned int x, unsigned int y,  unsigned int z = 0);

        Format format() const;
        void setFormat(Format f);

        void fill(Color32 c);

        void free();

    };


    inline const Color32& Image::pixel(unsigned int x, unsigned int y, unsigned int z) const
    {
        return pixel((z * m_height + y) * m_width + x);
    }

    inline Color32& Image::pixel(unsigned int x, unsigned int y, unsigned int z)
    {
        return pixel((z * m_height + y) * m_width + x);
    }

} // nv namespace


#endif // _image_included_0918426510235610235641723506178346501765415786328957164
