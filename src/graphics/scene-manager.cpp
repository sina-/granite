#include <oni-core/graphics/scene-manager.h>

#include <oni-core/graphics/shader.h>
#include <oni-core/graphics/renderer-2d.h>
#include <oni-core/graphics/batch-renderer-2d.h>
#include <oni-core/graphics/texture-manager.h>
#include <oni-core/physics/transformation.h>
#include <oni-core/entities/entity-manager.h>
#include <oni-core/entities/create-entity.h>
#include <oni-core/common/consts.h>


namespace oni {
    namespace graphics {
        // TODO: save this as a tag in the world registry
        static const common::real32 GAME_UNIT_TO_PIXELS = 20.0f;

        SceneManager::SceneManager(const components::ScreenBounds &screenBounds) :
                mSkidTileSizeX{64}, mSkidTileSizeY{64},
                mHalfSkidTileSizeX{mSkidTileSizeX / 2.0f},
                mHalfSkidTileSizeY{mSkidTileSizeY / 2.0f},
                // 64k vertices
                mMaxSpriteCount(16 * 1000), mScreenBounds(screenBounds) {

            mProjectionMatrix = math::mat4::orthographic(screenBounds.xMin, screenBounds.xMax, screenBounds.yMin,
                                                         screenBounds.yMax, -1.0f, 1.0f);
            mViewMatrix = math::mat4::identity();

            mModelMatrix = math::mat4::identity();

            mSkidEntityManager = std::make_unique<entities::EntityManager>();

            // TODO: Resources are not part of oni-core library! This structure as is not flexible, meaning
            // I am forcing the users to only depend on built-in shaders. I should think of a better way
            // to provide flexibility in type of shaders users can define and expect to just work by having buffer
            // structure and what not set up automatically.
            mTextureShader = std::make_unique<graphics::Shader>("resources/shaders/texture.vert",
                                                                "resources/shaders/texture.frag");
            initializeTextureRenderer(*mTextureShader);
            // TODO: As it is now, BatchRenderer is coupled with the Shader. It requires the user to setup the
            // shader prior to render series of calls. Maybe shader should be built into the renderer.

            mColorShader = std::make_unique<graphics::Shader>("resources/shaders/basic.vert",
                                                              "resources/shaders/basic.frag");

            mTextureManager = std::make_unique<TextureManager>();

            initializeColorRenderer(*mColorShader);
        }

        SceneManager::~SceneManager() = default;

        void SceneManager::initializeColorRenderer(const Shader &shader) {
            auto program = shader.getProgram();

            auto positionIndex = glGetAttribLocation(program, "position");
            auto colorIndex = glGetAttribLocation(program, "color");

            if (positionIndex == -1 || colorIndex == -1) {
                throw std::runtime_error("Invalid attribute name.");
            }

            GLsizei stride = sizeof(components::ColoredVertex);
            auto position = std::make_unique<components::BufferStructure>
                    (components::BufferStructure{static_cast<GLuint>(positionIndex), 3, GL_FLOAT, GL_FALSE,
                                                 stride,
                                                 static_cast<const GLvoid *>(nullptr)});
            auto color = std::make_unique<components::BufferStructure>
                    (components::BufferStructure{static_cast<GLuint>(colorIndex), 4, GL_FLOAT, GL_TRUE, stride,
                                                 reinterpret_cast<const GLvoid *>(offsetof(components::ColoredVertex,
                                                                                           color))});

            auto bufferStructures = common::BufferStructures();
            bufferStructures.push_back(std::move(position));
            bufferStructures.push_back(std::move(color));

            mColorRenderer = std::make_unique<BatchRenderer2D>(
                    mMaxSpriteCount,
                    common::maxNumTextureSamplers,
                    sizeof(components::ColoredVertex),
                    std::move(bufferStructures));
        }

