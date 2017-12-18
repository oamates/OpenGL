#include <glm/gtx/string_cast.hpp>

#include "font.hpp"
#include "image.hpp"
#include "vertex.hpp"
#include "log.hpp"

namespace text
{

    font_t::font_t(const font_desc_t& font_desc)
        { init(font_desc); }

    void font_t::init(const font_desc_t& font_desc)
    {
        desc = font_desc;
        texel_size = 1.0f / desc.texture_size;
        texture_id = image::png::texture2d(desc.texture_name, 0, GL_LINEAR, GL_LINEAR);
    };


    int font_t::char_index(int id)
    {
        int f = 0;
        int l = desc.chars - 1;
        int m = (f + l) / 2;

        while (f <= l)
        {
            if (desc.glyphs[m].id == id) return m;
            if (desc.glyphs[m].id < id)
                f = m + 1;
            else
                l = m - 1;

            m = (f + l) / 2;
        }
        return -1;
    }

    vbo_t font_t::create_vbo(const char* text, const glm::vec2& origin, const glm::vec2& scale)
    {
        std::vector<vertex_p2t2_t> text_data;
        glm::vec2 position = origin;
        int l = 0;

        const float text_size = 0.145f;

        for(const char* q = text; *q; ++q)
        {
            if (*q == '\n')
            {
                ++l;
                position = glm::vec2(origin.x, origin.y - text_size * l * scale.y);
                continue;
            }

            int c = char_index((int) *q);
            if (c == -1) continue;
            const glyph_t& glyph = desc.glyphs[c];

            glm::vec2 pmin = position + glm::vec2(glyph.xoffset, glyph.yoffset - glyph.height) * scale * texel_size;
            glm::vec2 pmax = pmin + glm::vec2(glyph.width, glyph.height) * scale * texel_size;
            glm::vec2 tmin = glm::vec2(glyph.x + 0.5f, desc.texture_size - glyph.height - glyph.y - 0.5f) * texel_size;
            glm::vec2 tmax = glm::vec2(glyph.x + glyph.width + 0.5f, desc.texture_size - glyph.y - 0.5f) * texel_size;

            text_data.push_back(vertex_p2t2_t(glm::vec2(pmin.x, pmin.y), glm::vec2(tmin.x, tmin.y)));
            text_data.push_back(vertex_p2t2_t(glm::vec2(pmax.x, pmin.y), glm::vec2(tmax.x, tmin.y)));
            text_data.push_back(vertex_p2t2_t(glm::vec2(pmax.x, pmax.y), glm::vec2(tmax.x, tmax.y)));
            text_data.push_back(vertex_p2t2_t(glm::vec2(pmin.x, pmin.y), glm::vec2(tmin.x, tmin.y)));
            text_data.push_back(vertex_p2t2_t(glm::vec2(pmax.x, pmax.y), glm::vec2(tmax.x, tmax.y)));
            text_data.push_back(vertex_p2t2_t(glm::vec2(pmin.x, pmax.y), glm::vec2(tmin.x, tmax.y)));

            position.x += glyph.xadvance * scale.x * texel_size;

        }

        return vbo_t(text_data);
    }

    void font_t::render_text(const char* text, const glm::vec2& origin, const glm::vec2& scale)
    {
        size_t size = 0;

        for(const char* q = text; *q; ++q)
        {
            if (*q == '\n') continue;
            int index = char_index((int) *q);
            if (index == -1) continue;
            size++;
        }

        GLuint vao_id;
        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);

        vbo_t vbo = vbo_t((vertex_p2t2_t*) 0, 6 * size);
        vertex_p2t2_t* text_data = (vertex_p2t2_t*) vbo_t::map();

        glm::vec2 position = origin;
        const float text_size = 0.145f;
        int l = 0;

        int index = 0;
        for(const char* q = text; *q; ++q)
        {
            if (*q == '\n')
            {
                ++l;
                position = glm::vec2(origin.x, origin.y - text_size * l * scale.y);
                continue;
            }

            int c = char_index((int) *q);
            if (c == -1) continue;
            const glyph_t& glyph = desc.glyphs[c];

            glm::vec2 pmin = position + glm::vec2(glyph.xoffset, glyph.yoffset - glyph.height) * scale * texel_size;
            glm::vec2 pmax = pmin + glm::vec2(glyph.width, glyph.height) * scale * texel_size;
            glm::vec2 tmin = glm::vec2(glyph.x, desc.texture_size - glyph.height - glyph.y) * texel_size;
            glm::vec2 tmax = glm::vec2(glyph.x + glyph.width, desc.texture_size - glyph.y) * texel_size;

            text_data[index++] = vertex_p2t2_t(glm::vec2(pmin.x, pmin.y), glm::vec2(tmin.x, tmin.y));
            text_data[index++] = vertex_p2t2_t(glm::vec2(pmax.x, pmin.y), glm::vec2(tmax.x, tmin.y));
            text_data[index++] = vertex_p2t2_t(glm::vec2(pmax.x, pmax.y), glm::vec2(tmax.x, tmax.y));
            text_data[index++] = vertex_p2t2_t(glm::vec2(pmin.x, pmin.y), glm::vec2(tmin.x, tmin.y));
            text_data[index++] = vertex_p2t2_t(glm::vec2(pmax.x, pmax.y), glm::vec2(tmax.x, tmax.y));
            text_data[index++] = vertex_p2t2_t(glm::vec2(pmin.x, pmax.y), glm::vec2(tmin.x, tmax.y));

            position.x += glyph.xadvance * scale.x * texel_size;

        }

        vbo_t::unmap();
        glDrawArrays(GL_TRIANGLES, 0, 6 * size);
        glDeleteVertexArrays(1, &vao_id);
    }

    font_t::~font_t()
        { glDeleteTextures(1, &texture_id); }

} // namespace text