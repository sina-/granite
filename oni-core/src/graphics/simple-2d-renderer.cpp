#include "graphics/simple-2d-renderer.h"

namespace oni {
	namespace graphics {
		void Simple2DRenderer::submit(const components::Position &position, const components::Appearance &appearance)
		{
//			m_RenderQueue.push_back(renderable);
		}

		void Simple2DRenderer::flush() 
		{
            /*
			while (!m_RenderQueue.empty()) {
				auto renderable = m_RenderQueue.front().get();

				auto vao = renderable->getVAO();
				auto ibo = renderable->getIBO();

				vao->bind();
				ibo->bind();*/

//				renderable->getShader().setUniformMat4("ml_matrix", renderable->getModelMatrix());
                // TODO: Store the vertex type, GL_UNSIGNED_SHORT, as the part of renderable.
//				glDrawElements(GL_TRIANGLES, ibo->getCount(), GL_UNSIGNED_SHORT, nullptr);

/*				ibo->unbind();
				vao->unbind();*/

				/*
				// Odd, but queue has no method to remove and return the front element, therefore
                // have to remove it manually after use.
				m_RenderQueue.pop_front();
			}
				*/
		}

	}
}
