#include <numeric>
#include <cassert>

#include <GL/glew.h>

#include <oni-core/graphic/batch-renderer-2d.h>
#include <oni-core/graphic/texture-manager.h>
#include <oni-core/buffer/buffer.h>
#include <oni-core/buffer/index-buffer.h>
#include <oni-core/buffer/vertex-array.h>
#include <oni-core/component/geometry.h>

namespace oni {
    namespace graphic {
        BatchRenderer2D::BatchRenderer2D(const common::oniGLsizei maxSpriteCount,
                                         const common::oniGLint maxNumTextureSamplers,
                                         const common::oniGLsizei maxVertexSize,
                                         std::vector<component::BufferStructure> bufferStructure)
                : mIndexCount{0},
                  mMaxSpriteCount{maxSpriteCount},
                  mMaxVertexSize{maxVertexSize},
                  mMaxNumTextureSamplers{maxNumTextureSamplers} {

            // Each sprite has 6 indices.
            mMaxIndicesCount = mMaxSpriteCount * 6;

            auto maxUIntSize = std::numeric_limits<common::int32>::max();
            assert(mMaxIndicesCount < maxUIntSize);

            // Each sprite has 4 vertices (6 in reality but 4 of them share the same data).
            mMaxSpriteSize = mMaxVertexSize * 4;
            mMaxBufferSize = mMaxSpriteSize * mMaxSpriteCount;

            auto vbo = std::make_unique<buffer::Buffer>(std::vector<common::oniGLfloat>(), mMaxBufferSize,
                                                        GL_STATIC_DRAW);
            mVAO = std::make_unique<buffer::VertexArray>(std::move(vbo), bufferStructure);

            std::vector<common::oniGLuint> indices(static_cast<unsigned long>(mMaxIndicesCount));

            common::oniGLushort offset = 0;
            for (auto i = 0; i < mMaxIndicesCount; i += 6) {
                indices[i + 0] = offset + 0;
                indices[i + 1] = offset + 1;
                indices[i + 2] = offset + 2;

                indices[i + 3] = offset + 2;
                indices[i + 4] = offset + 3;
                indices[i + 5] = offset + 0;

                offset += 4;
            }

            mIBO = std::make_unique<buffer::IndexBuffer>(indices, mMaxIndicesCount);

            mBuffer = nullptr;
        }

        BatchRenderer2D::~BatchRenderer2D() {
            glDeleteBuffers(1, &mVDO);
        }

        void BatchRenderer2D::_begin() {
            mNextSamplerID = 0;
            mSamplers.clear();
            mTextures.clear();

            mVAO->bindVBO();
            // Data written to mBuffer has to match the structure of VBO.
            mBuffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        }

        void BatchRenderer2D::_submit(const component::Shape &position, const component::Appearance &appearance) {
            // Check if Buffer can handle the number of vertices.
            // TODO: This seems to trigger even in none-debug mode
            assert(mIndexCount + 6 < mMaxIndicesCount);

            auto buffer = static_cast<component::ColoredVertex *>(mBuffer);

            /** The vertices are absolute coordinates, there is no model matrix.
             *    b    c
             *    +----+
             *    |    |
             *    +----+
             *    a    d
             */

            // a.
            buffer->position = position.vertexA;
            buffer->color = appearance.color;
            buffer++;

            // b.
            buffer->position = position.vertexB;
            buffer->color = appearance.color;
            buffer++;

            // c.
            buffer->position = position.vertexC;
            buffer->color = appearance.color;
            buffer++;

            // d.
            buffer->position = position.vertexD;
            buffer->color = appearance.color;
            buffer++;

            // Update the mBuffer to point to the head.
            mBuffer = static_cast<void *>(buffer);

            // +6 as there are 6 vertices that makes up two adjacent triangles but those triangles are
            // defined by 4 vertices only.
            /**
             *    1 +---+ 0
             *      |  /|
             *      |/  |
             *    2 +---+ 3
             **/
            mIndexCount += 6;
        }

