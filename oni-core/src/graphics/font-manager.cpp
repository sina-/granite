#include <graphics/font-manager.h>
#include <utils/oni-assert.h>
#include <graphics/texture.h>

namespace oni {
    namespace graphics {

        FontManager::FontManager(std::string font, int size)
//                m_FTAtlas(, ftgl::texture_atlas_delete),
//                m_FTFont(ftgl::texture_font_new_from_file(m_FTAtlas.get(), 10, "resources/fonts/Vera.ttf"),
//                         ftgl::texture_font_delete) {
        {

            m_FTAtlas = ftgl::texture_atlas_new(512, 512, 1);
            m_FTFont = ftgl::texture_font_new_from_file(m_FTAtlas, size, font.c_str());

            std::string cache = " !\"#$%&'()*+,-./0123456789:;<=>?@"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

            auto glyph = ftgl::texture_font_load_glyphs(m_FTFont, cache.c_str());

            ONI_DEBUG_ASSERT(glyph == 0);

            m_FTAtlas->id = graphics::LoadTexture::load(*this);

        }

        FontManager::~FontManager() {
            ftgl::texture_atlas_delete(m_FTAtlas);
            ftgl::texture_font_delete(m_FTFont);
        }

        const ftgl::texture_glyph_t *FontManager::findGlyph(const char &character) const {
            auto glyph = ftgl::texture_font_find_glyph(m_FTFont, &character);

            // Make sure the character is pre-loaded and valid.
            ONI_DEBUG_ASSERT(glyph);
            return glyph;
        }

        size_t FontManager::getAtlasWidth() const {
            return m_FTAtlas->width;
        }

        size_t FontManager::getAtlasHeight() const {
            return m_FTAtlas->height;
        }

        unsigned char * FontManager::getAtlasData() const {
            return m_FTAtlas->data;
        }

        GLuint FontManager::getTextureID() const { return m_FTAtlas->id; }
    }
}