#include <physics/movement.h>

namespace oni {
    namespace physics {

        void Movement::update(entities::World &world, const graphics::Window &window) {
            unsigned long entityIndex = 0;

            for (const auto &entity: world.getEntities()) {
                if ((entity & entities::DynamicSprite) == entities::DynamicSprite) {
                    auto position = world.getEntityPlacement(entityIndex);
                    auto velocity = world.getEntityVelocity(entityIndex);
                    auto magnitude = velocity.magnitude;

                    // TODO: Internals of IO, such as GLFW_KEY_..., should not
                    // be exposed outside.
                    switch (window.getKeyPressed()) {
                        case GLFW_KEY_W: {
                            velocity.direction += math::vec3(0.0f, magnitude, 0.0f);
                            break;
                        }
                        case GLFW_KEY_A: {
                            velocity.direction += math::vec3(-magnitude, 0.0f, 0.0f);
                            break;
                        }
                        case GLFW_KEY_S: {
                            velocity.direction += math::vec3(0.0f, -magnitude, 0.0f);
                            break;
                        }
                        case GLFW_KEY_D:
                            velocity.direction += math::vec3(magnitude, 0.0f, 0.0f);
                            break;

                        default:
                            break;
                    }
                    updatePosition(position, velocity.direction);
                    world.setEntityPlacement(entityIndex, position);
                    world.setEntityVelocity(entityIndex, velocity);

                }
                ++entityIndex;
            }

        }

        void Movement::updatePosition(components::Placement &position, const math::vec3 &direction) {
            position.vertexA += direction;
            position.vertexB += direction;
            position.vertexC += direction;
            position.vertexD += direction;
        }
    }
}