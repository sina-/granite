#pragma once

#include <oni-core/entities/entity.h>
#include <oni-core/components/physical.h>
#include <oni-core/entities/basic-entity-repo.h>

namespace oni {
    namespace entities {
        // TODO: Maybe move BasicEntityRepo into entity.h and rename it to Entity.
        // TODO: Make sure this inheritance doesn't add additional costs, or its minimum.
        class Vehicle : public BasicEntityRepo {
        public:
            Vehicle();

            ~Vehicle() = default;

            void setCar(const entities::entityID &id, const components::Car &car);

            void setCarConfig(const entities::entityID &id, const components::CarConfig &carConfig);

            components::Car &getCar(const entities::entityID &id);

            const components::CarConfig &getCarConfig(const entities::entityID &id) const;

        private:
            std::vector<components::Car> mCars;
            std::vector<components::CarConfig> mCarConfigs;

        protected:
            entities::entityID _createEntity() override;

        };
    }
}
