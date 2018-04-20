#include "Graphics/Mesh.h"
#include "Asset/MeshAssetLoader.h"
#include "Serialize/ObjectArchiver.h"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <IO/TinyLoader.h>

namespace Enjon 
{ 
	//=========================================================================

	usize VertexDataDeclaration::GetSizeInBytes( ) const
	{
		return mSizeInBytes;
	} 

	//=========================================================================

	s32 VertexDataDeclaration::GetByteOffset( const u32& attributeIndex ) const
	{ 
		// Error check
		if ( attributeIndex >= mDecl.size() )
		{
			return -1;
		}

		// Recursively calculate offset
		s32 totalOffset = 0;
		for ( u32 i = 0; i < attributeIndex; ++i )
		{ 
			totalOffset += GetByteOffset( i );
		}

		return totalOffset;
	}

	//=========================================================================

	void VertexDataDeclaration::Add( const VertexAttributeFormat& format )
	{ 
		// Push back format into decl
		mDecl.push_back( format );

		// Recalculate total size in bytes of this declaration
		CalculateSizeInBytes();
	}

	//=========================================================================

	void VertexDataDeclaration::CalculateSizeInBytes( )
	{
		// Iterate through all formats in delcarations and calculate total size
		usize sz = 0;
		for ( auto& format : mDecl )
		{
			switch ( format )
			{
				case VertexAttributeFormat::Float4:			{ sz += 4 * sizeof( f32 ); } break; 
				case VertexAttributeFormat::Float3:			{ sz += 3 * sizeof( f32 ); } break; 
				case VertexAttributeFormat::Float2:			{ sz += 2 * sizeof( f32 ); } break; 
				case VertexAttributeFormat::Float:			{ sz += 1 * sizeof( f32 ); } break; 
				case VertexAttributeFormat::UnsignedInt4:	{ sz += 4 * sizeof( u32 ); } break;
				case VertexAttributeFormat::UnsignedInt3:	{ sz += 3 * sizeof( u32 ); } break;
				case VertexAttributeFormat::UnsignedInt2:	{ sz += 2 * sizeof( u32 ); } break;
				case VertexAttributeFormat::UnsignedInt:	{ sz += 1 * sizeof( u32 ); } break; 
			} 
		}

		// Set size in bytes
		mSizeInBytes = sz;
	}

	//=========================================================================

	Result VertexDataDeclaration::SerializeData( ByteBuffer* buffer ) const
	{
		// Write out element size of decl
		buffer->Write< u32 >( mDecl.size( ) );

		// Write out decl info 
		for ( auto& f : mDecl )
		{
			buffer->Write< u32 >( u32( f ) );
		}

		return Result::SUCCESS;
	}

	//=========================================================================

	Result VertexDataDeclaration::DeserializeData( ByteBuffer* buffer )
	{
		// Read in element size of decl
		u32 sz = buffer->Read< u32 >( );

		// Read in declarations and push back into decl
		for ( u32 i = 0; i < sz; ++i )
		{
			Add( (VertexAttributeFormat)buffer->Read< u32 >( ) );
		}

		return Result::SUCCESS; 
	}

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

	const VertexDataDeclaration& Mesh::GetVertexDeclaration( )
	{
		return mVertexDecl;
	}

	//=========================================================================

	void Mesh::SetVertexDecl( const VertexDataDeclaration& decl )
	{
		mVertexDecl = decl;
	}

	//=========================================================================
			
	Result Mesh::SerializeData( ByteBuffer* buffer ) const
	{ 
		std::cout << "Serializing mesh...\n";

		// Write out vertex decl
		mVertexDecl.SerializeData( buffer );

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

		// Read in vertex decl
		mVertexDecl.DeserializeData( buffer );

		// Read in number of submeshes
		u32 numSubMeshes = buffer->Read< u32 >( );

		// Deserialize all submesh data
		for ( u32 i = 0; i < numSubMeshes; ++i )
		{
			// Emplace new submesh
			mSubMeshes.emplace_back( this );
			// pointer to submesh
			SubMesh* sm = &mSubMeshes.back( );
			// Deserialize submesh data
			sm->DeserializeData( buffer );
		}

		return Result::SUCCESS;
	} 

