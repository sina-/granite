#include <oni-particle-editor/entities/oni-particle-editor-entities-factory.h>

#include <oni-core/util/oni-util-structure.h>
#include <oni-core/entities/oni-entities-manager.h>
#include <oni-core/math/oni-math-z-layer-manager.h>
#include <oni-core/json/oni-json.h>
#include <oni-core/entities/oni-entities-serialization-json.h>
#include <oni-particle-editor/entities/oni-particle-editor-entities-serialization-json.h>

namespace oni {
    EntityFactory_ParticleEditor::EntityFactory_ParticleEditor(FontManager &fm,
                                                               TextureManager &tm,
                                                               ZLayerManager &zm) : EntityFactory_Client(fm, tm, zm) {
        COMPONENT_FACTORY_DEFINE(this, oni, Acceleration)
        COMPONENT_FACTORY_DEFINE(this, oni, BrushTrail)
        COMPONENT_FACTORY_DEFINE(this, oni, EntityName)
        COMPONENT_FACTORY_DEFINE(this, oni, GrowOverTime)
        COMPONENT_FACTORY_DEFINE(this, oni, Direction)
        COMPONENT_FACTORY_DEFINE(this, oni, Material_Definition)
        COMPONENT_FACTORY_DEFINE(this, oni, Material_Text)
        COMPONENT_FACTORY_DEFINE(this, oni, Orientation)
        COMPONENT_FACTORY_DEFINE(this, oni, ParticleEmitter)
        COMPONENT_FACTORY_DEFINE(this, oni, Scale)
        COMPONENT_FACTORY_DEFINE(this, oni, TimeToLive)
        COMPONENT_FACTORY_DEFINE(this, oni, Velocity)
        COMPONENT_FACTORY_DEFINE(this, oni, WorldP2D)
        COMPONENT_FACTORY_DEFINE(this, oni, WorldP3D)
        COMPONENT_FACTORY_DEFINE(this, oni, ZLayer)
    }
}