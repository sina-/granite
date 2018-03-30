#include <oni-core/graphics/light.h>

namespace oni {
    namespace graphics {

        void Light::tick(Layer &layer, double mouseX, double mouseY, int windowWidth, int windowHeight) {
            auto &shader = layer.getShader();
            shader.enable();
            shader.setUniform2f("light_pos", math::vec2(static_cast<float>(mouseX * 16.0f / windowWidth),
                                                        static_cast<float>(9.0f - mouseY * 9.0f / windowHeight)));
            shader.disable();
        }
    }
}