	SubMesh::SubMesh( )
	{
		mVertexData = new ByteBuffer( );
	}

	SubMesh::SubMesh( const SubMesh& other )
	{
		mMesh = other.mMesh;
		mDrawType = other.mDrawType; 
		mDrawCount = other.mDrawCount;
		mDrawStart = other.mDrawStart;
		mVertexData = new ByteBuffer( );
		mVertexData = other.mVertexData;
	}

	SubMesh::SubMesh( Mesh* mesh )
	{
		mMesh = mesh;
		mVertexData = new ByteBuffer( );
	}

	SubMesh::~SubMesh( )
	{ 
		Release( );

		if ( mVertexData )
		{
			delete( mVertexData );
		}
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
		// Write out size of data
		buffer->Write< u32 >( mVertexData->GetSize( ) );

		// Write out vertex data
		buffer->AppendBuffer( *mVertexData );

		return Result::SUCCESS;
	}

	//=========================================================================

	Result SubMesh::DeserializeData( ByteBuffer* buffer ) 
	{
		// Release previous data ( if any )
		Release( );

		// Need to be able to read back original buffer from the buffer passed in
		u32 byteSize = buffer->Read< u32 >( );

		// God...this is horrible, but it should work
		for ( u32 i = 0; i < byteSize; ++i )
		{
			mVertexData->Write< u8 >( buffer->Read< u8 >( ) );
		}

		// If owning mesh doesn't exit, then return failure
		if ( !mMesh )
		{
			return Result::FAILURE;
		} 

		// Get vertex data decl from owning mesh
		const VertexDataDeclaration& vertDecl = mMesh->GetVertexDeclaration( );

		// Create and upload mesh data
		glGenBuffers( 1, &mVBO );
		glBindBuffer( GL_ARRAY_BUFFER, mVBO );
		glBufferData( GL_ARRAY_BUFFER, mVertexData->GetSize(), mVertexData->GetData(), GL_STATIC_DRAW );
 
		glGenVertexArrays( 1, &mVAO );
		glBindVertexArray( mVAO ); 

		// Grab total size in bytes for data declaration
		usize vertexDeclSize = vertDecl.GetSizeInBytes( );

		// Vertex Attributes
		for ( u32 i = 0; i < vertDecl.mDecl.size(); ++i )
		{
			// Grab attribute
			VertexAttributeFormat attribute = vertDecl.mDecl.at( i );

			// Enable vertex attribute array
			glEnableVertexAttribArray( i );

			// Upload attribute
			switch ( attribute )
			{
				case VertexAttributeFormat::Float4:
				{
					glVertexAttribPointer( i, 4, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::Float3:
				{
					glVertexAttribPointer( i, 3, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::Float2:
				{
					glVertexAttribPointer( i, 2, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::Float:
				{
					glVertexAttribPointer( i, 1, GL_FLOAT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt4:
				{
					glVertexAttribPointer( i, 4, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt3:
				{
					glVertexAttribPointer( i, 3, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt2:
				{
					glVertexAttribPointer( i, 2, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;

				case VertexAttributeFormat::UnsignedInt:
				{
					glVertexAttribPointer( i, 1, GL_UNSIGNED_INT, GL_FALSE, vertexDeclSize, (void*)vertDecl.GetByteOffset( i ) );
				} break;
			}
		} 

		// Unbind mVAO
		glBindVertexArray( 0 ); 

		// Set draw type
		mDrawType = GL_TRIANGLES;
		// Set draw count
		mDrawCount = mVertexData->GetSize( ) / vertDecl.GetSizeInBytes( );

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
