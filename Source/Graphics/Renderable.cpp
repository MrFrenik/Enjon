#include "Graphics/Renderable.h"
#include "Graphics/GraphicsScene.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/GLSLProgram.h"
#include "Graphics/GraphicsSubsystem.h" 
#include "Graphics/Color.h"
#include "ImGui/ImGuiManager.h"
#include "Asset/AssetManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

#include <assert.h> 

namespace Enjon  
{
	//--------------------------------------------------------------------
	Renderable::Renderable()
	{
	}

	//--------------------------------------------------------------------
	Renderable::~Renderable()
	{
	} 

	//==============================================================

	Vec3 Renderable::GetPosition() const
	{
		return mTransform.GetPosition();
	}

	//==============================================================

	Vec3 Renderable::GetScale() const
	{
		return mTransform.GetScale();
	}

	//==============================================================

	Quaternion Renderable::GetRotation() const
	{
		return mTransform.GetRotation();
	}

	//==============================================================

	AssetHandle< Material > Renderable::GetMaterial( const u32& idx ) const
	{ 
		if ( idx < mMaterialElements.size( ) )
		{
			return mMaterialElements.at( idx );
		}

		// Otherwise return last element
		return mMaterialElements.back( );
	} 

	//==============================================================

	GraphicsScene* Renderable::GetGraphicsScene() const 
	{ 
		return mGraphicsScene; 
	}

	//==============================================================

	Transform Renderable::GetTransform() const 
	{ 
		return mTransform; 
	}

	//==============================================================

	void Renderable::SetTransform(const Transform& transform) 
	{ 
		mTransform = transform; 
	}

	//==============================================================

	void Renderable::SetPosition(const Vec3& position)
	{
		mTransform.SetPosition(position);
	}

	//--------------------------------------------------------------------
	void Renderable::SetScale(const Vec3& scale)
	{
		mTransform.SetScale(scale);
	}

	//--------------------------------------------------------------------
	void Renderable::SetScale(const f32& scale)
	{
		mTransform.SetScale(scale);
	}

	//--------------------------------------------------------------------
	void Renderable::SetRotation(const Quaternion& rotation)
	{
		mTransform.SetRotation(rotation);
	}

	//==============================================================

	void Renderable::OffsetRotation(const f32& Yaw, const f32& Pitch)
	{
		Quaternion X = Quaternion::AngleAxis(Yaw, Vec3::YAxis()); 								// Absolute Up
		Quaternion Y = Quaternion::AngleAxis(Pitch, mTransform.GetRotation() * Vec3(1, 0, 0));	// Relative Right

		mTransform.SetRotation( X * Y * mTransform.GetRotation( ) );
	}

	//--------------------------------------------------------------------

	void Renderable::SetMaterial( const AssetHandle< Material >& material, const u32& idx )
	{
		if ( idx < mMaterialElements.size( ) )
		{
			mMaterialElements.at( idx ) = material; 
		}
	} 

	//--------------------------------------------------------------------

	void Renderable::SetGraphicsScene(GraphicsScene* scene) 
	{
		mGraphicsScene = scene;
	}

	//==============================================================================

	u32 Renderable::GetMaterialsCount( ) const
	{
		return mMaterialElements.size( );
	}

	//==============================================================================

	const Vector<AssetHandle<Material>>& Renderable::GetMaterials( ) const
	{
		return mMaterialElements;
	}

	//==============================================================================

	void Renderable::Bind( )
	{ 
		mCurrentModelMatrix = Mat4x4::Translate( GetPosition( ) ) * QuaternionToMat4x4( GetRotation( ) ) * Mat4x4::Scale( GetScale( ) );
	}

	//==============================================================================

	void Renderable::Unbind( )
	{
		mPreviousModelMatrix = mCurrentModelMatrix; 
	}

	//==============================================================================

	void Renderable::Submit( const Enjon::Shader* shader, const SubMesh* subMesh )
	{
		if ( shader == nullptr )
		{
			return;
		}

		const_cast< Enjon::Shader* > ( shader )->SetUniform( "uModel", mCurrentModelMatrix );
		const_cast< Enjon::Shader* > ( shader )->SetUniform( "uPreviousModel", mPreviousModelMatrix );

		// Bind submesh
		subMesh->Bind( );
		{
			// Submit for rendering
			subMesh->Submit( ); 
		}
		// Unbind submesh
		subMesh->Unbind( ); 
	}

