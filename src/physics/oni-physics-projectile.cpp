#include <oni-core/physics/oni-physics-projectile.h>

#include <Box2D/Box2D.h>
#include <GLFW/glfw3.h>

#include <oni-core/entities/oni-entities-client-data-manager.h>
#include <oni-core/entities/oni-entities-manager.h>
#include <oni-core/entities/oni-entities-factory.h>
#include <oni-core/component/oni-component-gameplay.h>
#include <oni-core/game/oni-game-event.h>

namespace oni {
    namespace physics {
        Projectile::Projectile(b2World *physicsWorld) :
                mPhysicsWorld{physicsWorld},
                mGunCoolDownS{0.2f} {}

        Projectile::~Projectile() = default;

        void
        Projectile::tick(entities::EntityFactory &entityFactory,
                         entities::ClientDataManager *clientData,
                         common::r64 tickTime) {
            /// Update cool-downs
            {
                auto view = entityFactory.getEntityManager().createView<component::GunCoolDown>();
                for (auto &&entity: view) {
                    auto &coolDown = view.get<component::GunCoolDown>(entity);
                    math::subAndZeroClip(coolDown.value, tickTime);
                }
            }

            /// Spawn projectile
            {
                auto carView = entityFactory.getEntityManager().createView<
                        component::WorldP3D,
                        component::Heading,
                        component::Car,
                        component::CarConfig,
                        component::EntityAttachment>();

                common::r32 velocity{200.f};

                for (auto &&entity: carView) {
                    auto *input = clientData->getClientInput(entity);
                    if (!input) {
                        continue;
                    }

                    if (input->isPressed(GLFW_KEY_F)) {
                        const auto &carPos = carView.get<component::WorldP3D>(entity);
                        const auto &carHeading = carView.get<component::Heading>(entity);
                        const auto &carConfig = carView.get<component::CarConfig>(entity);
                        const auto &car = carView.get<component::Car>(entity);
                        const auto &attachments = carView.get<component::EntityAttachment>(entity);
                        fireRocket(entityFactory, velocity, carPos, carHeading, carConfig, attachments);
                    }
                }
            }
        }

        void
        Projectile::fireRocket(entities::EntityFactory &entityFactory,
                               const common::r32 velocity,
                               const component::WorldP3D &pos,
                               const component::Heading &heading,
                               const component::CarConfig &carConfig,
                               const component::EntityAttachment &attachments) {
            for (common::size i = 0; i < attachments.entities.size(); ++i) {
                if (attachments.entityTypes[i] == entities::EntityType::VEHICLE_GUN) {
                    auto &gunCoolDown = entityFactory.getEntityManager().get<component::GunCoolDown>(
                            attachments.entities[i]);
                    if (gunCoolDown.value > 0) {
                        continue;
                    }
                    gunCoolDown.value = mGunCoolDownS;

                    // NOTE: Car rotates around its center of gravity, that means radius of the circle traversed by the
                    // car nose is half bounding rectangle's diagonal.
                    // Using car heading direction and its diagonal we could find the position
                    // of car nose, which is where we like to spawn the bullet projectiles.
                    // TODO: This looks unnecessary, can I not just use attached gun position to find the bullet
                    // spawn location?
                    common::r32 carHalfX =
                            static_cast<common::r32 >(carConfig.cgToFront + carConfig.cgToRear) / 2.f;
                    auto carHalfY = static_cast<common::r32>(carConfig.halfWidth);
                    auto offset = sqrt(carHalfX * carHalfX + carHalfY * carHalfY);

                    auto size = math::vec2{1.0f, 0.3f};
                    auto projectileOffset = sqrt(
                            size.x * size.x / 4.f + size.y * size.y / 4.f);
                    auto fudge = 0.2f;

                    auto rocketPos = component::WorldP3D{};
                    rocketPos.x = pos.x + (offset + projectileOffset + fudge) * cos(heading.value);
                    rocketPos.y = pos.y + (offset + projectileOffset + fudge) * sin(heading.value);
                    rocketPos.z = pos.z;

                    auto id = entityFactory.createEntity_SimpleRocket();
                    entityFactory.setWorldP3D(id, rocketPos.x, rocketPos.y, rocketPos.z);
                    entityFactory.setScale(id, size.x, size.y);
                    entityFactory.setTexture(id, "resources/images/bullet/2.png");
                    entityFactory.setHeading(id, heading.value);
                    entityFactory.createPhysics(id, rocketPos, size, heading.value);

                    auto * body = entityFactory.getEntityManager().get<component::PhysicalProperties>(id).body;

                    body->ApplyForceToCenter(
                            b2Vec2(static_cast<common::r32>(cos(heading.value) * velocity),
                                   static_cast<common::r32>(sin(heading.value) * velocity)),
                            true);
                    body->ApplyAngularImpulse(1, true);
/*
                    entityFactory.createEvent(ROCKET_LAUNCH, component::WorldP3D);
                    entityFactory.createEvent(COLLISION, ENTITY_A, ENTITY_B, component::WorldP3D);

                    entityFactory.createEvent_RocketLaunch(component::WorldP3D);
                    entityFactory.createEvent_Collision(ENTITY_A, ENTITY_B, component::WorldP3D);

                    for(event: entityFactory.getEvent_RocketLaunch()){
                        server.send(packet_RocketLaunch);
                    }

                    for(event: entityFactory.getEvent_Collision()){
                        server.send(packet_Collision);
                    }*/

                    entityFactory.getEntityManager().enqueueEvent<game::Event_RocketLaunch>(rocketPos);
                }
            }
        }
    }
}