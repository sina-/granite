#pragma once

#include <oni-core/math/vec2.h>
#include <oni-core/math/vec3.h>
#include <oni-core/math/vec4.h>
#include <oni-core/math/mat4.h>
#include <oni-core/common/typedefs.h>

class b2Body;

namespace oni {
    namespace components {

        struct Shape {
            /**
             *    NOTE: Local coordinates for dynamic objects and world coordinates for static objects.
             *
             *    B    C
             *    +----+
             *    |    |
             *    +----+
             *    A    D
             */
            math::vec3 vertexA{0.0f, 0.0f, 0.0f};
            math::vec3 vertexB{0.0f, 0.0f, 0.0f};
            math::vec3 vertexC{0.0f, 0.0f, 0.0f};
            math::vec3 vertexD{0.0f, 0.0f, 0.0f};

            math::vec3 getPosition() const { return vertexA; }

            math::vec2 getSize() const {
                return math::vec2{vertexD.x - vertexA.x, vertexB.y - vertexA.y};
            }

            static Shape fromPositionAndSize(const math::vec3 &position, const math::vec2 &size) {
                return Shape{
                        math::vec3{position.x, position.y, position.z},
                        math::vec3{position.x, position.y + size.y, position.z},
                        math::vec3{position.x + size.x, position.y + size.y, position.z},
                        math::vec3{position.x + size.x, position.y, position.z}};
            }

            static Shape fromSizeAndRotation(const math::vec2 &size, const common::real32 rotation) {
                auto shape = Shape{
                        math::vec3{0, 0, 1},
                        math::vec3{0, size.y, 1},
                        math::vec3{size.x, size.y, 1},
                        math::vec3{size.x, 0, 1}
                };
                // Cast to ignore float imprecision.
                if (static_cast<common::uint16>(rotation)) {
                    auto halfSize = math::vec3{size.x / 2.0f, size.y / 2.0f, 0.0f};

                    shape.vertexA -= halfSize;
                    shape.vertexB -= halfSize;
                    shape.vertexC -= halfSize;
                    shape.vertexD -= halfSize;

                    auto rotationMat = math::mat4::rotation(math::toRadians(rotation), math::vec3{0.0f, 0.0f, 1.0f});
                    shape.vertexA = rotationMat * shape.vertexA;
                    shape.vertexB = rotationMat * shape.vertexB;
                    shape.vertexC = rotationMat * shape.vertexC;
                    shape.vertexD = rotationMat * shape.vertexD;

                    shape.vertexA += halfSize;
                    shape.vertexB += halfSize;
                    shape.vertexC += halfSize;
                    shape.vertexD += halfSize;
                }
                return shape;
            }

            template<class Archive>
            void serialize(Archive &archive) {
                archive(vertexA, vertexB, vertexC, vertexD);
            }

        };

        // TODO: Not super happy about keeping a raw pointer to an object! But as long as I use naked Box2D
        // I don't think there is a better way.
        struct PhysicalProperties {
            b2Body *body{};
        };

        struct Placement {
            math::vec3 position{0.0f, 0.0f, 0.0f};
            common::real32 rotation{0.0f};
            math::vec3 scale{1.0f, 1.0f, 0.0f};

            template<class Archive>
            void serialize(Archive &archive) {
                archive(position, rotation, scale);
            }
        };

        struct TagStatic {

            template<class Archive>
            void serialize(Archive &archive) {}
        };

        struct TagDynamic {
            template<class Archive>
            void serialize(Archive &archive) {}
        };

        struct TagVehicle {
            template<class Archive>
            void serialize(Archive &archive) {}
        };

        struct ChunkIndices {
            common::int64 x{0};
            common::int64 y{0};

            template<class Archive>
            void serialize(Archive &archive) {
                archive(x, y);
            }
        };

        struct RoadTileIndices {
            // NOTE: This is relative to Chunk
            common::uint16 x{0};
            common::uint16 y{0};

            template<class Archive>
            void serialize(Archive &archive) {
                archive(x, y);
            }
        };

        struct TileIndices {
            common::int64 x{0};
            common::int64 y{0};
        };

        struct BoarderRoadTiles {
            RoadTileIndices eastBoarder{};
            RoadTileIndices southBoarder{};
            RoadTileIndices westBoarder{};
            RoadTileIndices northBoarder{};

            template<class Archive>
            void serialize(Archive &archive) {
                archive(eastBoarder, southBoarder, westBoarder, northBoarder);
            }
        };

        struct Chunk {
            math::vec3 position{0.0f, 0.0f, 0.0f};
            common::uint64 packedIndices{0};
            BoarderRoadTiles boarderRoadTiles{};

            template<class Archive>
            void serialize(Archive &archive) {
                archive(position, packedIndices, boarderRoadTiles);
            }
        };