        void BatchRenderer2D::_submit(const component::Shape &position, const component::Texture &texture) {
            // Check if Buffer can handle the number of vertices.
            assert(mIndexCount + 6 < mMaxIndicesCount);

            auto samplerID = getSamplerID(texture.textureID);

            auto buffer = static_cast<component::TexturedVertex *>(mBuffer);

            buffer->position = position.vertexA;
            buffer->uv = texture.uv[0];
            buffer->samplerID = samplerID;
            buffer++;

            buffer->position = position.vertexB;
            buffer->uv = texture.uv[1];
            buffer->samplerID = samplerID;
            buffer++;

            buffer->position = position.vertexC;
            buffer->uv = texture.uv[2];
            buffer->samplerID = samplerID;
            buffer++;

            buffer->position = position.vertexD;
            buffer->uv = texture.uv[3];
            buffer->samplerID = samplerID;
            buffer++;

            mBuffer = static_cast<void *>(buffer);

            mIndexCount += 6;
        }

        void BatchRenderer2D::_submit(const component::Text &text) {
            auto buffer = static_cast<component::TexturedVertex *>(mBuffer);

            auto samplerID = getSamplerID(text.textureID);

            auto advance = 0.0f;

            auto scaleX = text.xScaling;
            auto scaleY = text.yScaling;

            common::real32 z = 1.f;

            for (common::uint32 i = 0; i < text.textContent.size(); i++) {
                assert(mIndexCount + 6 < mMaxIndicesCount);

                auto x0 = text.position.x + text.offsetX[i] / scaleX + advance;
                auto y0 = text.position.y + text.offsetY[i] / scaleY;
                auto x1 = x0 + text.width[i] / scaleX;
                auto y1 = y0 - text.height[i] / scaleY;

                auto u0 = text.uv[i].x;
                auto v0 = text.uv[i].y;
                auto u1 = text.uv[i].z;
                auto v1 = text.uv[i].w;

                buffer->position = math::vec3{x0, y0, z};
                buffer->uv = math::vec2{u0, v0};
                buffer->samplerID = samplerID;
                buffer++;

                buffer->position = math::vec3{x0, y1, z};
                buffer->uv = math::vec2{u0, v1};
                buffer->samplerID = samplerID;
                buffer++;

                buffer->position = math::vec3{x1, y1, z};
                buffer->uv = math::vec2{u1, v1};
                buffer->samplerID = samplerID;
                buffer++;

                buffer->position = math::vec3{x1, y0, z};
                buffer->uv = math::vec2{u1, v0};
                buffer->samplerID = samplerID;
                buffer++;

                advance += text.advanceX[i] / scaleX;
                mIndexCount += 6;
            }

            mBuffer = static_cast<void *>(buffer);
        }

        void BatchRenderer2D::_flush() {
            TextureManager::bindRange(0, mTextures);

            mVAO->bindVAO();
            mIBO->bind();

            glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, nullptr);

            mIBO->unbind();
            mVAO->unbindVAO();

            TextureManager::unbind();

            mIndexCount = 0;

        }

        void BatchRenderer2D::_end() {
            glUnmapBuffer(GL_ARRAY_BUFFER);
            mVAO->unbindVBO();
        }

        void BatchRenderer2D::reset() {
            end();
            flush();
            begin();
        }

        common::oniGLint BatchRenderer2D::getSamplerID(common::oniGLuint textureID) {
            auto it = std::find(mTextures.begin(), mTextures.end(), textureID);
            if (it == mTextures.end()) {
                /*
                 * This can be used to render more than max number of samplers, but it slows things down.
                if (mNextSamplerID > mMaxNumTextureSamplers) {
                    reset();
                }*/
                assert(mNextSamplerID <= mMaxNumTextureSamplers);

                mTextures.push_back(textureID);
                mSamplers.push_back(mNextSamplerID++);
                return mSamplers.back();
            } else {
                auto index = std::distance(mTextures.begin(), it);
                return mSamplers[index];
            }
        }

        std::vector<common::oniGLint> BatchRenderer2D::generateSamplerIDs() {
            std::vector<common::oniGLint> samplers;
            samplers.assign(static_cast<common::uint32>(mMaxNumTextureSamplers), 0);
            // Fill the vector with 0, 1, 2, 3, ...
            std::iota(samplers.begin(), samplers.end(), 0);
            return samplers;
        }
    }
}
