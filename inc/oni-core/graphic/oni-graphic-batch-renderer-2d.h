#pragma once

#include <map>

#include <oni-core/graphic/oni-graphic-renderer-2d.h>
#include <oni-core/graphic/buffer/oni-graphic-buffer-data.h>
#include <oni-core/common/oni-common-typedef.h>

namespace oni {
    namespace buffer {
        class IndexBuffer;

        class VertexArray;
    }

    namespace component {
        struct Texture;

        struct Text;

        struct Appearance;
    }

    namespace graphic {

        class BatchRenderer2D : public Renderer2D {
        public:
            BatchRenderer2D(common::oniGLsizei maxSpriteCount,
                            common::oniGLsizei maxNumTextureSamplers,
                            common::oniGLsizei maxVertexSize,
                            const std::vector<graphic::BufferStructure> &bufferStructures,
                            PrimitiveType type
            );

            ~BatchRenderer2D() override;

            BatchRenderer2D(const BatchRenderer2D &) = delete;

            BatchRenderer2D &
            operator=(BatchRenderer2D &) = delete;

            std::vector<common::oniGLint>
            generateSamplerIDs();

        private:
            void
            _begin() override;

            void
            _submit(const component::WorldP3D &,
                    const component::Heading &,
                    const component::Scale &,
                    const component::Appearance &,
                    const component::Texture &) override;

            void
            _submit(const component::Text &,
                    const component::WorldP3D &) override;

            void
            _flush() override;

            void
            _end() override;

            void
            reset();

            // TODO: checkout texture arrays.
            /**
             * There are 0, 1, ..., mMaxNumTextureSamplers texture samplers available.
             * Each texture is assigned one and the id to the sampler is saved as part of vertex data
             * in the vertex buffer. During rendering in the shader the proper sampler is selected based
             * on the sampler id in the buffer.
             */
            common::oniGLint
            getSamplerID(common::oniGLuint textureID);

        private:
            PrimitiveType mPrimitiveType{PrimitiveType::UNKNOWN};
            // Actual number of indices used.
            common::oniGLsizei mIndexCount{0};

            common::oniGLsizei mMaxPrimitiveCount{0};
            common::oniGLsizei mMaxVertexSize{0};
            const common::oniGLint mMaxNumTextureSamplers;
            common::oniGLsizei mMaxIndicesCount{0};
            common::oniGLsizei mMaxPrimitiveSize{0};

            std::unique_ptr<buffer::VertexArray> mVertexArray;
            std::unique_ptr<buffer::IndexBuffer> mIndexBuffer;

            common::oniGLint mNextSamplerID{0};
            std::vector<common::oniGLint> mSamplers{};
            std::vector<common::oniGLuint> mTextures{};

            // The buffer that will hold component::Vertex data, or its variants, in the batch.
            void *mBuffer;
        };

    }
}