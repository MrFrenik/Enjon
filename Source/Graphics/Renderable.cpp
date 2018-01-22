#include "Graphics/Renderable.h"
#include "Graphics/Scene.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/GLSLProgram.h"
#include "Graphics/GraphicsSubsystem.h" 
#include "Graphics/Color.h"

#include <assert.h>

namespace Enjon  
{
	//--------------------------------------------------------------------
	Renderable::Renderable()
		: mMaterial(nullptr)
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

	AssetHandle< Material > Renderable::GetMaterial() const
	{ 
		return mMaterial; 
	}

	//==============================================================

	AssetHandle< Mesh > Renderable::GetMesh() const 
	{ 
		return mMesh; 
	}

	//==============================================================

	Scene* Renderable::GetScene() const 
	{ 
		return mScene; 
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
		Quaternion X = Quaternion::AngleAxis(Yaw, 	Vec3(0, 1, 0)); 						// Absolute Up
		Quaternion Y = Quaternion::AngleAxis(Pitch, mTransform.Rotation * Vec3(1, 0, 0));	// Relative Right

		mTransform.Rotation = X * Y * mTransform.Rotation;
	}

	//--------------------------------------------------------------------

	void Renderable::SetMaterial( const AssetHandle< Material >& material )
	{
		mMaterial = material;
	}

	//--------------------------------------------------------------------
	void Renderable::SetMesh(const AssetHandle<Mesh>& mesh)
	{
		mMesh = mesh;
	}

	//--------------------------------------------------------------------
	void Renderable::SetScene(Scene* scene)
	{
		mScene = scene;
	}

	//==============================================================================

	void Renderable::Submit( const Enjon::Shader* shader )
	{
		// Check for shader validity
		if ( shader == nullptr )
		{
			return;
		}

		// Bind mesh and submit
		const Mesh* mesh = GetMesh( ).Get( );
		if ( mesh != nullptr )
		{
			mesh->Bind( );
			{
				Mat4 Model;
				Model *= Mat4::Translate( GetPosition( ) );
				Model *= QuaternionToMat4( GetRotation( ) );
				Model *= Mat4::Scale( GetScale( ) );
				const_cast< Enjon::Shader* > ( shader )->SetUniform( "uModel", Model );
				mesh->Submit( );
			}
			mesh->Unbind( ); 
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
		const Mesh* mesh = GetMesh( ).Get( );
		if ( mesh != nullptr )
		{
			mesh->Bind( );
			{
				Mat4 Model;
				Model *= Mat4::Translate( GetPosition( ) );
				Model *= QuaternionToMat4( GetRotation( ) );
				Model *= Mat4::Scale( GetScale( ) );
				const_cast< Enjon::GLSLProgram* > ( shader )->SetUniform( "u_model", Model );
				mesh->Submit( );
			}
			mesh->Unbind( ); 
		} 
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
}










