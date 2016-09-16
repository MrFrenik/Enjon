#ifndef ENJON_MESH_H
#define ENJON_MESH_H

#include <System/Types.h>
#include <Defines.h>
#include <Math/Maths.h>
#include <Graphics/Vertex.h>

#include <vector>

namespace Enjon { namespace Graphics { 

	struct Mesh
	{
		struct Data
		{
			GLenum DrawType = GL_TRIANGLES;
			std::vector<Vertex3> Verticies;
			std::vector<u32> Indicies;	
		};
	};

}}


#endif