#include <oni-core/buffers/buffer.h>
#include <oni-core/components/buffer.h>

#include <cassert>

namespace oni {
    namespace buffers {
        Buffer::Buffer(const std::vector<GLfloat> &data, GLsizeiptr dataSize, GLenum usage,
                       common::BufferStructures bufferStructures)
                : mBufferStructures(std::move(bufferStructures)) {
            // Check for supported usages.
            assert(usage == GL_STATIC_DRAW || usage == GL_DYNAMIC_DRAW);

            // TODO: Check this comment on how to use OpenGL4.5 to create buffers:
            // http://stackoverflow.com/a/21652955/558366
            // Also this example can help:
            // https://bcmpinc.wordpress.com/2015/10/07/copy-a-texture-to-screen/
            // TODO: Another guide to using modern OpenGL that covers more than just
            // Buffers: https://github.com/Fennec-kun/Guide-to-Modern-OpenGL-Functions
            glGenBuffers(1, &mBufferID);
            bind();
            // Use the data for GL_STATIC_DRAW, otherwise pass nullptr for GL_DYNAMIC_DRAW.
            auto dataPtr = !data.empty() ? data.data() : nullptr;
            glBufferData(GL_ARRAY_BUFFER, dataSize, dataPtr, usage);

            GLint actualSize{0};
            glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &actualSize);
            if (dataSize != actualSize) {
                glDeleteBuffers(1, &mBufferID);
                throw std::runtime_error("Could not allocate vertex buffer!");
            }

            unbind();
        }

        Buffer::~Buffer() { glDeleteBuffers(1, &mBufferID); }

        void Buffer::bind() { glBindBuffer(GL_ARRAY_BUFFER, mBufferID); }

        void Buffer::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

        const common::BufferStructures &Buffer::getBufferStructure() const {
            return mBufferStructures;
        }
    }
}
