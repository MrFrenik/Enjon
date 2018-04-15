#include "Graphics/Mesh.h"
#include "Asset/MeshAssetLoader.h"
#include "Serialize/ObjectArchiver.h"

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
		Release( );
	}

	//=========================================================================

	Result Mesh::Release( )
	{
		// Clear all submesh data
		mSubMeshes.clear( ); 

		return Result::SUCCESS;
	}

	//=========================================================================

	const Vector<SubMesh>& Mesh::GetSubmeshes( ) const
	{
		return mSubMeshes;
	}

	//=========================================================================

	u32 Mesh::GetSubMeshCount( ) const
	{
		return mSubMeshes.size( );
	}

	//=========================================================================
			
	Result Mesh::SerializeData( ByteBuffer* buffer ) const
	{ 
		std::cout << "Serializing mesh...\n";

		// Write out submesh count
		buffer->Write< u32 >( mSubMeshes.size( ) );

		// Write out each submesh to file
		for ( auto& sm : mSubMeshes )
		{
			sm.SerializeData( buffer );
		}

		return Result::SUCCESS;
	}

	//=========================================================================

	Result Mesh::DeserializeData( ByteBuffer* buffer )
	{
		std::cout << "Deserializing mesh...\n";

		// Read in number of submeshes
		u32 numSubMeshes = buffer->Read< u32 >( );

		// Deserialize all submesh data
		for ( u32 i = 0; i < numSubMeshes; ++i )
		{
			// Construct new submesh
			SubMesh sm;
			// Deserialize submesh data
			sm.DeserializeData( buffer );
			// Push back submesh
			mSubMeshes.push_back( sm );
		}

		return Result::SUCCESS;
	} 

	SubMesh::SubMesh( )
	{
	}

	SubMesh::~SubMesh( )
	{ 
		Release( );
	}
	
	Result SubMesh::Release( )
	{
		if ( mVBO )
		{
			// TODO(): Get rid of all exposed OpenGL/ DX API calls
			glDeleteBuffers( 1, &mVBO ); 
		}

		if ( mVAO )
		{
			glDeleteBuffers( 1, &mVBO ); 
		}

		mVerticies.clear( );
		mIndicies.clear( );

		return Result::SUCCESS;
	}

	//=========================================================================

	void SubMesh::Bind() const
	{
		glBindVertexArray(mVAO);
	}

	//=========================================================================

	void SubMesh::Unbind() const
	{
		glBindVertexArray(0);
	}

	//=========================================================================

	void SubMesh::Submit() const
	{
		glDrawArrays(mDrawType, 0, mDrawCount);	
	}

	//=========================================================================

	u32 SubMesh::GetDrawCount( ) const
	{ 
		return mDrawCount;
	}

	//=========================================================================

	u32 SubMesh::GetVAO( ) const
	{ 
		return mVAO;
	}

	//=========================================================================

	u32 SubMesh::GetVBO( ) const
	{ 
		return mVBO;
	}

	//=========================================================================

	u32 SubMesh::GetIBO( ) const
	{ 
		return mIBO;
	}

	//=========================================================================

	Result SubMesh::SerializeData( ByteBuffer* buffer ) const
	{
		//// Write out size of verticies
		buffer->Write< usize >( mVerticies.size( ) );

		// Write out verticies
		for ( auto& v : mVerticies )
		{
			// Position
			buffer->Write< f32 >( v.Position[ 0 ] );
			buffer->Write< f32 >( v.Position[ 1 ] );
			buffer->Write< f32 >( v.Position[ 2 ] );

			// Normal
			buffer->Write< f32 >( v.Normals[ 0 ] );
			buffer->Write< f32 >( v.Normals[ 1 ] );
			buffer->Write< f32 >( v.Normals[ 2 ] );

			// Tangent
			buffer->Write< f32 >( v.Tangent[ 0 ] );
			buffer->Write< f32 >( v.Tangent[ 1 ] );
			buffer->Write< f32 >( v.Tangent[ 2 ] );

			// UV
			buffer->Write< f32 >( v.UV[ 0 ] );
			buffer->Write< f32 >( v.UV[ 1 ] );
		} 

		return Result::SUCCESS;
	}

	//=========================================================================

	Result SubMesh::DeserializeData( ByteBuffer* buffer ) 
	{
		// Release previous data ( if any )
		Release( );

		// Get size of verts from archiver
		usize vertCount = buffer->Read< usize >( );

		// Read in verts from archiver
		for ( usize i = 0; i < vertCount; ++i )
		{
			Vert v;

			// Position
			v.Position[ 0 ] = buffer->Read< f32 >( );
			v.Position[ 1 ] = buffer->Read< f32 >( );
			v.Position[ 2 ] = buffer->Read< f32 >( );
			
			// Normal
			v.Normals[ 0 ] = buffer->Read< f32 >( );
			v.Normals[ 1 ] = buffer->Read< f32 >( );
			v.Normals[ 2 ] = buffer->Read< f32 >( );
			
			// Tangent
			v.Tangent[ 0 ] = buffer->Read< f32 >( );
			v.Tangent[ 1 ] = buffer->Read< f32 >( );
			v.Tangent[ 2 ] = buffer->Read< f32 >( );
			
			// UV
			v.UV[ 0 ] = buffer->Read< f32 >( );
			v.UV[ 1 ] = buffer->Read< f32 >( );

			// Push back vert
			mVerticies.push_back( v );
		} 

		// Create and upload mesh data
		glGenBuffers( 1, &mVBO );
		glBindBuffer( GL_ARRAY_BUFFER, mVBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( Vert ) * mVerticies.size( ), &mVerticies[ 0 ], GL_STATIC_DRAW );

		glGenVertexArrays( 1, &mVAO );
		glBindVertexArray( mVAO );

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

		// Unbind mVAO
		glBindVertexArray( 0 );

		// Set draw type
		mDrawType = GL_TRIANGLES;
		// Set draw count
		mDrawCount = mVerticies.size( );

		return Result::SUCCESS; 
	} 

	//=========================================================================
}


/* 
	Mesh holds submeshes - Renderable holds Mesh as well as Vector<AssetHandle<Material>> that goes along with each element of the submesh 

	class Mesh
	{
		private:
			Vector<SubMesh> mSubmeshes;
	};

	class Renderable
	{
		private:
			AssetHandle<Mesh> mMesh;
			Vector<AssetHandle<Material>> mMaterials;
	}; 

	// MeshAssetLoader creates mesh by processing aiScene of all individual submeshes in a mesh, composites them together and then returns Asset* as Mesh for asset subsystem 

*/
