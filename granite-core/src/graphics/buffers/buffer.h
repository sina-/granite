#pragma once

#include <glew.h>
#include <stdexcept>
#include <vector>

namespace granite {
	namespace graphics {
		class Buffer {
			GLuint m_BufferID;
			GLuint m_ComponentCount;
			GLsizei m_Size;

		public:
			Buffer(const std::vector<GLfloat>& data, GLuint componentCount, GLsizei size = 1);
			~Buffer() { glDeleteBuffers(m_Size, &m_BufferID); }

			inline void bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_BufferID); }
			inline void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

			inline GLuint getComponentCount() const { return m_ComponentCount; }
		};

	}
}
