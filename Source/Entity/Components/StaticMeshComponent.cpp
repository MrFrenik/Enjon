#include "Entity/Components/StaticMeshComponent.h"
#include "Entity/EntityManager.h" 
#include "Graphics/GraphicsScene.h"
#include "Serialize/AssetArchiver.h"
#include "Asset/AssetManager.h"
#include "Graphics/GraphicsSubsystem.h"
#include "SubsystemCatalog.h"
#include "ImGui/ImGuiManager.h"
#include "Entity/EntityManager.h"
#include "Base/World.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

namespace Enjon
{
	//====================================================================

	void StaticMeshComponent::ExplicitConstructor()
	{ 
		// Set explicit tick state
		mTickState = ComponentTickState::TickNever;
	} 

	//====================================================================

	void StaticMeshComponent::ExplicitDestructor()
	{
		// Remove renderable from scene
		if (mRenderable.GetGraphicsScene() != nullptr)
		{
			mRenderable.GetGraphicsScene()->RemoveStaticMeshRenderable(&mRenderable);
		}
	}

	//==================================================================== 

	void StaticMeshComponent::PostConstruction( )
	{
		// Add default mesh and material for renderable
		AssetManager* am = EngineSubsystem( AssetManager );
		mRenderable.SetMesh( am->GetDefaultAsset< Mesh >( ) );

		// Set default materials for all material elements
		for ( u32 i = 0; i < mRenderable.GetMesh( )->GetSubMeshCount( ); ++i ) 
		{
			mRenderable.SetMaterial( am->GetDefaultAsset< Material >( ), i ); 
		} 

		// Get graphics scene from world graphics context
		World* world = GetEntity()->GetWorld( )->ConstCast< World >( );
		GraphicsScene* gs = world->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( ); 

		// Add renderable to scene
		gs->AddStaticMeshRenderable( &mRenderable );

		// Set id of renderable to entity id
		mRenderable.SetRenderableID( GetEntity()->GetID( ) ); 

		// Hate doing this. For real. Fucking hate it.
		u32 handle = gs->AllocateStaticMeshRenderable( GetEntity()->GetID( ) ); 
		mRenderableHandle = handle;
		mGraphicsScene = gs;
	}

	//==================================================================== 

	void StaticMeshComponent::AddToWorld( World* world )
	{
		RemoveFromWorld( );

		// Get graphics scene from world graphics context
		GraphicsScene* gs = world->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( ); 

		// Add to graphics scene
		if ( gs )
		{
			gs->AddStaticMeshRenderable( &mRenderable );
		}
	}

	//==================================================================== 

	void StaticMeshComponent::RemoveFromWorld( )
	{
		if ( mRenderable.GetGraphicsScene( ) != nullptr )
		{
			mRenderable.GetGraphicsScene( )->RemoveStaticMeshRenderable( &mRenderable );
		} 
	}

	void StaticMeshComponent::Update( )
	{
		mRenderable.SetTransform(mEntity->GetWorldTransform());
	}
	
	//====================================================================
		
	Vec3 StaticMeshComponent::GetPosition() const
	{ 
		return mRenderable.GetPosition(); 
	}
	
	//====================================================================

	Vec3 StaticMeshComponent::GetScale() const
	{ 
		return mRenderable.GetScale(); 
	}

	//====================================================================

	Quaternion StaticMeshComponent::GetRotation() const 
	{ 
		return mRenderable.GetRotation(); 
	}

	//====================================================================

	AssetHandle< Material > StaticMeshComponent::GetMaterial( const u32& idx ) const
	{ 
		return mRenderable.GetMaterial( idx ); 
	}

	//====================================================================

	AssetHandle<Mesh> StaticMeshComponent::GetMesh() const
	{ 
		return mRenderable.GetMesh(); 
	}

	//====================================================================

	GraphicsScene* StaticMeshComponent::GetGraphicsScene() const
	{ 
		return mRenderable.GetGraphicsScene(); 
	}

	//====================================================================

	Transform StaticMeshComponent::GetTransform() const 
	{ 
		return mRenderable.GetTransform(); 
	} 

	//====================================================================

	StaticMeshRenderable* StaticMeshComponent::GetRenderable()
	{ 
		return &mRenderable; 
	}
	
	/* Sets world transform */
	void StaticMeshComponent::SetTransform( const Transform& transform )
	{
		mRenderable.SetTransform( transform );
	}

	//====================================================================

	void StaticMeshComponent::SetPosition(const Vec3& position)
	{
		mRenderable.SetPosition(position);
	}

	//====================================================================

	void StaticMeshComponent::SetScale(const Vec3& scale)
	{
		mRenderable.SetScale(scale);
	}

	//====================================================================

	void StaticMeshComponent::SetScale(const f32& scale)
	{
		mRenderable.SetScale(scale);
	}

	//====================================================================

