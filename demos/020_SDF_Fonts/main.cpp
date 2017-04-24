#include <cmath>
#include <ctime>
#include <cstdio>
#include <vector>
#include <cctype> 

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "font.hpp"
#include "binpacker.hpp"
#include "image.hpp"

unsigned char get_SDF_radial(unsigned char *fontmap, int w, int h, int x, int y, int max_radius);
bool gen_pack_list(FT_Face &ft_face, int pixel_size, int pack_tex_size, const std::vector<int>& render_list, std::vector<text::glyph_t>& packed_glyphs);

const int scaler = 16;                                                                                          // number of rendered pixels per SDF pixel (larger value means higher quality, up to a point)

void str_tolower(char* s)
{
    for (char* p = s; *p != '\0'; p++) 
        *p = (char) tolower(*p);
}


int main(int argc, char** argv)
{
    //===================================================================================================================================================================================================================
    // make sure font file name is provided
    //===================================================================================================================================================================================================================
    printf("\n\nPNG Signed Distance Field font creator ... \n");
    if (argc < 2)
    {
        printf("ERROR :: Font file not specified! Exiting ... \n");
        return -1;
    }
    const char* font_file = argv[1]; 

    //===================================================================================================================================================================================================================
    // initialize FreeType library and load the font file
    //===================================================================================================================================================================================================================
    FT_Library ft_lib;
    if (FT_Init_FreeType(&ft_lib))
    {
        printf("ERROR :: Failed to initialize the FreeType library!\n");
        return -1;
    }

    FT_Face ft_face;
    if (FT_New_Face(ft_lib, font_file, 0, &ft_face))
    {
        printf("ERROR :: Failed to read the font file [%s]'\n", font_file);
        return false;
    }

    //===================================================================================================================================================================================================================
    // initialize FreeType library and load the font file
    //===================================================================================================================================================================================================================
    int texture_size;
    printf("\n\nChoose texture size in range [64..4096] : " );
    scanf("%i", &texture_size);
    if (texture_size < 64)   texture_size = 64;
    if (texture_size > 4096) texture_size = 4096;
    
    printf("\n\nSelect the highest unicode character you wish to render : ");
    int max_unicode_char;
    scanf("%i", &max_unicode_char);
    if(max_unicode_char < 1) max_unicode_char = 1;
    
    std::vector<int> render_list;                                                                                                   // Try all characters up to a user selected value (it will auto-skip any without glyphs)
    for(int char_idx = 0; char_idx <= max_unicode_char; ++char_idx)
        render_list.push_back(char_idx);

    printf("\n\nReady to convert [%s] to a SDF.\n\nDetermining pixel size: ", font_file);

    //===================================================================================================================================================================================================================
    // the main job
    //===================================================================================================================================================================================================================
    std::vector<text::glyph_t> glyphs;                                                                                              // find the perfect size
    
    int sz = 4;                                                                                                                     // initial guess for the size of the Signed Distance Field font
    bool keep_going = true;
    while(keep_going)
    {
        sz <<= 1;
        printf(" %i", sz);
        keep_going = gen_pack_list(ft_face, sz, texture_size, render_list, glyphs);
    }
    int sz_step = sz >> 2;
    while(sz_step)
    {
        if (keep_going)
            sz += sz_step;
        else
            sz -= sz_step;
        printf(" %i", sz);
        sz_step >>= 1;
        keep_going = gen_pack_list(ft_face, sz, texture_size, render_list, glyphs);
    }
    
    while((!keep_going) && (sz > 1))                                                                                                // just in case
    {
        --sz;
        printf(" %i", sz);
        keep_going = gen_pack_list(ft_face, sz, texture_size, render_list, glyphs);
    }
    printf("\nResult = %i pixels\n", sz);

    if(!keep_going)
    {
        printf("The data will not fit in a texture %i x %i\n", texture_size, texture_size);
        return -1;
    }
    
    unsigned char* pixel_buffer = (unsigned char*) malloc(texture_size * texture_size);
    
    printf("\nRendering characters into a png image: %i x %i\n", texture_size, texture_size);                                       //  render all the glyphs individually
    int packed_glyph_index = 0;
    for(unsigned int char_index = 0; char_index < render_list.size(); ++char_index)
    {
        int glyph_index = FT_Get_Char_Index(ft_face, render_list[char_index]);
        if (glyph_index)
        {
            int ft_err = FT_Load_Glyph(ft_face, glyph_index, 0);
            if (!ft_err)
            {
                ft_err = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_MONO);
                if (!ft_err)
                {
                    int w = ft_face->glyph->bitmap.width;                                                                           // we have the glyph, already rendered, get the data about it
                    int h = ft_face->glyph->bitmap.rows;
                    int p = ft_face->glyph->bitmap.pitch;
                    
                    int sw = w + scaler * 4;                                                                                        // oversize the holding buffer so I can smooth it!
                    int sh = h + scaler * 4;
                    unsigned char* smooth_buf = new unsigned char[sw * sh];
                    for(int i = 0; i < sw * sh; ++i) smooth_buf[i] = 0;

                    unsigned char* buf = ft_face->glyph->bitmap.buffer;                                                             // copy the glyph into the buffer to be smoothed
                    for(int j = 0; j < h; ++j)
                        for(int i = 0; i < w; ++i)
                            smooth_buf[scaler * 2 + i + (j + scaler * 2) * sw] = 255 * ((buf[j * p + (i >> 3)] >> (7 - (i & 7))) & 1);
                    
                    int sdfw = glyphs[packed_glyph_index].width;                                                                    // do the SDF
                    int sdfx = glyphs[packed_glyph_index].x;
                    int sdfh = glyphs[packed_glyph_index].height;
                    int sdfy = glyphs[packed_glyph_index].y;
                    for(int j = 0; j < sdfh; ++j)
                        for(int i = 0; i < sdfw; ++i)
                        {
                            int pd_idx = i + sdfx + (j + sdfy) * texture_size;
                            pixel_buffer[pd_idx] = get_SDF_radial(smooth_buf, sw, sh, i * scaler + (scaler >> 1), j * scaler + (scaler >> 1), 2 * scaler);
                        }
                    ++packed_glyph_index;

                    delete[] smooth_buf;
                }
                printf("%i ", render_list[char_index]);
            }
        }
    }

    char file_name[128];
    str_tolower(ft_face->family_name);
    str_tolower(ft_face->style_name);

    //===================================================================================================================================================================================================================
    // save the png file
    //===================================================================================================================================================================================================================
    sprintf(file_name, "%s_%s.png", ft_face->family_name, ft_face->style_name);
    printf("\n\nRendering done.\n\nSaving image to PNG file ... [%s] \n", file_name);
    image::png::write(file_name, texture_size, texture_size, pixel_buffer, PNG_COLOR_TYPE_GRAY);    

    //===================================================================================================================================================================================================================
    // save glyph description into a text file
    //===================================================================================================================================================================================================================
    FILE* fp;
    sprintf(file_name, "%s_%s.txt", ft_face->family_name, ft_face->style_name);
    printf("\n\nSaving glyph description to a text file ... [%s] \n", file_name);
    if (fp = fopen(file_name, "w"))
    {
        int size = glyphs.size();
        fprintf(fp, "Face family name = [%s]\nFace style name = [%s]\nchars count = %i\n", ft_face->family_name, ft_face->style_name, size);
        for(int i = 0; i < size; ++i)
        {
            fprintf(fp, "char id = %-6ix = %-6iy = %-6iw = %-6ih = %-6ixoffset = %-10.3fyoffset = %-10.3fxadvance = %-10.3f\n", 
                    glyphs[i].id, glyphs[i].x, glyphs[i].y, glyphs[i].width, glyphs[i].height, glyphs[i].xoffset, glyphs[i].yoffset, glyphs[i].xadvance);
        }
        fclose(fp);
    }

    //===================================================================================================================================================================================================================
    // save glyph description into a binary file
    //===================================================================================================================================================================================================================
    sprintf(file_name, "%s_%s.sdf", ft_face->family_name, ft_face->style_name);
    if (fp = fopen(file_name, "wb"))
    {
        fwrite(glyphs.data(), sizeof(text::glyph_t), glyphs.size(), fp);
        fclose(fp);
    }

    //===================================================================================================================================================================================================================
    // save a C/C++ header file
    //===================================================================================================================================================================================================================
    sprintf(file_name, "%s_%s.hpp", ft_face->family_name, ft_face->style_name);
    printf("\n\nSaving the SDF data in a C header file ... [%s]\n", file_name);
    if (fp = fopen(file_name, "w"))
    {
        fprintf(fp, "#ifndef _%s_%s_included_\n#define _%s_%s_included_\n\nnamespace text {\n\nnamespace %s {\n\nnamespace %s {\n\n"
                    "\tconst int font_size = %i;\n\tconst int chars = %i;\n\n\tconst int texture_size = %i;\n\tconst char* texture_name = \"/fonts/%s_%s.png\";\n\n"
                    "\tconst text::glyph_t glyphs[] = \n\t{\n", 
                    ft_face->family_name, ft_face->style_name, ft_face->family_name, ft_face->style_name, ft_face->family_name, ft_face->style_name, 
                    sz, (unsigned int) glyphs.size(), texture_size, ft_face->family_name, ft_face->style_name);

        for(int i = 0; i < glyphs.size() - 1; ++i)
            fprintf(fp, "\t\t{%i, %i, %i, %i, %i, %.6f, %.6f, %.6f},\n", glyphs[i].id, glyphs[i].x, glyphs[i].y, glyphs[i].width, glyphs[i].height,
                        glyphs[i].xoffset, glyphs[i].yoffset, glyphs[i].xadvance);
        const text::glyph_t& glyph = glyphs.back();
        fprintf(fp, "\t\t{%i, %i, %i, %i, %i, %.6f, %.6f, %.6f}\n\t};\n\n} //namespace %s\n\n} //namespace %s\n\n} //namespace text\n\n#endif // _%s_%s_included_\n", 
                    glyph.id, glyph.x, glyph.y, glyph.width, glyph.height, glyph.xoffset, glyph.yoffset, glyph.xadvance, ft_face->family_name, ft_face->style_name, ft_face->family_name, ft_face->style_name);
        fclose(fp);
    }

    free(pixel_buffer);
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_lib);
    return 0;
}

