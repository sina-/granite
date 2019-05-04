#pragma once

#include <oni-core/common/typedefs.h>

namespace oni {
    namespace game {
        enum class EventType : common::uint8 {
            COLLISION,
            SPAWN_PARTICLE,
            ONE_SHOT_SOUND_EFFECT,
            ROCKET_LAUNCH,
        };

        struct CollidingEntity {
            entities::EntityType entityA{entities::EntityType::UNKNOWN};
            entities::EntityType entityB{entities::EntityType::UNKNOWN};
        };
    }
}