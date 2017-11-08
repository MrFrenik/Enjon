#include "Graphics/Mesh.h"
#include "Asset/MeshAssetLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <IO/TinyLoader.h>

namespace Enjon 
{ 
	//=========================================================================

	Mesh::Mesh()
	{
	}

	//=========================================================================

	Mesh::~Mesh()
	{
	}

	Mesh::Mesh( const Enjon::String& filePath )
	{ 
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err;
		bool ret = tinyobj::LoadObj( &attrib, &shapes, &materials, &err, filePath.c_str( ) );

		// `err` may contain warning message.
		if ( !err.empty( ) )
		{
			std::cerr << err << std::endl;
		}

		if ( !ret )
		{
			exit( 1 );
		}

		// Loop over shapes
		for ( size_t s = 0; s < shapes.size( ); s++ )
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for ( size_t f = 0; f < shapes[ s ].mesh.num_face_vertices.size( ); f++ )
			{
				s32 fv = shapes[ s ].mesh.num_face_vertices[ f ];

				// Loop over vertices in the face.
				for ( size_t v = 0; v < fv; v++ )
				{
					// access to vertex
					tinyobj::index_t idx = shapes[ s ].mesh.indices[ index_offset + v ];
					f32 vx = attrib.vertices[ 3 * idx.vertex_index + 0 ];
					f32 vy = attrib.vertices[ 3 * idx.vertex_index + 1 ];
					f32 vz = attrib.vertices[ 3 * idx.vertex_index + 2 ];
					f32 nx = attrib.normals[ 3 * idx.normal_index + 0 ];
					f32 ny = attrib.normals[ 3 * idx.normal_index + 1 ];
					f32 nz = attrib.normals[ 3 * idx.normal_index + 2 ];
					f32 tx = attrib.texcoords[ 2 * idx.texcoord_index + 0 ];
					f32 ty = attrib.texcoords[ 2 * idx.texcoord_index + 1 ];

					Vert Vertex = { };
					Vertex.Position[ 0 ] = vx;
					Vertex.Position[ 1 ] = vy;
					Vertex.Position[ 2 ] = vz;
					Vertex.Normals[ 0 ] = nx;
					Vertex.Normals[ 1 ] = ny;
					Vertex.Normals[ 2 ] = nz;
					Vertex.UV[ 0 ] = tx;
					Vertex.UV[ 1 ] = ty;
					Vertex.Tangent[ 0 ] = 1.0f;
					Vertex.Tangent[ 1 ] = 0.0f;
					Vertex.Tangent[ 2 ] = 0.0f;

					Verticies.push_back( Vertex );
				}

				index_offset += fv;

				// per-face material
				shapes[ s ].mesh.material_ids[ f ];
			}
		}

		// Now calculate tangents for each vert in mesh
		for ( int i = 0; i < Verticies.size( ); i += 3 )
		{
			auto& Vert1 = Verticies.at( i );
			auto& Vert2 = Verticies.at( i + 1 );
			auto& Vert3 = Verticies.at( i + 2 );

			Vec3 pos1 = Vec3( Vert1.Position[ 0 ], Vert1.Position[ 1 ], Vert1.Position[ 2 ] );
			Vec3 pos2 = Vec3( Vert2.Position[ 0 ], Vert2.Position[ 1 ], Vert2.Position[ 2 ] );
			Vec3 pos3 = Vec3( Vert3.Position[ 0 ], Vert3.Position[ 1 ], Vert3.Position[ 2 ] );

			Vec2 uv1 = Vec2( Vert1.UV[ 0 ], Vert1.UV[ 1 ] );
			Vec2 uv2 = Vec2( Vert2.UV[ 0 ], Vert2.UV[ 1 ] );
			Vec2 uv3 = Vec2( Vert3.UV[ 0 ], Vert3.UV[ 1 ] );

			// calculate tangent vectors of both triangles
			Vec3 tangent;

			// - triangle 1
			Vec3 edge1 = pos2 - pos1;
			Vec3 edge2 = pos3 - pos1;
			Enjon::Vec2 deltaUV1 = uv2 - uv1;
			Enjon::Vec2 deltaUV2 = uv3 - uv1;

			f32 f = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y );

			tangent.x = f * ( deltaUV2.y * edge1.x - deltaUV1.y * edge2.x );
			tangent.y = f * ( deltaUV2.y * edge1.y - deltaUV1.y * edge2.y );
			tangent.z = f * ( deltaUV2.y * edge1.z - deltaUV1.y * edge2.z );
			tangent = Vec3::Normalize( tangent );

			// Set tangents for the verticies
			Vert1.Tangent[ 0 ] = tangent.x;
			Vert1.Tangent[ 1 ] = tangent.y;
			Vert1.Tangent[ 2 ] = tangent.z;