bool gen_pack_list(FT_Face& ft_face, int pixel_size, int pack_tex_size, const std::vector<int>& render_list, std::vector<text::glyph_t>& glyphs)
{
    glyphs.clear();
    int ft_err = FT_Set_Pixel_Sizes(ft_face, pixel_size * scaler, 0);
    std::vector<int> rectangle_info;
    std::vector<std::vector<int>> packed_glyph_info;
    for(unsigned int char_index = 0; char_index < render_list.size(); ++char_index)
    {
        int glyph_index = FT_Get_Char_Index(ft_face, render_list[char_index]);
        if (glyph_index)
        {
            ft_err = FT_Load_Glyph(ft_face, glyph_index, 0);
            if (!ft_err)
            {
                ft_err = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_MONO);
                if (!ft_err)
                {
                    text::glyph_t glyph;

                    int w = ft_face->glyph->bitmap.width;                                                                   // we have a glyph, already rendered, get the data about it
                    int h = ft_face->glyph->bitmap.rows;
                    int sw = w + scaler * 4;                                                                                // oversize the holding buffer so it can be smoothed it!
                    int sh = h + scaler * 4;
                    int sdfw = sw / scaler;                                                                                 // do the SDF
                    int sdfh = sh / scaler;
                    rectangle_info.push_back(sdfw);
                    rectangle_info.push_back(sdfh);
                    glyph.id = render_list[char_index];                                                                     // add in the data we already know
                    glyph.width = sdfw;
                    glyph.height = sdfh;
                    glyph.x = -1;                                                                                           // these need to be filled in later (after packing)
                    glyph.y = -1;
                    glyph.xoffset = ft_face->glyph->bitmap_left / scaler - 1.5;                                             // so scale them (the 1.5's have to do with the padding
                    glyph.yoffset = ft_face->glyph->bitmap_top / scaler + 1.5;                                              // border and the sampling locations for the SDF)         
                    glyph.xadvance = 0.015625 * ft_face->glyph->advance.x / scaler;                                         // 0.015625 = 1 / 64
                    glyphs.push_back(glyph);                                                                                // add it to the list
                }
            }
        }
    }

    bin_packer_t bin_packer;
    bin_packer.pack(rectangle_info, packed_glyph_info, pack_tex_size);
    
    if (packed_glyph_info.size() != 1) return false;                                                                        // populate the actual coordinates

    for(unsigned int i = 0; i < packed_glyph_info[0].size(); i += 3)                                                        // it all fit into one!
    {
        unsigned int idx = packed_glyph_info[0][i + 0];                                                                     // index, x, y
        glyphs[idx].x = packed_glyph_info[0][i + 1];
        glyphs[idx].y = packed_glyph_info[0][i + 2];
    }
    return true;
}

