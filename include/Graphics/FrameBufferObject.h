#ifndef FRAME_BUFFER_OBJECT_H
#define FRAME_BUFFER_OBJECT_H

#include "System/Types.h"
#include "GLEW/glew.h"

namespace Enjon { namespace Graphics { 

	/**
	*	A Frame Buffer Object is used by OpenGL to render into a texture. Specifically this implementation assumes that the
	*	rendered model will provide diffuse, position and normal at the same time in a MRT fashion
	*/
	class FrameBufferObject
	{
		public:
			FrameBufferObject(uint32 Width, uint32 Height);
			~FrameBufferObject();

			void Begin();
			void End();


			GLuint inline GetDiffuseTexture() const 	{ return m_diffuseTexture; } 
			GLuint inline GetPositionTexture() const 	{ return m_positionTexture; } 
			GLuint inline GetNormalsTexture() const 	{ return m_normalsTexture; } 

		private:
		
			GLuint			m_fbo; 					// The FBO ID

			GLuint			m_diffuseRT; 			// The diffuse render target
			GLuint			m_positionRT; 			// The position render target
			GLuint			m_normalsRT; 			// The normals render target
			GLuint			m_depthBuffer; 			// Depth buffer handle

			unsigned int	m_diffuseTexture; 		// The OpenGL texture for the diffuse render target
			unsigned int	m_positionTexture; 		// The OpenGL texture for the position render target
			unsigned int	m_normalsTexture; 		// The OpenGL texture for the normals render target

			unsigned int	m_width; 				// FBO width
			unsigned int	m_height; 				// FBO height
	};

}}

#endif