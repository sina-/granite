#pragma once

#include <oni-core/common/oni-common-typedef.h>
#include <oni-core/component/oni-component-geometry.h>

namespace oni {
    namespace entities {
        class EntityManager;
    }

    namespace component {
        struct Rectangle;
    }

    namespace math {
        struct vec2;
        struct vec3;
        struct mat4;
    }


    namespace math {
        component::Rectangle
        shapeTransformation(const math::mat4 &,
                            const component::Rectangle &);

        math::mat4
        createTransformation(const component::WorldP3D &position,
                             const component::Heading &rotation,
                             const component::Scale &scale);

        void
        worldToLocalTranslation(const component::WorldP3D &reference,
                                component::WorldP3D &operand);

        void
        localToWorldTranslation(const component::WorldP3D &reference,
                                vec3 &operand);

        void
        localToWorldTranslation(common::r32 x,
                                common::r32 y,
                                vec3 &operand);

        void
        localToWorldTranslation(const component::WorldP3D &,
                                component::Rectangle &);

        component::WorldP3D
        localToWorldTranslation(const math::mat4 &trans,
                                const component::WorldP3D &operand);

        void
        localToTextureTranslation(common::r32 ratio,
                                  component::WorldP3D &operand);

        /**
         * Translates operand in the world coordinates to local coordinates of reference in texture coordinates.
         * For example, if object A is at (5, 10, 0) in the world, and object B at (1, 1, 0), given that
         * each unit of distance in game occupies 20 units of pixel in a texture, that is ratio, then point B
         * is located at (1 - 5, 10 - 1, 0 - 0) in reference to A in game units and multiply that
         * by the ratio to get the texture coordinates.
         *
         * @param reference to which the translation is applied in world coordinates
         * @param ratio each game unit is worth this many texture pixels
         * @param operand input as world coordinate and outputs as local texture coordinates
         */
        void
        worldToTextureCoordinate(const oni::component::WorldP3D &reference,
                                 common::r32 ratio,
                                 oni::component::WorldP3D &operand);
    }
}
