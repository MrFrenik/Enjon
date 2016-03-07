#ifndef DEFERRED_RENDERER_H
#define DEFERRED_RENDERER_H

#include "System/Types.h"
#include "Defines.h"
#include "Graphics/FrameBufferObject.h"
#include "Graphics/GLSLProgram.h"

#include "GLEW/glew.h"

namespace Enjon { namespace Graphics {

	/**
	*	This object is used to render a big screen sized quad with the deferred rendering shader applied on it.
	*/
	class DeferredRenderer
	{
	public:
		DeferredRenderer(uint32 Width, uint32 Height, FrameBufferObject* FBO, EG::GLSLProgram* Shader);
		~DeferredRenderer();

		void	Render();

	private:
		EG::GLSLProgram* 		m_shader; 				// Deferred rendering shader
		EG::FrameBufferObject*	m_fbo; 					// A pointer to the FBO render texture that contains diffuse, normals and positions

		Enjon::uint32			m_width; 				// width
		Enjon::uint32			m_height; 				// height

		GLuint					m_diffuseID; 			// Diffuse texture handle for the shader
		GLuint					m_positionID; 			// Position texture handle for the shader
		GLuint					m_normalsID; 			// Normals texture handle for the shader
	};


}}


#endif