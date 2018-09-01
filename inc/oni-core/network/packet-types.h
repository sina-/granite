#pragma once

#include <oni-core/common/typedefs.h>

namespace oni {
    namespace network {
        enum class PacketType : common::int8 {
            UNKNOWN = 0,
            PING = 1,
            MESSAGE = 2,
            SETUP_SESSION = 3,
            CAR_ENTITY_ID = 4,
            CLIENT_INPUT = 5,
            ENTITIES_ALL = 6,
            ENTITIES_DELTA = 7,
        };
    }
}
