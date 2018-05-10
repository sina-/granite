#include <fmod.hpp>

#include <oni-core/audio/audio-manager-fmod.h>
#include <oni-core/utils/oni-assert.h>
#include <oni-core/common/consts.h>
#include <oni-core/common/defines.h>

#define ERRCHECK(_result) ONI_DEBUG_ASSERT((_result) == FMOD_OK)

namespace oni {
    namespace audio {

        AudioManagerFMOD::AudioManagerFMOD() : mSystem{}, mSounds{}, mChannels{} {

            FMOD::System *system{nullptr};

            auto result = FMOD::System_Create(&system);
            ERRCHECK(result);
            mSystem = std::unique_ptr<FMOD::System, FMODDeleter>(system, FMODDeleter());

            common::uint32 version;
            result = system->getVersion(&version);
            ERRCHECK(result);
            ONI_DEBUG_ASSERT(version >= FMOD_VERSION)

            result = system->init(32, FMOD_INIT_NORMAL, nullptr);
            ERRCHECK(result);

            result = system->update();
            ERRCHECK(result);
        }

        void AudioManagerFMOD::tick() {
            auto result = mSystem->update();
            ERRCHECK(result);
        }

        oniSoundID AudioManagerFMOD::loadSound(const std::string &name) {
            // TODO: This will break for one-off sound effects, currently all the functions in this
            // class work if the audio is to be looped over.

            FMOD::Sound *sound{nullptr};
            auto result = mSystem->createSound(name.c_str(), FMOD_DEFAULT, nullptr, &sound);
            mSounds.emplace_back(std::unique_ptr<FMOD::Sound, FMODDeleter>(sound, FMODDeleter()));
            ERRCHECK(result);

            result = mSounds.back()->setMode(FMOD_LOOP_NORMAL);
            ERRCHECK(result);

            FMOD::Channel *channel{nullptr};
            result = mSystem->playSound(sound, nullptr, true, &channel);
            ERRCHECK(result);

            mChannels.emplace_back(std::unique_ptr<FMOD::Channel, FMODDeleter>(channel, FMODDeleter()));

            return mChannels.size() - 1;
        }

        void AudioManagerFMOD::playSound(oniSoundID id) {
            auto result = mChannels[id]->setPaused(false);
            ERRCHECK(result);
        }

        double AudioManagerFMOD::pauseSound(oniSoundID id) {
            auto result = mChannels[id]->setPaused(true);
            ERRCHECK(result);

            common::uint32 pos;
            result = mChannels[id]->getPosition(&pos, FMOD_TIMEUNIT_MS);
            ERRCHECK(result);

            // TODO: This is just a work around to keep the interface consistent with double.
            return (static_cast<double>(pos) + common::ep);
        }

        void AudioManagerFMOD::stopSound(oniSoundID id) {
            UNUSED(id);
        }

        void AudioManagerFMOD::setLoop(oniSoundID id, bool loop) {
            UNUSED(id);
            UNUSED(loop);

        }

        void AudioManagerFMOD::setVolume(oniSoundID id, common::real32 volume) {
            UNUSED(id);
            UNUSED(volume);

        }

        common::real32 AudioManagerFMOD::getVolume(oniSoundID id) {
            UNUSED(id);
            return 0;
        }

        bool AudioManagerFMOD::isPlaying(oniSoundID id) {
            bool isPaused{false};
            auto result = mChannels[id]->getPaused(&isPaused);
            ERRCHECK(result);
            return !isPaused;
        }

        void AudioManagerFMOD::seek(oniSoundID id, double position) {
            auto result = mChannels[id]->setPosition(static_cast<common::uint32>(position + common::ep),
                                                     FMOD_TIMEUNIT_MS);
            ERRCHECK(result);
        }

        void AudioManagerFMOD::setPitch(oniSoundID id, common::real32 pitch) {
            auto result = mChannels[id]->setPitch(pitch);
            ERRCHECK(result);
        }

        void FMODDeleter::operator()(FMOD::Sound *s) const {
            s->release();
        }

        void FMODDeleter::operator()(FMOD::System *sys) const {
            sys->close();
            sys->release();
        }

        void FMODDeleter::operator()(FMOD::Channel *channel) const {
            UNUSED(channel);
        }
    }
}