        void SceneManager::initializeTextureRenderer(const Shader &shader) {
            auto program = shader.getProgram();

            auto positionIndex = glGetAttribLocation(program, "position");
            auto samplerIDIndex = glGetAttribLocation(program, "samplerID");
            auto uvIndex = glGetAttribLocation(program, "uv");

            if (positionIndex == -1 || samplerIDIndex == -1 || uvIndex == -1) {
                throw std::runtime_error("Invalid attribute name.");
            }

            GLsizei stride = sizeof(components::TexturedVertex);
            auto position = std::make_unique<components::BufferStructure>
                    (components::BufferStructure{static_cast<GLuint>(positionIndex), 3, GL_FLOAT, GL_FALSE, stride,
                                                 static_cast<const GLvoid *>(nullptr)});
            auto samplerID = std::make_unique<components::BufferStructure>
                    (components::BufferStructure{static_cast<GLuint>(samplerIDIndex), 1, GL_FLOAT, GL_FALSE, stride,
                                                 reinterpret_cast<const GLvoid *>(offsetof(components::TexturedVertex,
                                                                                           samplerID))});
            auto uv = std::make_unique<components::BufferStructure>
                    (components::BufferStructure{static_cast<GLuint>(uvIndex), 2, GL_FLOAT, GL_FALSE, stride,
                                                 reinterpret_cast<const GLvoid *>(offsetof(components::TexturedVertex,
                                                                                           uv))});

            auto bufferStructures = common::BufferStructures();
            bufferStructures.push_back(std::move(position));
            bufferStructures.push_back(std::move(samplerID));
            bufferStructures.push_back(std::move(uv));

            auto renderer = std::make_unique<BatchRenderer2D>(
                    mMaxSpriteCount,
                    // TODO: If there are more than this number of textures to render in a draw call, it will fail
                    common::maxNumTextureSamplers,
                    sizeof(components::TexturedVertex),
                    std::move(bufferStructures));

            shader.enable();
            shader.setUniformiv("samplers", renderer->generateSamplerIDs());
            shader.disable();

            mTextureRenderer = std::move(renderer);
        }

        void SceneManager::begin(const Shader &shader, Renderer2D &renderer2D) {
            shader.enable();

            auto view = math::mat4::scale(math::vec3{mCamera.z, mCamera.z, 1.0f}) *
                        math::mat4::translation(-mCamera.x, -mCamera.y, 0.0f);
            auto mvp = mProjectionMatrix * view;
            shader.setUniformMat4("mvp", mvp);
            renderer2D.begin();
        }

        void SceneManager::prepareTexture(components::Texture &texture) {
            // TODO: implement
            switch (texture.status) {
                case components::TextureStatus::READY: {
                    break;
                }
                case components::TextureStatus::NEEDS_LOADING_USING_PATH: {
                    auto loadedTexture = mTextureManager->findOrLoad(texture.filePath);
                    texture = *loadedTexture;
                    break;
                }
                case components::TextureStatus::NEEDS_LOADING_USING_DATA: {
                    assert(texture.width);
                    assert(texture.height);

                    auto loadedTexture = mTextureManager->loadFromData(texture.width, texture.height, texture.data);
                    texture = loadedTexture;
                    break;
                }
                case components::TextureStatus::NEEDS_RELOADING_USING_PATH: {
                    break;
                }
                case components::TextureStatus::NEEDS_RELOADING_USING_DATA: {
                    break;
                }
                default: {
                    assert(false);
                    break;
                }
            }
        }

        void SceneManager::end(const Shader &shader, Renderer2D &renderer2D) {
            renderer2D.end();
            renderer2D.flush();
            shader.disable();
        }

        void SceneManager::render(entities::EntityManager &manager, common::EntityID lookAtEntity) {
            auto halfViewWidth = getViewWidth() / 2.0f;
            auto halfViewHeight = getViewHeight() / 2.0f;

            {
                auto lock = manager.scopedLock();
                begin(*mColorShader, *mColorRenderer);
                renderColored(manager, halfViewWidth, halfViewHeight);
            }
            end(*mColorShader, *mColorRenderer);

            {
                auto lock = manager.scopedLock();
                if (lookAtEntity && manager.has<components::Placement>(lookAtEntity)) {
                    const auto &pos = manager.get<components::Placement>(lookAtEntity).position;
                    lookAt(pos.x, pos.y);
                }

                begin(*mTextureShader, *mTextureRenderer);
                renderStaticTextured(manager, halfViewWidth, halfViewHeight);
                renderStaticTextured(*mSkidEntityManager, halfViewWidth, halfViewHeight);
                renderDynamicTextured(manager, halfViewWidth, halfViewHeight);
                // Release the lock as soon as we are done with the registry.
            }

            end(*mTextureShader, *mTextureRenderer);
        }

        void SceneManager::renderRaw(const components::Shape &shape, const components::Appearance &appearance) {
            mColorRenderer->submit(shape, appearance);
            ++mRenderedSpritesPerFrame;
        }

        void SceneManager::beginColorRendering() {
            begin(*mColorShader, *mColorRenderer);
        }

