#include "Graphics/MeshOG.h"


namespace Enjon { namespace Graphics {

		void Mesh::Init()
		{
			// Create arrays and buffers
			glGenVertexArrays(1, &VertexArrayObj);
			glGenBuffers(1, &VertexBuffer);
			glGenBuffers(1, &IndexBuffer);

			// Bind arrays and buffers
			glBindVertexArray(VertexArrayObj);
			glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);

			// Set up attribute for mesh vertex position
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, Position));

			// Set up attribute for mesh vertex normal
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)(offsetof(MeshVertex, Normal)));

			// Set up attribute for mesh texture coords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)(offsetof(MeshVertex, UV)));

			// Unbind vertex array
			glBindVertexArray(0);
		}

		void Mesh::UploadData(const MeshVertexBuffer& Verticies, const MeshVertexIndexBuffer& Indicies)
		{
			if (Verticies.empty() || Indicies.empty())
			{
				return;
			}

			// Bind array
			glBindVertexArray(VertexArrayObj);

			// Bind array Buffer and upload to GPU
			glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * Verticies.size(), &Verticies[0], GL_STATIC_DRAW);

			// Bind element buffer and upload to GPU
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Enjon::int32) * Indicies.size(), &Indicies[0], GL_STATIC_DRAW);
			NumIndicies = Indicies.size();

			// Unbind vertex array
			glBindVertexArray(0);
		}

		void Mesh::Destroy()
		{
			// Delete buffers and array
			glDeleteBuffers(1, &IndexBuffer);
			glDeleteBuffers(1, &VertexBuffer);
			glDeleteVertexArrays(1, &VertexArrayObj);
		}

}}
