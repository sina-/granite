#pragma once

#include <glew.h>
#include <vector>

#include "../utils/file.h"
#include "../math/math.h"

namespace granite {
	namespace graphics {
		class Shader {
			GLuint m_ShaderID;
			const char* m_VertPath;
			const char* m_FragPath;

			GLuint load();

			GLint getUniformLocation(const GLchar * name);

		public:
			Shader(const char* vertPath, const char* fragPath);
			~Shader() { glDeleteProgram(m_ShaderID); };

			inline void enable() const { glUseProgram(m_ShaderID); }
			void disable() const { glUseProgram(0); }


			inline void setUniform1f(const GLchar* name, float value)
			{
				glUniform1f(getUniformLocation(name), value);
			};
			inline void setUniform2f(const GLchar* name, const math::vec2& vector)
			{
				glUniform2f(getUniformLocation(name), vector.x, vector.y);
			};
			inline void setUniform3f(const GLchar* name, const math::vec3& vector)
			{
				glUniform3f(getUniformLocation(name), vector.x, vector.y, vector.z);
			};
			inline void setUniform4f(const GLchar* name, const math::vec4& vector)
			{
				glUniform4f(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w);
			};
			inline void setUniform1i(const GLchar* name, int value)
			{
				glUniform1i(getUniformLocation(name), value);
			};
			inline void setUniformMat4(const GLchar* name, const math::mat4& matrix) {
				glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix.getArray());
			};
		};
	}
}