        void SceneManager::endColorRendering() {
            end(*mColorShader, *mColorRenderer);
        }

        void SceneManager::renderStaticTextured(entities::EntityManager &manager, common::real32 halfViewWidth,
                                                common::real32 halfViewHeight) {
            {
                auto staticTextureSpriteView = manager.createView<components::TagTextureShaded, components::Shape,
                        components::Texture, components::TagStatic>();
                for (const auto &entity: staticTextureSpriteView) {
                    const auto &shape = staticTextureSpriteView.get<components::Shape>(entity);
                    if (!visibleToCamera(shape, halfViewWidth, halfViewHeight)) {
                        continue;
                    }
                    auto &texture = staticTextureSpriteView.get<components::Texture>(entity);
                    prepareTexture(texture);

                    ++mRenderedSpritesPerFrame;
                    ++mRenderedTexturesPerFrame;

                    // TODO: submit will fail if we reach maximum number of sprites.
                    // I could also check the number of entities using the view and decide before hand at which point I
                    // flush the renderer and open up room for new sprites.

                    // TODO: For buffer data storage take a look at: https://www.khronos.org/opengl/wiki/Buffer_Object#Immutable_Storage
                    // Currently the renderer is limited to 32 samplers, I have to either use the reset method
                    // or look to alternatives of how to deal with many textures, one solution is to create a texture atlas
                    // by merging many textures to keep below the limit. Other solutions might be looking into other type
                    // of texture storage that can hold bigger number of textures.
                    mTextureRenderer->submit(shape, texture);
                }
            }
        }

        void SceneManager::renderDynamicTextured(entities::EntityManager &manager, common::real32 halfViewWidth,
                                                 common::real32 halfViewHeight) {
            {
                // TODO: Maybe I can switch to none-locking view if I can split the registry so that rendering and
                // other systems don't share any entity component, or the shared section is minimum and I can create
                // copy of that data before starting rendering and only lock the registry at that point
                auto dynamicTextureSpriteView = manager.createView<components::TagTextureShaded, components::Shape,
                        components::Texture, components::Placement, components::TagDynamic>();
                for (const auto &entity: dynamicTextureSpriteView) {
                    const auto &shape = dynamicTextureSpriteView.get<components::Shape>(entity);
                    const auto &placement = dynamicTextureSpriteView.get<components::Placement>(entity);

                    auto transformation = physics::Transformation::createTransformation(placement.position,
                                                                                        placement.rotation,
                                                                                        placement.scale);

                    // TODO: I need to do this for physics anyway! Maybe I can store PlacementLocal and PlacementWorld
                    // separately for each entity and each time a physics system updates an entity it will automatically
                    // recalculate PlacementWorld for the entity and all its child entities.
                    // TODO: Instead of calling .has(), slow operation, split up dynamic entity rendering into two
                    // 1) Create a view with all of them that has TransformParent; 2) Create a view without parent
                    if (manager.has<components::TransformParent>(entity)) {
                        const auto &transformParent = manager.get<components::TransformParent>(entity);
                        // NOTE: Order matters. First transform by parent's transformation then child.
                        transformation = transformParent.transform * transformation;
                    }

                    auto shapeTransformed = physics::Transformation::shapeTransformation(transformation, shape);
                    if (!visibleToCamera(shapeTransformed, halfViewWidth, halfViewHeight)) {
                        continue;
                    }

                    auto &texture = dynamicTextureSpriteView.get<components::Texture>(entity);

                    prepareTexture(texture);
                    mTextureRenderer->submit(shapeTransformed, texture);

                    ++mRenderedSpritesPerFrame;
                    ++mRenderedTexturesPerFrame;
                }
            }
        }

        void SceneManager::renderColored(entities::EntityManager &manager, common::real32 halfViewWidth,
                                         common::real32 halfViewHeight) {
            {
                auto staticSpriteView = manager.createView<components::TagColorShaded, components::Shape,
                        components::Appearance, components::TagStatic>();
                for (const auto &entity: staticSpriteView) {
                    const auto &shape = staticSpriteView.get<components::Shape>(entity);
                    if (!visibleToCamera(shape, halfViewWidth, halfViewHeight)) {
                        continue;
                    }
                    const auto &appearance = staticSpriteView.get<components::Appearance>(entity);

                    ++mRenderedSpritesPerFrame;

                    mColorRenderer->submit(shape, appearance);
                }
            }
        }