			Vert2.Tangent[ 0 ] = tangent.x;
			Vert2.Tangent[ 1 ] = tangent.y;
			Vert2.Tangent[ 2 ] = tangent.z;

			Vert3.Tangent[ 0 ] = tangent.x;
			Vert3.Tangent[ 1 ] = tangent.y;
			Vert3.Tangent[ 2 ] = tangent.z;
		}

		// Create and upload mesh data
		glGenBuffers( 1, &VBO );
		glBindBuffer( GL_ARRAY_BUFFER, VBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( Vert ) * Verticies.size( ), &Verticies[ 0 ], GL_STATIC_DRAW );

		glGenVertexArrays( 1, &VAO );
		glBindVertexArray( VAO );

		// Position
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Position ) );
		// Normal
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Normals ) );
		// Tangent
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Tangent ) );
		// UV
		glEnableVertexAttribArray( 3 );
		glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, UV ) );

		// Unbind VAO
		glBindVertexArray( 0 );

		// Set draw type
		DrawType = GL_TRIANGLES;
		// Set draw count
		DrawCount = Verticies.size( );
	}

	//=========================================================================

	void Mesh::Bind() const
	{
		glBindVertexArray(VAO);
	}

	//=========================================================================

	void Mesh::Unbind() const
	{
		glBindVertexArray(0);
	}

	//=========================================================================

	void Mesh::Submit() const
	{
		glDrawArrays(DrawType, 0, DrawCount);	
	}

	//=========================================================================
			
	Result Mesh::Serialize( Enjon::ByteBuffer& buffer )
	{ 
		// Write out size of verticies
		buffer.Write< usize >( Verticies.size( ) );

		// Write out verticies
		for ( auto& v : Verticies )
		{
			// Position
			buffer.Write< f32 >( v.Position[ 0 ] );
			buffer.Write< f32 >( v.Position[ 1 ] );
			buffer.Write< f32 >( v.Position[ 2 ] );

			// Normal
			buffer.Write< f32 >( v.Normals[ 0 ] );
			buffer.Write< f32 >( v.Normals[ 1 ] );
			buffer.Write< f32 >( v.Normals[ 2 ] );

			// Tangent
			buffer.Write< f32 >( v.Tangent[ 0 ] );
			buffer.Write< f32 >( v.Tangent[ 1 ] );
			buffer.Write< f32 >( v.Tangent[ 2 ] );

			// UV
			buffer.Write< f32 >( v.UV[ 0 ] );
			buffer.Write< f32 >( v.UV[ 1 ] );
		} 

		return Result::SUCCESS;
	}

	//=========================================================================

	Result Mesh::Deserialize( Enjon::ByteBuffer& buffer )
	{
		// Get size of buffer
		usize amount = buffer.Read< usize >( );

		for ( usize i = 0; i < amount; ++i )
		{
			Vert v;

			// Position
			v.Position[ 0 ] = buffer.Read< f32 >( );
			v.Position[ 1 ] = buffer.Read< f32 >( );
			v.Position[ 2 ] = buffer.Read< f32 >( );
			
			// Normal
			v.Normals[ 0 ] = buffer.Read< f32 >( );
			v.Normals[ 1 ] = buffer.Read< f32 >( );
			v.Normals[ 2 ] = buffer.Read< f32 >( );
			
			// Tangent
			v.Tangent[ 0 ] = buffer.Read< f32 >( );
			v.Tangent[ 1 ] = buffer.Read< f32 >( );
			v.Tangent[ 2 ] = buffer.Read< f32 >( );
			
			// UV
			v.UV[ 0 ] = buffer.Read< f32 >( );
			v.UV[ 1 ] = buffer.Read< f32 >( );

			// Push back vert
			Verticies.push_back( v );
		}

		// Create and upload mesh data
		glGenBuffers( 1, &VBO );
		glBindBuffer( GL_ARRAY_BUFFER, VBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( Vert ) * Verticies.size( ), &Verticies[ 0 ], GL_STATIC_DRAW );

		glGenVertexArrays( 1, &VAO );
		glBindVertexArray( VAO );

		// Position
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Position ) );
		// Normal
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Normals ) );
		// Tangent
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, Tangent ) );
		// UV
		glEnableVertexAttribArray( 3 );
		glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, sizeof( Vert ), ( void* )offsetof( Vert, UV ) );

		// Unbind VAO
		glBindVertexArray( 0 );

		// Set draw type
		DrawType = GL_TRIANGLES;
		// Set draw count
		DrawCount = Verticies.size( );

		return Result::SUCCESS; 
	}
}