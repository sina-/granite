#include <oni-core/graphic/oni-graphic-system.h>

#include <oni-core/graphic/oni-graphic-scene-manager.h>
#include <oni-core/entities/oni-entities-factory.h>

namespace oni {
    System_ParticleEmitter::System_ParticleEmitter(oni::EntityManager &tickEm,
                                                   oni::EntityManager &storageEm,
                                                   oni::SceneManager &sm,
                                                   oni::EntityFactory &ef) :
            SystemTemplate(tickEm), mEntityManager(storageEm), mSceneManager(sm), mEntityFactory(ef) {

    }

    void
    System_ParticleEmitter::update(oni::EntityTickContext &etc,
                                   oni::ParticleEmitter &emitter,
                                   oni::WorldP3D &pos) {
        if (!mSceneManager.isVisible(pos)) {
            return;
        }

        for (oni::u8 i = 0; i < emitter.count; ++i) {
            auto pID = mEntityFactory.loadEntity_Local(mEntityManager, mEntityManager, emitter.particle);
            // TODO: I can't just use pos.z as often these particles render over other objects and this will
            // cause z-fighting, I should ask the layer manager for a z value at least!
            mEntityManager.setWorldP3D(pID, pos.x, pos.y, pos.z);
            mEntityManager.setScale(pID, emitter.size, emitter.size);
            mEntityManager.setRandOrientation(pID, emitter.orientMin, emitter.orientMax);
            mEntityManager.setDirectionFromOrientation(pID);
            mEntityManager.setRandVelocity(pID, emitter.initialVMin, emitter.initialVMax);

            auto &growth = mEntityManager.get<oni::GrowOverTime>(pID);
            growth = emitter.growth;

            auto &acc = mEntityManager.get<oni::Acceleration>(pID);
            acc.current = emitter.acc;

            auto &md = mEntityManager.get<oni::Material_Definition>(pID);
            md.finish = emitter.material.finish;
            md.skin.color = emitter.material.skin.color;
            md.skin.texture.image.name = emitter.material.skin.texture.image.name;
        }
    }

    void
    System_ParticleEmitter::postUpdate(oni::EntityManager &mng,
                                       oni::duration32 dt) {

    }
}