        struct CarConfig {
            common::CarSimDouble gravity{9.81f};
            common::CarSimDouble mass{1200};
            common::CarSimDouble inertialScale{1.0f};
            common::CarSimDouble halfWidth{0.9f};
            common::CarSimDouble cgToFront{2.0f}; // Distance from center of gravity to the front in meters.
            common::CarSimDouble cgToRear{2.0f};
            common::CarSimDouble cgToFrontAxle{1.25f};
            common::CarSimDouble cgToRearAxle{1.25f};
            common::CarSimDouble cgHeight{0.55f};
            common::CarSimDouble wheelRadius{0.3f}; // For rendering only
            common::CarSimDouble wheelWidth{0.2f}; // For rendering only
            common::CarSimDouble tireGrip{2.0f};
            common::CarSimDouble lockGrip{0.6f}; // % of grip when wheel is locked
            common::CarSimDouble engineForce{4000.0f};
            common::CarSimDouble brakeForce{12000.0f};
            common::CarSimDouble eBrakeForce{12000.0f / 5.5f};
            common::CarSimDouble weightTransfer{0.2f};
            common::CarSimDouble maxSteer{0.6f}; // in radians
            common::CarSimDouble cornerStiffnessFront{5.0f};
            common::CarSimDouble cornerStiffnessRear{5.2f};
            common::CarSimDouble airResist{2.5f};
            common::CarSimDouble rollResist{8.0f};

            common::CarSimDouble gearRatio{2.7f};
            common::CarSimDouble differentialRatio{3.4f};

            template<class Archive>
            void serialize(Archive &archive) {
                archive(gravity,
                        mass,
                        inertialScale,
                        halfWidth,
                        cgToFront,
                        cgToRear,
                        cgToFrontAxle,
                        cgToRearAxle,
                        cgHeight,
                        wheelRadius,
                        wheelWidth,
                        tireGrip,
                        lockGrip,
                        engineForce,
                        brakeForce,
                        eBrakeForce,
                        weightTransfer,
                        maxSteer,
                        cornerStiffnessFront,
                        cornerStiffnessRear,
                        airResist,
                        rollResist,
                        gearRatio,
                        differentialRatio
                );
            }
        };

        struct Car {
            common::CarSimDouble heading{};
            common::CarSimDouble velocityAbsolute{};
            common::CarSimDouble angularVelocity{}; // Angular velocity in radians (rad/s)
            common::CarSimDouble steer{}; // (-1.0..1.0)
            common::CarSimDouble steerAngle{}; // (-maxSteer..maxSteer)
            common::CarSimDouble inertia{};
            common::CarSimDouble wheelBase{};
            common::CarSimDouble axleWeightRatioFront{};
            common::CarSimDouble axleWeightRatioRear{};
            common::CarSimDouble rpm{};
            common::CarSimDouble maxVelocityAbsolute{};
            common::CarSimDouble accumulatedEBrake{};
            common::CarSimDouble slipAngleFront{};
            common::CarSimDouble slipAngleRear{};


            math::vec2 position{};
            math::vec2 velocity{}; // m/s
            math::vec2 velocityLocal{}; // m/s
            math::vec2 acceleration{};
            math::vec2 accelerationLocal{};

            bool accelerating{};
            bool slippingFront{};
            bool slippingRear{};

            bool smoothSteer{};
            bool safeSteer{};

            common::CarSimDouble distanceFromCamera{};

            common::EntityID tireFR{};
            common::EntityID tireFL{};
            common::EntityID tireRR{};
            common::EntityID tireRL{};

            template<class Archive>
            void serialize(Archive &archive) {
                archive(
                        heading,
                        velocityAbsolute,
                        angularVelocity,
                        steer,
                        steerAngle,
                        inertia,
                        wheelBase,
                        axleWeightRatioFront,
                        axleWeightRatioRear,
                        rpm,
                        maxVelocityAbsolute,
                        accumulatedEBrake,
                        slipAngleFront,
                        slipAngleRear,
                        position,
                        velocity,
                        velocityLocal,
                        acceleration,
                        accelerationLocal,
                        accelerating,
                        slippingFront,
                        slippingRear,
                        smoothSteer,
                        safeSteer,
                        distanceFromCamera,

                        tireFR,
                        tireFL,
                        tireRR,
                        tireRL);
            }

            Car() {}

            explicit Car(const components::CarConfig &c) {
                heading = 0.0f;
                velocityAbsolute = 0.0f;
                angularVelocity = 0.0f;
                steer = 0.0f;
                steerAngle = 0.0f;
                inertia = c.mass * c.inertialScale;
                wheelBase = c.cgToFrontAxle + c.cgToRearAxle;
                axleWeightRatioFront = c.cgToRearAxle / wheelBase;
                axleWeightRatioRear = c.cgToFrontAxle / wheelBase;
                slipAngleFront = 0.0f;
                slipAngleRear = 0.0f;

                position = math::vec2{0.0f, 0.0f};
                velocity = math::vec2{0.0f, 0.0f};
                velocityLocal = math::vec2{0.0f, 0.0f};
                acceleration = math::vec2{0.0f, 0.0f};
                accelerationLocal = math::vec2{0.0f, 0.0f};

                rpm = 0.0f;
                accelerating = false;
                // Formula for solving quadratic equation: -c.airResistance*v^2 - c.rollResistance*v + c.engineForce = 0
                // Notice we are only interested in the positive answer.
                maxVelocityAbsolute = (c.rollResist -
                                       std::sqrt(c.rollResist * c.rollResist + 4 * c.airResist * c.engineForce)) /
                                      (-2 * c.airResist);
                accumulatedEBrake = 0.0f;

                smoothSteer = true;
                safeSteer = true;
                slippingFront = false;
                slippingRear = false;

                distanceFromCamera = 1.0f;
            }
        };


    }
}