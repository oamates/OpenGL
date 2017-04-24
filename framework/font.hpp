#ifndef _font_included_81041351051207410824772645214816087092198491297741823430
#define _font_included_81041351051207410824772645214816087092198491297741823430

#include "image.hpp"
#include "vao.hpp"
#include "vertex.hpp"
#include "log.hpp"

namespace text
{
    struct glyph_t
    {
        int id;
        int x, y;
        int width, height;
        float xoffset, yoffset, xadvance;
    };

    struct font_desc_t
    {
        int size;
        const text::glyph_t* glyphs;
        int chars;
        const char* texture_name;
        int texture_size;
    };

    struct font_t
    {
        font_desc_t desc;
        GLuint texture_id;
        float texel_size;

        font_t() {};
        font_t(const font_desc_t& font_desc);
        void init(const font_desc_t& font_desc);
        int char_index(int id);
        vbo_t create_vbo(const char* text, const glm::vec2& origin, const glm::vec2& scale);
        void render_text(const char* text, const glm::vec2& origin, const glm::vec2& scale);
        ~font_t();
    };

} // namespace text

#endif // _font_included_81041351051207410824772645214816087092198491297741823430