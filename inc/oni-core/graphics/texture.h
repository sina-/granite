#pragma once

#include <string>

#include <GL/glew.h>
#include <ftgl/texture-atlas.h>

#include <oni-core/entities/basic-entity-repo.h>
#include <oni-core/graphics/font-manager.h>

namespace oni {
    namespace graphics {
        class Texture {
            Texture() = default;

            ~Texture() = default;

        public:
            // TODO: At some point would be nice to have a list of all loaded textures and only load a new
            // one if it has not been loaded already. Perhaps its easier to have a texture manager just like
            // font manager or audio manager.
            static components::Texture load(const std::string &path);

            static components::Texture generate(const int width, const int height, const components::PixelRGBA &pixel);

            static std::vector<unsigned char> generateBits(const int width, const int height,
                                                           const components::PixelRGBA &pixel);

            static void updateSubTexture(components::Texture texture, const GLint xOffset,
                                         const GLint yOffset, const GLint width,
                                         const GLint height,
                                         const std::vector<unsigned char> &bits);

            static GLuint load(const graphics::FontManager &fontManager);

            static void bind(GLuint textureID);

            static void unbind();
        };
    }
}