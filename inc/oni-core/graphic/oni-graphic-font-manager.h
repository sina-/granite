#pragma once

#include <oni-core/math/oni-math-vec3.h>
#include <oni-core/common/oni-common-typedefs-graphic.h>

namespace ftgl {
    class texture_atlas_t;

    class texture_font_t;

    class texture_glyph_t;
}

namespace oni {
    namespace component {
        struct Text;
        union WorldP3D;
    }

    namespace entities {
        class EntityManager;

        class EntityFactory;
    }

    namespace graphic {
        class FontManager {
        public:
            FontManager(std::string font,
                        common::u8 size,
                        common::r32 gameWidth,
                        common::r32 gameHeight);

            ~FontManager();

            FontManager(const FontManager &) = delete;

            FontManager &
            operator=(FontManager &) = delete;

            common::EntityID
            createTextFromString(entities::EntityFactory &entityFactory,
                                 const std::string &text,
                                 const component::WorldP3D &position);

            void
            updateText(const std::string &textContent,
                       component::Text &text);

            size_t
            getAtlasWidth() const;

            size_t
            getAtlasHeight() const;

            unsigned char *
            getAtlasData() const;

            common::oniGLuint
            getTextureID() const;

        private:
            const ftgl::texture_glyph_t *
            findGlyph(const char &character) const;

            void
            assignGlyphs(component::Text &);

        private:
            // Wrap atlas and font with unique_ptr and pass the custom deleter
//            std::unique_ptr<ftgl::texture_font_t, decltype(&ftgl::texture_font_delete)> m_FTFont;
//            std::unique_ptr<ftgl::texture_atlas_t, decltype(&ftgl::texture_atlas_delete)> m_FTAtlas;
            ftgl::texture_atlas_t *m_FTAtlas;
            ftgl::texture_font_t *m_FTFont;

            common::r32 mGameWidth;
            common::r32 mGameHeight;

        };
    }
}