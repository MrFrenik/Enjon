 #ifndef ENJON_MESHOG_H
#define ENJON_MESHOG_H

#include <vector>

#include "GLEW/glew.h"
#include "Math/Maths.h"
#include "System/Types.h"
#include "Defines.h"

namespace Enjon { namespace Graphics { 

	///////////////////////////////////////////////////////////////////////////

	struct MeshVertex
	{
		MeshVertex(const EM::Vec3& _Position, const EM::Vec3& _Normal, const EM::Vec2& _UV)
			: 
			Position(_Position), 
			Normal(_Normal),
			UV(_UV)
		{}

		EM::Vec3 Position;
		EM::Vec3 Normal;
		EM::Vec2 UV;
	};

	typedef std::vector<MeshVertex> MeshVertexBuffer;
	typedef std::vector<int32> MeshVertexIndexBuffer;

	///////////////////////////////////////////////////////////////////////////

	class Mesh
	{
		public:
			Mesh()
				: VertexArrayObj(0)
				, VertexBuffer(0)
				, IndexBuffer(0)
				, NumIndicies(0)
			{}	

		void Init();
		void UploadData(const MeshVertexBuffer& Verticies, const MeshVertexIndexBuffer& Indicies);
		void Destroy();

		GLuint VertexArrayObj, VertexBuffer, IndexBuffer;
		Enjon::int32 NumIndicies;
	};
}}

#endif