unsigned char get_SDF_radial(unsigned char* fontmap, int w, int h, int x, int y, int max_radius)
{    
    float d2 = max_radius * max_radius + 1.0;                                                                               // brute force method
    unsigned char v = fontmap[x + y * w];
    for(int radius = 1; (radius <= max_radius) && (radius * radius < d2); ++radius)
    {
        int line, lo, hi;
        
        line = y - radius;                                                                                                  // north
        if ((line >= 0) && (line < h))
        {
            lo = x - radius;
            hi = x + radius;
            if (lo < 0) lo = 0;
            if (hi >= w) hi = w - 1;
            int idx = line * w + lo;
            for(int i = lo; i <= hi; ++i)
            {
                if (fontmap[idx] != v)                                                                                      // check this pixel
                {
                    float nx = i - x;
                    float ny = line - y;
                    float nd2 = nx * nx + ny * ny;
                    if (nd2 < d2) d2 = nd2;
                }
                ++idx;                                                                                                      // move on
            }
        }
        
        line = y + radius;                                                                                                  // south
        if ((line >= 0) && (line < h))
        {
            lo = x - radius;
            hi = x + radius;
            if (lo < 0) lo = 0;
            if (hi >= w) hi = w - 1;
            int idx = line * w + lo;
            for(int i = lo; i <= hi; ++i)
            {
                if (fontmap[idx] != v)                                                                                      // check this pixel
                {
                    float nx = i - x;
                    float ny = line - y;
                    float nd2 = nx * nx + ny * ny;
                    if (nd2 < d2) d2 = nd2;
                }
                ++idx;                                                                                                      // move on
            }
        }
        
        line = x - radius;                                                                                                  // west
        if ((line >= 0) && (line < w))
        {
            lo = y - radius + 1;
            hi = y + radius - 1;
            if (lo < 0) lo = 0;
            if (hi >= h) hi = h - 1;
            int idx = lo * w + line;
            for(int i = lo; i <= hi; ++i)
            {
                if (fontmap[idx] != v)                                                                                      // check this pixel
                {
                    float nx = line - x;
                    float ny = i - y;
                    float nd2 = nx * nx + ny * ny;
                    if (nd2 < d2) d2 = nd2;
                }
                idx += w;                                                                                                   // move on
            }
        }
        
        line = x + radius;                                                                                                  // east
        if ((line >= 0) && (line < w))
        {
            lo = y - radius + 1;
            hi = y + radius - 1;
            if (lo < 0) lo = 0;
            if (hi >= h) hi = h - 1;
            int idx = lo * w + line;
            for(int i = lo; i <= hi; ++i)
            {
                if (fontmap[idx] != v)                                                                                      // check this pixel
                {
                    float nx = line - x;
                    float ny = i - y;
                    float nd2 = nx * nx + ny * ny;
                    if (nd2 < d2) d2 = nd2;
                }
                idx += w;                                                                                                   //  move on
            }
        }
    }
    d2 = sqrtf(d2);
    if (v == 0) d2 = -d2;
    d2 *= 127.5 / max_radius;
    d2 += 127.5;
    if(d2 < 0.0) d2 = 0.0;
    if(d2 > 255.0) d2 = 255.0;
    return (unsigned char)(d2 + 0.5);
}
