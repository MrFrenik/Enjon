#include "Graphics/Mesh.h"
#include "Asset/MeshAssetLoader.h"

namespace Enjon {

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
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}

	void Mesh::Submit()
	{
		glDrawArrays(DrawType, 0, DrawCount);	
	}
}