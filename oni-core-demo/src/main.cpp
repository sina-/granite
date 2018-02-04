#include <iostream>
#include <graphics/batchrenderer2d.h>
#include <graphics/shader.h>
#include <graphics/window.h>
#include <chrono>

int main() {

    using namespace oni;
    using namespace graphics;
    using namespace math;
    using namespace std;
    using namespace buffers;

    int width = 1600;
    int height = 900;

    // NOTE: any call to GLEW functions will fail with Segfault if GLFW is uninitialized (initialization happens in Window).
    Window window("Oni Demo", width, height);

    Shader shader("shaders/basic.vert", "shaders/basic.frag");
    shader.enable();
//    shader.setUniformMat4("vw_matrix", mat4::translation(vec3(2, 2, 0)));
    shader.setUniformMat4("pr_matrix", mat4::orthographic(0.0f, 16.0f, 0.0f, 9.0f, -1.0f, 1.0f));

//    shader.setUniform4f("input_color", vec4(0.2f, 0.3f, 0.8f, 1.0f));
//    shader.setUniform2f("light_pos", vec2(4.0f, 1.5f));

    auto sprite1 = std::make_unique<Renderable2D>(math::vec2(4, 4), math::vec3(5, 5, 0), math::vec4(1, 0, 1, 1));
    auto sprite2 = std::make_unique<Renderable2D>(math::vec2(1, 2), math::vec3(0, 1, 1), math::vec4(0, 1, 1, 1));

    srand(static_cast<unsigned int>(time(nullptr)));

    std::vector<unique_ptr<Renderable2D>> sprites;

    float yStep = 0.03f;
    float xStep = 0.01f;

/*
    for (float y = 0.5; y < 8.5f; y += yStep) {
        for (float x = 0.5; x < 15.5f; x += xStep) {
            sprites.push_back(
                    make_unique<Renderable2D>(math::vec2(xStep, yStep), math::vec3(x, y, 0.0f),
                                              math::vec4(rand() % 1000 / 1000.0f, 0, 1, 1)));
        }
    }
    auto renderer = std::make_unique<BatchRenderer2D>(sprites.size() + 1);
*/

    auto renderer = std::make_unique<BatchRenderer2D>(100000);
    float frameTime = 0.0f;

    while (!window.closed()) {
        auto start = std::chrono::steady_clock::now();

        window.clear();

        const auto mouseX = window.getCursorX();
        const auto mouseY = window.getCursorY();

        width = window.getWidth();
        height = window.getHeight();

/*        shader.setUniform2f("light_pos", vec2(static_cast<float>(mouseX * 16.0f / width),
                                              static_cast<float>(9.0f - mouseY * 9.0f / height)));*/

        if (window.getMouseButton() != GLFW_KEY_UNKNOWN) {
            sprites.push_back(
                    make_unique<Renderable2D>(math::vec2(0.05f, 0.07f), math::vec3(((const float) mouseX) * 16.0f / width,
                                                                                 9.0f - ((const float) mouseY) * 9.0f / height , 0.0f),
                                              math::vec4(rand() % 1000 / 1000.0f, 0, 1, 1)));
        }

        renderer->begin();
        for (const auto &sprite: sprites) {
            renderer->submit(sprite);
        }
        renderer->end();
        renderer->flush();
        window.update();

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<float> diff = end - start;
        frameTime += diff.count();
        if (frameTime > 1.0f) {
            printl(1.0f / diff.count());
            frameTime = 0;
        }
    }

    return 0;
}