        void SceneManager::tick(entities::EntityManager &manager) {
            std::vector<components::Placement> carTireRRPlacements{};
            std::vector<components::Placement> carTireRLPlacements{};
            std::vector<components::TransformParent> carTireRRTransformParent{};
            std::vector<components::TransformParent> carTireRLTransformParent{};
            std::vector<common::uint8> skidOpacity{};
            {
                auto carView = manager.createViewScopeLock<components::Car, components::Placement>();
                for (auto carEntity: carView) {
                    const auto car = carView.get<components::Car>(carEntity);
                    // NOTE: Technically I should use slippingRear, but this gives better effect
                    if (car.slippingFront || true) {
                        // TODO: This is not in the view and it will be very slow as more entities are added to the
                        // registry. Perhaps I can save the tires together with the car
                        const auto &carTireRRPlacement = manager.get<components::Placement>(car.tireRR);
                        carTireRRPlacements.push_back(carTireRRPlacement);
                        const auto &carTireRLPlacement = manager.get<components::Placement>(car.tireRL);
                        carTireRLPlacements.push_back(carTireRLPlacement);

                        const auto &transformParentRR = manager.get<components::TransformParent>(car.tireRR);
                        carTireRRTransformParent.push_back(transformParentRR);
                        const auto &transformParentRL = manager.get<components::TransformParent>(car.tireRL);
                        carTireRLTransformParent.push_back(transformParentRL);

                        auto alpha = static_cast<common::uint8>((car.velocityAbsolute / car.maxVelocityAbsolute) * 255);
                        skidOpacity.push_back(alpha);
                    }

                }
            }

            for (size_t i = 0; i < skidOpacity.size(); ++i) {
                auto skidMarkRRPos = carTireRRTransformParent[i].transform * carTireRRPlacements[i].position;
                auto skidMarkRLPos = carTireRLTransformParent[i].transform * carTireRLPlacements[i].position;

                common::EntityID skidEntityRL{0};
                common::EntityID skidEntityRR{0};

                skidEntityRL = createSkidTileIfMissing(skidMarkRLPos.getXY());
                skidEntityRR = createSkidTileIfMissing(skidMarkRRPos.getXY());

                updateSkidTexture(skidMarkRLPos, skidEntityRL, skidOpacity[i]);
                updateSkidTexture(skidMarkRRPos, skidEntityRR, skidOpacity[i]);
            }
        }

        common::EntityID SceneManager::createSkidTileIfMissing(const math::vec2 &position) {
            const auto x = math::positionToIndex(position.x, mSkidTileSizeX);
            const auto y = math::positionToIndex(position.y, mSkidTileSizeY);
            const auto packedIndices = math::packIntegers(x, y);

            common::EntityID skidTileID{};
            auto exists = mPackedSkidIndicesToEntity.find(packedIndices) != mPackedSkidIndicesToEntity.end();
            if (!exists) {
                const auto skidIndexX = math::positionToIndex(position.x, mSkidTileSizeX);
                const auto skidIndexY = math::positionToIndex(position.y, mSkidTileSizeY);
                const auto skidTilePosX = math::indexToPosition(skidIndexX, mSkidTileSizeX);
                const auto skidTilePosY = math::indexToPosition(skidIndexY, mSkidTileSizeY);
                const auto positionInWorld = math::vec3{skidTilePosX, skidTilePosY, 1.0f};
                const auto skidTileSize = math::vec2{static_cast<common::real32>(mSkidTileSizeX),
                                                     static_cast<common::real32>(mSkidTileSizeY)};

                const auto skidWidthInPixels = static_cast<common::uint16>(mSkidTileSizeX * GAME_UNIT_TO_PIXELS +
                                                                           common::ep);
                const auto skidHeightInPixels = static_cast<common::uint16>(mSkidTileSizeY * GAME_UNIT_TO_PIXELS +
                                                                            common::ep);
                const auto skidDefaultPixel = components::PixelRGBA{};
                auto skidData = graphics::TextureManager::generateBits(skidWidthInPixels, skidHeightInPixels,
                                                                       skidDefaultPixel);
                auto skidTexture = mTextureManager->loadFromData(skidWidthInPixels, skidHeightInPixels, skidData);
                // TODO: I can blend skid textures using this data
                skidTexture.data = skidData;

                skidTileID = entities::createStaticEntity(*mSkidEntityManager, skidTileSize, positionInWorld);
                mSkidEntityManager->assign<components::Texture>(skidTileID, skidTexture);
                mPackedSkidIndicesToEntity.emplace(packedIndices, skidTileID);
            } else {
                skidTileID = mPackedSkidIndicesToEntity.at(packedIndices);
            }
            return skidTileID;
        }

