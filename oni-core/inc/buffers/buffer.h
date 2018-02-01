#pragma once

#include <GL/glew.h>
#include <stdexcept>
#include <vector>
#include <graphics/renderable2d.h>

namespace oni {
    namespace buffers {
        using namespace graphics;
        /*
         * Great tutorial on OpenGL buffers: https://open.gl/drawing
         */
        class Buffer {
            GLuint m_BufferID;
            BufferStructures m_BufferStructures;

        public:
            Buffer(const std::vector<GLfloat> &data, GLsizeiptr dataSize, GLenum usage,
                   BufferStructures bufferStructures);

            ~Buffer() { glDeleteBuffers(1, &m_BufferID); }

            Buffer &operator=(const Buffer &) = delete;

            Buffer &operator=(Buffer &) = delete;

            inline void bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_BufferID); }

            inline void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

            inline const auto getBufferStructure() const { return &m_BufferStructures; }
        };

    }
}
