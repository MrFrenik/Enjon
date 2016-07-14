#include "Graphics/DeferredRenderer.h"

namespace Enjon { namespace Graphics {

	/* Constructor */
	DeferredRenderer::DeferredRenderer(uint32 Width, uint32 Height, FrameBufferObject* FBO, EG::GLSLProgram* Shader)
		: m_width(Width) 
		, m_height(Height)
		, m_fbo(FBO)
		, m_shader(Shader)
	{
		// Get the handles from the shader
		GLint shaderID = m_shader->GetProgramID();
		m_diffuseID = glGetUniformLocationARB(shaderID,"Diffuse");
		m_positionID = glGetUniformLocationARB(shaderID,"Position");
		m_normalsID = glGetUniformLocationARB(shaderID,"Normals");
	}

	/* Destructor */
	DeferredRenderer::~DeferredRenderer()
	{
	}

	void DeferredRenderer::Render()
	{
		//Projection setup
		// glMatrixMode(GL_PROJECTION);
		// glPushMatrix();
		// glLoadIdentity();
		// glOrtho(0,m_width,0,m_height,0.1f,2);	
		
		//Model setup
		// glMatrixMode(GL_MODELVIEW);
		// glPushMatrix();
		
		glUseProgramObjectARB(m_shader->GetProgramID());

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_fbo->GetDiffuseTexture());
		glUniform1iARB (m_diffuseID, 0);
		
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_fbo->GetPositionTexture());
		glUniform1iARB (m_positionID, 1);
		
		glActiveTextureARB(GL_TEXTURE2_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_fbo->GetNormalsTexture());
		glUniform1iARB (m_normalsID, 2);

		// Render the quad
		glLoadIdentity();
		glColor3f(1,1,1);
		glTranslatef(0,0,-1.0);
		
		glBegin(GL_QUADS);
			glTexCoord2f( 0, 0 );
			glVertex3f(0.0f, 0.0f, 0.0f);
			glTexCoord2f(1, 0);
			glVertex3f((float) m_width, 0.0f, 0.0f);
			glTexCoord2f(1, 1);
			glVertex3f((float) m_width, (float) m_height, 0.0f);
			glTexCoord2f( 0, 1 );
			glVertex3f(0.0f, (float) m_height, 0.0f);
		glEnd();
		
		// Reset OpenGL state
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTextureARB(GL_TEXTURE2_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgramObjectARB(0);
		
		//Reset to the matrices	
		// glMatrixMode(GL_PROJECTION);
		// glPopMatrix();
		// glMatrixMode(GL_MODELVIEW);
		// glPopMatrix();

	}
}}
