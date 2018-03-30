#pragma once

#include <cmath>
#include <algorithm>

#include <oni-core/math/vec2.h>
#include <oni-core/components/input-data.h>

namespace oni {
    namespace physics {

        template<class T>
        int sign(T n) {
            return (T(0) < n) - (T(0) > n);
        }

        template<class T>
        T clip(const T &n, const T &lower, const T &upper) {
            return std::max(lower, std::min(n, upper));
        }

        static void tickCar(components::Car &car, const components::CarConfig &config,
                            const components::CarInput &inputs, float dt) {
            using components::carSimDouble;
            carSimDouble sn = std::sin(car.heading);
            carSimDouble cs = std::cos(car.heading);

            car.velocityLocal.x = cs * car.velocity.x + sn * car.velocity.y;
            car.velocityLocal.y = cs * car.velocity.y - sn * car.velocity.x;

            carSimDouble axleWeightFront = config.mass * (car.axleWeightRatioFront * config.gravity -
                                                       config.weightTransfer * car.accelerationLocal.x *
                                                       config.cgHeight /
                                                       car.wheelBase);
            carSimDouble axleWeightRear = config.mass * (car.axleWeightRatioRear * config.gravity +
                                                      config.weightTransfer * car.accelerationLocal.x *
                                                      config.cgHeight /
                                                      car.wheelBase);

            // Resulting velocity of the wheels as result of the yaw rate of the car body.
            // v = yawRate * r where r is distance from axle to CG and yawRate (angular velocity) in rad/s.
            carSimDouble yawSpeedFront = config.cgToFrontAxle * car.yawRate;
            carSimDouble yawSpeedRear = -config.cgToRearAxle * car.yawRate;

            // Calculate slip angles for front and rear wheels (a.k.a. alpha)
            carSimDouble slipAngleFront = std::atan2(car.velocityLocal.y + yawSpeedFront, std::abs(car.velocityLocal.x)) -
                                       sign(car.velocityLocal.x) * car.steerAngle;
            carSimDouble slipAngleRear = std::atan2(car.velocityLocal.y + yawSpeedRear, std::abs(car.velocityLocal.x));

            carSimDouble tireGripFront = config.tireGrip;
            carSimDouble tireGripRear = config.tireGrip *
                                     (1.0 -
                                      inputs.eBrake * (1.0 - config.lockGrip)); // reduce rear grip when eBrake is on

            carSimDouble frictionForceFront_cy =
                    clip(-config.cornerStiffnessFront * slipAngleFront, -tireGripFront, tireGripFront) *
                    axleWeightFront;
            carSimDouble frictionForceRear_cy =
                    clip(-config.cornerStiffnessRear * slipAngleRear, -tireGripRear, tireGripRear) * axleWeightRear;

            //  Get amount of brake/throttle from our inputs
            carSimDouble brake = std::min(inputs.brake * config.brakeForce + inputs.eBrake * config.eBrakeForce,
                                       config.brakeForce);
            carSimDouble throttle = inputs.throttle * config.engineForce;

            //  Resulting force in local car coordinates.
            //  This is implemented as a RWD car only.
            carSimDouble tractionForce_cx = throttle - brake * sign(car.velocityLocal.x);
            carSimDouble tractionForce_cy = 0;

            carSimDouble dragForce_cx = -config.rollResist * car.velocityLocal.x -
                                     config.airResist * car.velocityLocal.x * std::abs(car.velocityLocal.x);
            carSimDouble dragForce_cy = -config.rollResist * car.velocityLocal.y -
                                     config.airResist * car.velocityLocal.y * std::abs(car.velocityLocal.y);

            // total force in car coordinates
            carSimDouble totalForce_cx = dragForce_cx + tractionForce_cx;
            carSimDouble totalForce_cy =
                    dragForce_cy + tractionForce_cy + std::cos(car.steerAngle) * frictionForceFront_cy +
                    frictionForceRear_cy;

            // acceleration along car axes
            car.accelerationLocal.x = totalForce_cx / config.mass;  // forward/reverse acceleration
            car.accelerationLocal.y = totalForce_cy / config.mass;  // sideways acceleration

            // acceleration in world coordinates
            car.acceleration.x = cs * car.accelerationLocal.x - sn * car.accelerationLocal.y;
            car.acceleration.y = sn * car.accelerationLocal.x + cs * car.accelerationLocal.y;

            // update velocity
            car.velocity.x += car.acceleration.x * dt;
            car.velocity.y += car.acceleration.y * dt;

            car.velocityAbsolute = car.velocity.len();

            // calculate rotational forces
            carSimDouble angularTorque = (frictionForceFront_cy + tractionForce_cy) * config.cgToFrontAxle -
                                      frictionForceRear_cy * config.cgToRearAxle;

            //  Sim gets unstable at very slow speeds, so just stop the car
            if (std::abs(car.velocityAbsolute) < 0.5 && !throttle) {
                car.velocity.x = car.velocity.y = car.velocityAbsolute = 0;
                angularTorque = car.yawRate = 0;
            }

            carSimDouble angularAcceleration = angularTorque / car.inertia;

            car.yawRate += angularAcceleration * dt;
            car.heading += car.yawRate * dt;

            //  finally we can update position
            car.position.x += car.velocity.x * dt;
            car.position.y += car.velocity.y * dt;

        }

        static components::carSimDouble applySmoothSteer(const components::Car &car, components::carSimDouble steerInput, float dt) {
            components::carSimDouble steer = 0;

            if (std::abs(steerInput) > 0.001) {
                //  Move toward steering input
                steer = clip(car.steer + steerInput * dt * 2.0, -1.0, 1.0); // -inp.right, inp.left);
            } else {
                //  No steer input - move toward centre (0)
                if (car.steer > 0) {
                    steer = std::max(car.steer - dt * 1.0, (components::carSimDouble) 0.0f);
                } else if (car.steer < 0) {
                    steer = std::min(car.steer + dt * 1.0, (components::carSimDouble) 0.0f);
                }
            }

            return steer;
        };

        static components::carSimDouble applySafeSteer(const components::Car &car, components::carSimDouble steerInput) {
            auto avel = std::min(car.velocityAbsolute, (components::carSimDouble) 250.0);
            auto steer = steerInput * (1.0 - (avel / 280.0));
            return steer;
        };

    }
}
