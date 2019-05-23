#pragma once

#include <map>
#include <memory>

#include <oni-core/component/oni-component-visual.h>
#include <oni-core/math/oni-math-mat4.h>
#include <oni-core/gameplay/oni-gameplay-lap-tracker.h>
#include <oni-core/graphic/oni-graphic-camera.h>

class b2World;

namespace oni {
    namespace math {
        struct vec2;

        class Rand;

        class ZLayerManager;
    }

    namespace entities {
        class EntityManager;

        class EntityFactory;
    }

    namespace component {
        struct Shape;
    }

    namespace graphic {
        class Shader;

        class BatchRenderer2D;

        class Renderer2D;

        class TextureManager;

        class FontManager;

        class DebugDrawBox2D;

        struct Brush;

        class SceneManager {
        public:
            // TODO: Getting quite big, I can split it into, SceneRenderer, SceneUpdater, and SceneManager
            SceneManager(const graphic::ScreenBounds &,
                         FontManager &,
                         math::ZLayerManager &,
                         b2World &,
                         common::r32
            );

            ~SceneManager();

            void
            render(entities::EntityFactory &);

            void
            renderPhysicsDebugData();

            void
            tick(const entities::EntityFactory &server,
                 entities::EntityFactory &client,
                 common::r64 tickTime);

            void
            renderRaw(const component::WorldP3D pos,
                      const component::Appearance &appearance);

            void
            splat(entities::EntityFactory &,
                  const component::WorldP3D &,
                  const component::Scale &,
                  const graphic::Brush &);

            void
            lookAt(common::r32 x,
                   common::r32 y);

            void
            lookAt(common::r32 x,
                   common::r32 y,
                   common::r32 distance);

            void
            zoom(common::r32 distance);

            const graphic::Camera &
            getCamera() const;

            const math::mat4 &
            getProjectionMatrix() const;

            const math::mat4 &
            getViewMatrix() const;

            common::r32
            getViewWidth() const;

            common::r32
            getViewHeight() const;

            common::u16
            getSpritesPerFrame() const;

            common::u16
            getParticlesPerFrame() const;

            common::u16
            getTexturesPerFrame() const;

            void
            resetCounters();

            // TODO: This is awful and inconsistent with the API of this class where I should only expose render().
            // Instead of exposing the internals, I can batch the render objects in DebugDrawBox2D and then
            // pass them to a new render() function that receives a vector of sprites
            // .
            void
            beginColorRendering();

            void
            endColorRendering();

        private:
            void
            renderStaticText(entities::EntityManager &,
                             common::r32 viewWidth,
                             common::r32 viewHeight);

            void
            _render(entities::EntityManager &,
                    common::r32 viewWidth,
                    common::r32 viewHeight);

        private:
            struct RaceInfoEntities {
                common::EntityID lapEntity{0};
                common::EntityID lapTimeEntity{0};
                common::EntityID lapBestTimeEntity{0};
            };

            struct WorldP3DAndHeading {
                component::WorldP3D pos;
                component::Heading heading;
            };

        private:
            void
            begin(const Shader &shader,
                  Renderer2D &renderer2D,
                  bool translate,
                  bool scale,
                  bool setMVP);

            void
            end(const Shader &shader,
                Renderer2D &renderer2D);

            void
            initRenderer();

            void
            prepareTexture(component::Texture &texture);

            common::EntityID
            createText(entities::EntityFactory &entityFactory,
                       const component::WorldP3D &worldPos,
                       const std::string &text);

            void
            updateRaceInfo(entities::EntityManager &,
                           const gameplay::CarLapInfo &carLap,
                           const RaceInfoEntities &carLapTextEntities);

            void
            updateCanvasTile(entities::EntityManager &entityManager,
                             common::EntityID entityID,
                             const graphic::Brush &,
                             const component::WorldP3D &,
                             const component::Scale &);


            const RaceInfoEntities &
            getOrCreateLapText(entities::EntityFactory &,
                               common::EntityID carEntityID,
                               const gameplay::CarLapInfo &carLap);

            common::EntityID
            getOrCreateCanvasTile(entities::EntityFactory &,
                                  const component::WorldP3D &pos);

            component::SmokeEmitterCD &
            getOrCreateEmitterCD(entities::EntityFactory &,
                                 common::EntityID);

            SceneManager::WorldP3DAndHeading
            applyParentTransforms(entities::EntityManager &manager,
                                  common::EntityID child,
                                  const component::WorldP3D &pos,
                                  const component::Heading &heading);

        private:
            std::unique_ptr<Shader> mShader{};
            std::unique_ptr<BatchRenderer2D> mRenderer{};

            std::unique_ptr<TextureManager> mTextureManager{};
            std::unique_ptr<DebugDrawBox2D> mDebugDrawBox2D{};
            FontManager &mFontManager;
            b2World &mPhysicsWorld;

            std::unique_ptr<math::Rand> mRand{};

            math::mat4 mModelMatrix{};
            math::mat4 mViewMatrix{};
            math::mat4 mProjectionMatrix{};

            std::map<common::u64, common::EntityID> mCanvasTileLookup{};
            std::map<common::EntityID, RaceInfoEntities> mLapInfoLookup{};

            const common::u16 mCanvasTileSizeX{0};
            const common::u16 mCanvasTileSizeY{0};
            const common::r32 mHalfCanvasTileSizeX{0.0f};
            const common::r32 mHalfCanvasTileSizeY{0.0f};

            graphic::ScreenBounds mScreenBounds{};
            graphic::Camera mCamera{0.0f, 0.0f, 1.0f};

            const common::u16 mMaxSpriteCount{0};
            const common::r32 mGameUnitToPixels{0};

            common::u16 mRenderedSpritesPerFrame{0};
            common::u16 mRenderedTexturesPerFrame{0};
            common::u16 mRenderedParticlesPerFrame{0};

            math::ZLayerManager &mZLayerManager;
        };
    }
}