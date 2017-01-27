#include "Graphics/Mesh.h"

namespace Enjon { namespace Graphics {

	Mesh::Mesh()
	{
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Bind()
	{
		glBindVertexArray(VAO);
	}

	void Mesh::Unbind()
	{
		glBindVertexArray(0);
	}

	void Mesh::Submit()
	{
		glDrawArrays(DrawType, 0, DrawCount);	
	}
}}