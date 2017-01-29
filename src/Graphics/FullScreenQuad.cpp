#include "Graphics/FullScreenQuad.h"

namespace Enjon { namespace Graphics { 

	FullScreenQuad::FullScreenQuad()
	{
		static const GLfloat vertexBufferData[] = 
		{
			-1.0f, -1.0f, 0.0f, 
			1.0f, -1.0f, 0.0f, 
			-1.0f, -1.0f, 0.0f, 
			-1.0f, 1.0f, 0.0f, 
			1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f
		};

		glGenVertexArrays(1, &mVAO);
		glGenBuffers(1, &mVBO);
		glBindVertexArray(mVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), &vertexBufferData, GL_STATIC_DRAW);		
		glEnableVertexAttribArray(0);
	  	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	    glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	    glBindVertexArray(0);
	}

	FullScreenQuad::~FullScreenQuad()
	{
		// Clean up mVAO
	}

	void FullScreenQuad::Bind()
	{
		glBindVertexArray(mVAO);
	}

	void FullScreenQuad::Unbind()
	{
		glBindVertexArray(0);
	}

	void FullScreenQuad::Submit()
	{
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}}