	//==============================================================================

	void Renderable::Submit( const GLSLProgram* shader, const SubMesh* subMesh )
	{ 
		if ( shader == nullptr )
		{
			return;
		}

		const_cast< Enjon::GLSLProgram* > ( shader )->SetUniform( "uModel", mCurrentModelMatrix );
		const_cast< Enjon::GLSLProgram* > ( shader )->SetUniform( "uPreviousModel", mPreviousModelMatrix );

		// Bind submesh
		subMesh->Bind( );
		{
			// Submit for rendering
			subMesh->Submit( ); 
		}
		// Unbind submesh
		subMesh->Unbind( ); 
	}

	void Renderable::Submit( const Enjon::Shader* shader )
	{
		// Check for shader validity
		if ( shader == nullptr )
		{
			return;
		}

		// Bind mesh and submit
		const Mesh* mesh = GetMesh( );
		if ( mesh != nullptr )
		{
			// Check if mesh has submeshes first
			const Vector<SubMesh*>& subMeshes = mesh->GetSubmeshes( );

			{
				Mat4x4 Model;
				Model *= Mat4x4::Translate( GetPosition( ) );
				Model *= QuaternionToMat4x4( GetRotation( ) );
				Model *= Mat4x4::Scale( GetScale( ) );
				const_cast< Enjon::Shader* > ( shader )->SetUniform( "uModel", Model );
				const_cast< Enjon::Shader* > ( shader )->SetUniform( "uPreviousModel", mPreviousModelMatrix );

				// For each submesh, bind
				for ( auto& sm : subMeshes )
				{
					sm->Bind( );
					{
						sm->Submit( ); 
					}
					sm->Unbind( );
				}

				// Set the previous model matrix with current one
				mPreviousModelMatrix = Model;
			} 
		} 
	}
	
	void Renderable::Submit( const Enjon::GLSLProgram* shader )
	{
		// Check for shader validity
		if ( shader == nullptr )
		{
			return;
		}

		// Bind mesh and submit
		//const Mesh* mesh = GetMesh( ).Get( );
		//if ( mesh != nullptr )
		//{
		//	mesh->Bind( );
		//	{
		//		Mat4 Model;
		//		Model *= Mat4::Translate( GetPosition( ) );
		//		Model *= QuaternionToMat4( GetRotation( ) );
		//		Model *= Mat4::Scale( GetScale( ) );
		//		const_cast< Enjon::GLSLProgram* > ( shader )->SetUniform( "u_model", Model );
		//		const_cast< Enjon::GLSLProgram* > ( shader )->SetUniform( "uPreviousModel", mPreviousModelMatrix );
		//		mesh->Submit( );

		//		// Set the previous model matrix with current one
		//		mPreviousModelMatrix = Model;
		//	}
		//	mesh->Unbind( ); 
		//} 
	}

	//=================================================================================================

	void Renderable::SetRenderableID( const u32& id )
	{
		mRenderableID = id;
	}

	//=================================================================================================

	u32 Renderable::GetRenderableID( ) const
	{
		return mRenderableID;
	}

	//=================================================================================================

	ColorRGBA32 Renderable::IdToColor( const u32& id )
	{
		f32 r = f32( ( id & 0x000000FF ) >> 0 ) / 255.0f;
		f32 g = f32( ( id & 0x0000FF00 ) >> 8 ) / 255.0f;
		f32 b = f32( ( id & 0x00FF0000 ) >> 16 ) / 255.0f;

		return ColorRGBA32( r, g, b, 1.0f );
	}

	//=================================================================================================

	u32 Renderable::ColorToID( const ColorRGBA32& color )
	{
		u32 id = ( u32 )( color.r * 255
						+ color.g * 255 * 256
						+ color.b * 255 * 256 * 256 );

		return id;
	}

	//================================================================================================= 

	Result Renderable::OnEditorUI( )
	{
		// Show mesh

		
		// Show material elements

		return Result::INCOMPLETE;
	}

	//================================================================================================= 

	const Mat4x4 Renderable::GetModelMatrix( ) const
	{
		return mCurrentModelMatrix;
	}

	//================================================================================================= 

	const Mat4x4 Renderable::GetPreviousModelMatrix( ) const
	{
		return mPreviousModelMatrix;
	}

	//================================================================================================= 
}