        void SceneManager::updateSkidTexture(const math::vec3 &position, common::EntityID skidTextureEntity,
                                             common::uint8 alpha) {
            auto skidMarksTexture = mSkidEntityManager->get<components::Texture>(skidTextureEntity);
            const auto skidMarksTexturePos = mSkidEntityManager->get<components::Shape>(
                    skidTextureEntity).getPosition();

            auto skidPos = position;
            physics::Transformation::worldToLocalTextureTranslation(skidMarksTexturePos, GAME_UNIT_TO_PIXELS,
                                                                    skidPos);

            // TODO: I can not generate geometrical shapes that are rotated. Until I have that I will stick to
            // squares.
            //auto width = static_cast<int>(carConfig.wheelRadius * GAME_UNIT_TO_PIXELS * 2);
            //auto height = static_cast<int>(carConfig.wheelWidth * GAME_UNIT_TO_PIXELS / 2);
            common::uint16 height = 5;
            common::uint16 width = 5;

            auto bits = graphics::TextureManager::generateBits(width, height, components::PixelRGBA{0, 0, 0, alpha});
            // TODO: Need to create skid texture data as it should be and set it.
            mTextureManager->updateSubTexture(skidMarksTexture,
                                              static_cast<GLint>(skidPos.x - width / 2.0f),
                                              static_cast<GLint>(skidPos.y - height / 2.0f),
                                              width, height, bits);
        }

        const components::Camera &SceneManager::getCamera() const {
            return mCamera;
        }

        void SceneManager::zoom(common::real32 distance) {
            mCamera.z = 1 / distance;
        }

        void SceneManager::lookAt(common::real32 x, common::real32 y) {
            mCamera.x = x;
            mCamera.y = y;
        }

        void SceneManager::lookAt(common::real32 x, common::real32 y, common::real32 distance) {
            mCamera.x = x;
            mCamera.y = y;
            mCamera.z = 1 / distance;
        }

        const math::mat4 &SceneManager::getProjectionMatrix() const {
            return mProjectionMatrix;
        }

        const math::mat4 &SceneManager::getViewMatrix() const {
            return mViewMatrix;
        }

        common::uint16 SceneManager::getSpritesPerFrame() const {
            return mRenderedSpritesPerFrame;
        }

        common::uint16 SceneManager::getTexturesPerFrame() const {
            return mRenderedTexturesPerFrame;
        }

        common::real32 SceneManager::getViewWidth() const {
            return (mScreenBounds.xMax - mScreenBounds.xMin) * (1.0f / mCamera.z);
        }

        common::real32 SceneManager::getViewHeight() const {
            return (mScreenBounds.yMax - mScreenBounds.yMin) * (1.0f / mCamera.z);
        }

        bool SceneManager::visibleToCamera(const components::Shape &shape, const common::real32 halfViewWidth,
                                           const common::real32 halfViewHeight) const {
            auto x = false;

            auto xMin = shape.vertexA.x;
            auto xMax = shape.vertexC.x;

            auto viewXMin = mCamera.x - halfViewWidth;
            auto viewXMax = mCamera.x + halfViewWidth;

            if (xMin >= viewXMin && xMin <= viewXMax) {
                x = true;
            }

            if (xMax >= viewXMin && xMax <= viewXMax) {
                x = true;
            }

            // Object is bigger than the frustum, i.e., view is inside the object
            if (viewXMin >= xMin && viewXMax <= xMax) {
                x = true;
            }

            if (!x) {
                return false;
            }

            auto y = false;

            auto yMin = shape.vertexA.y;
            auto yMax = shape.vertexC.y;

            auto viewYMin = mCamera.y - halfViewHeight;
            auto viewYMax = mCamera.y + halfViewHeight;

            if (yMin >= viewYMin && yMin <= viewYMax) {
                y = true;
            }
            if (yMax >= viewYMin && yMax <= viewYMax) {
                y = true;
            }

            // Object is bigger than the frustum, i.e., view is inside the object
            if (viewYMin >= yMin && viewYMax <= yMax) {
                y = true;
            }

            return y;
        }

        void SceneManager::resetCounters() {
            mRenderedSpritesPerFrame = 0;
            mRenderedTexturesPerFrame = 0;
        }

    }
}