	void StaticMeshComponent::SetRotation(const Quaternion& rotation)
	{
		mRenderable.SetRotation(rotation);
	}
 
	//====================================================================

	void StaticMeshComponent::SetMaterial( const AssetHandle< Material >& material, const u32& idx ) 
	{
		mRenderable.SetMaterial( material, idx );

		// Get graphics scene from world graphics context
		World* world = GetEntity()->GetWorld( )->ConstCast< World >( );
		GraphicsScene* gs = world->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( ); 

		gs->SetStaticMeshRenderableMaterial( GetEntity()->GetID( ), material, idx ); 
	}

	//====================================================================

	void StaticMeshComponent::SetMesh( const AssetHandle<Mesh>& mesh)
	{
		mRenderable.SetMesh( mesh );
 
		// Get graphics scene from world graphics context
		World* world = GetEntity()->GetWorld( )->ConstCast< World >( );
		GraphicsScene* gs = world->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( ); 

		gs->SetStaticMeshRenderableMesh( GetEntity()->GetID( ), mesh ); 
	}

	//==================================================================== 

	void StaticMeshComponent::SetGraphicsScene(GraphicsScene* scene)
	{
		mRenderable.SetGraphicsScene(scene);
	} 

	//====================================================================

	Result StaticMeshComponent::SerializeData( ByteBuffer* buffer ) const
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

	Result StaticMeshComponent::DeserializeData( ByteBuffer* buffer )
	{
		// Get asset manager
		AssetManager* am = EngineSubsystem( AssetManager );

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

	Result StaticMeshComponent::OnEditorUI( )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		EntityManager* em = EngineSubsystem( EntityManager );

		// Grab renderable from graphics subsystem
		GraphicsScene* gs = em->GetRawEntity( mEntityID )->GetWorld( )->ConstCast< World >( )->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( );
		StaticMeshRenderable* rend = gs->GetStaticMeshRenderable( mRenderableHandle ); 

		// Debug dump renderable
		igm->InspectObject( rend );

		// Reset renderable mesh
		rend->SetMesh( rend->GetMesh( ) );

		return Result::SUCCESS;
	}

	//==================================================================== 

	void StaticMeshComponentSystem::ExplicitConstructor( )
	{
		mTickState = ComponentTickState::TickAlways;

		// Register for component callback function
		IComponentInstanceData* data = EngineSubsystem( EntityManager )->GetIComponentInstanceData< StaticMeshComponent >( ); 

		// Or do a lambda which captures this with its member function
		data->RegisterPostConstructionCallback( [ & ]( const u32& id, IComponentInstanceData* d ) 
		{
			return PostComponentConstruction( id, d );
		} );
	}

	//==================================================================== 

	// This makes the IDIOTIC assumption that this component data is in face what I want. No type safety there whatsoever.
	Result StaticMeshComponentSystem::PostComponentConstruction( const u32& id, IComponentInstanceData* data )
	{ 
		// Get subsystems		
		AssetManager* am = EngineSubsystem( AssetManager );
		EntityManager* em = EngineSubsystem( EntityManager );

		// Get graphics scene from world graphics context
		Entity* ent = em->GetRawEntity( id );
		World* world = ent->GetWorld( )->ConstCast< World >( );
		GraphicsScene* gs = world->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( );

		// Should probably be a reference instead...
		ComponentHandle< StaticMeshComponent >& smc = data->GetComponentHandle< StaticMeshComponent >( id );

		// Allocate new renderable handle
		u32 handle = gs->AllocateStaticMeshRenderable( ent->GetID( ) ); 
		smc->mRenderableHandle = handle;
		smc->mGraphicsScene = gs;
			
		// I still like this syntax
		//data->SetValue( id, &StaticMeshComponent::mRenderableHandle, handle ); 

		return Result::SUCCESS;
	}

	//==================================================================== 

	void StaticMeshComponentSystem::Update( )
	{ 
		// Grab component instance data and update that
		EntityManager* em = EngineSubsystem( EntityManager ); // This might make the case to have EntityManagers be an instanced thing PER world. That way all entities / systems / components are grouped logically together and reduces lookups 
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );

		ComponentInstanceData< StaticMeshComponent >* iData = em->GetIComponentInstanceData< StaticMeshComponent >( );
		StaticMeshComponent* compData = iData->GetComponentData( )->ConstCast< StaticMeshComponent >( );
		const Vector< u32 >& eids = iData->GetEntityIDs();

		// Update all data
		for ( usize i = 0; i < iData->GetCount( ); ++i )
		{
			Entity* ent = em->GetRawEntity( eids.at( i ) );		// Getting raw entity isn't terrible, but the cache is killed by loading up an entity that has a UUID associated with it. Need to store these elsewhere ( probably just in the EntityMangager itself ); 
			compData[ i ].mGraphicsScene->SetStaticMeshRenderableTransform( compData[ i ].mRenderableHandle, ent->GetWorldTransform( ) ); 
		} 
	}
}
