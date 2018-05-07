#include "Entity/Components/GraphicsComponent.h"
#include "Entity/EntityManager.h" 
#include "Graphics/GraphicsScene.h"
#include "Serialize/AssetArchiver.h"
#include "Asset/AssetManager.h"
#include "Graphics/GraphicsSubsystem.h"
#include "SubsystemCatalog.h"
#include "ImGui/ImGuiManager.h"
#include "Engine.h"

namespace Enjon
{
	//====================================================================

	void GraphicsComponent::ExplicitConstructor()
	{ 
		// Add default mesh and material for renderable
		const AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );
		mRenderable.SetMesh( am->GetDefaultAsset< Mesh >( ) );

		// Set default materials for all material elements
		for ( u32 i = 0; i < mRenderable.GetMesh( )->GetSubMeshCount( ); ++i ) 
		{
			mRenderable.SetMaterial( am->GetDefaultAsset< Material >( ), i ); 
		}

		// Add renderable to scene
		GraphicsSubsystem* gs = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< GraphicsSubsystem >( )->ConstCast< GraphicsSubsystem >( );
		gs->GetGraphicsScene( )->AddStaticMeshRenderable( &mRenderable );

		// Set explicit tick state
		mTickState = ComponentTickState::TickAlways;
	} 

	//====================================================================

	void GraphicsComponent::ExplicitDestructor()
	{
		// Remove renderable from scene
		if (mRenderable.GetGraphicsScene() != nullptr)
		{
			mRenderable.GetGraphicsScene()->RemoveStaticMeshRenderable(&mRenderable);
		}
	}

	//==================================================================== 

	void GraphicsComponent::PostConstruction( )
	{
		// Set id of renderable to entity id
		mRenderable.SetRenderableID( mEntity->GetID( ) );
	}

	//==================================================================== 

	void GraphicsComponent::Update( )
	{
		mRenderable.SetTransform(mEntity->GetWorldTransform());
	}
	
	//====================================================================
		
	Vec3 GraphicsComponent::GetPosition() const
	{ 
		return mRenderable.GetPosition(); 
	}
	
	//====================================================================

	Vec3 GraphicsComponent::GetScale() const
	{ 
		return mRenderable.GetScale(); 
	}

	//====================================================================

	Quaternion GraphicsComponent::GetRotation() const 
	{ 
		return mRenderable.GetRotation(); 
	}

	//====================================================================

	AssetHandle< Material > GraphicsComponent::GetMaterial( const u32& idx ) const
	{ 
		return mRenderable.GetMaterial( idx ); 
	}

	//====================================================================

	AssetHandle<Mesh> GraphicsComponent::GetMesh() const
	{ 
		return mRenderable.GetMesh(); 
	}

	//====================================================================

	GraphicsScene* GraphicsComponent::GetGraphicsScene() const
	{ 
		return mRenderable.GetGraphicsScene(); 
	}

	//====================================================================

	Transform GraphicsComponent::GetTransform() const 
	{ 
		return mRenderable.GetTransform(); 
	} 

	//====================================================================

	StaticMeshRenderable* GraphicsComponent::GetRenderable()
	{ 
		return &mRenderable; 
	}
	
	/* Sets world transform */
	void GraphicsComponent::SetTransform( const Transform& transform )
	{
		mRenderable.SetTransform( transform );
	}

	//====================================================================

	void GraphicsComponent::SetPosition(const Vec3& position)
	{
		mRenderable.SetPosition(position);
	}

	//====================================================================

	void GraphicsComponent::SetScale(const Vec3& scale)
	{
		mRenderable.SetScale(scale);
	}

	//====================================================================

	void GraphicsComponent::SetScale(const f32& scale)
	{
		mRenderable.SetScale(scale);
	}

	//====================================================================

	void GraphicsComponent::SetRotation(const Quaternion& rotation)
	{
		mRenderable.SetRotation(rotation);
	}
 
	//====================================================================

	void GraphicsComponent::SetMaterial( const AssetHandle< Material >& material, const u32& idx ) 
	{
		mRenderable.SetMaterial( material, idx );
	}

	//====================================================================

	void GraphicsComponent::SetMesh(const AssetHandle<Mesh>& mesh)
	{
		mRenderable.SetMesh( mesh );
	}

	//====================================================================

	void GraphicsComponent::SetGraphicsScene(GraphicsScene* scene)
	{
		mRenderable.SetGraphicsScene(scene);
	} 

	//====================================================================

	Result GraphicsComponent::SerializeData( ByteBuffer* buffer ) const
	{
		// Write uuid of mesh
		buffer->Write< UUID >( mRenderable.GetMesh( )->GetUUID( ) );

		// Write out renderable material size
		buffer->Write< u32 >( mRenderable.GetMaterialsCount( ) );

		// Write uuid of materials in renderable
		for ( auto& mat : mRenderable.GetMaterials( ) )
		{
			buffer->Write< UUID >( mat.Get()->GetUUID( ) );
		}

		return Result::SUCCESS;
	}

	//====================================================================

	Result GraphicsComponent::DeserializeData( ByteBuffer* buffer )
	{
		// Get asset manager
		const AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );

		// Set mesh
		mRenderable.SetMesh( am->GetAsset< Mesh >( buffer->Read< UUID >( ) ) );

		// Get count of materials
		u32 matCount = buffer->Read< u32 >( );

		// Deserialize materials
		for ( u32 i = 0; i < matCount; ++i )
		{
			// Grab the material
			AssetHandle< Material > mat = am->GetAsset< Material >( buffer->Read< UUID >( ) );

			// Set material in renderable at index
			mRenderable.SetMaterial( mat, i );
		} 

		return Result::SUCCESS;
	}

	//==================================================================== 

	Result GraphicsComponent::OnEditorUI( )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		// Debug dump renderable
		igm->InspectObject( &mRenderable );

		// Reset renderable mesh
		mRenderable.SetMesh( mRenderable.GetMesh( ) );

		return Result::SUCCESS;
	}

	//==================================================================== 
}
