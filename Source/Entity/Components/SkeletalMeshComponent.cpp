#include "Entity/Components/SkeletalMeshComponent.h"
#include "Entity/Components/SkeletalAnimationComponent.h"
#include "Entity/EntityManager.h" 
#include "Graphics/GraphicsScene.h"
#include "Serialize/AssetArchiver.h"
#include "Asset/AssetManager.h"
#include "Graphics/GraphicsSubsystem.h"
#include "Graphics/AnimationSubsystem.h"
#include "SubsystemCatalog.h"
#include "ImGui/ImGuiManager.h"
#include "Base/World.h"
#include "Engine.h"

namespace Enjon
{
	//====================================================================

	void SkeletalMeshComponent::ExplicitConstructor()
	{ 
		// Set explicit tick state
		mTickState = ComponentTickState::TickAlways;
	} 

	//====================================================================

	void SkeletalMeshComponent::ExplicitDestructor()
	{
		// Remove renderable from scene
		if (mRenderable.GetGraphicsScene() != nullptr)
		{
			mRenderable.GetGraphicsScene()->RemoveSkeletalMeshRenderable(&mRenderable);
		}
	}

	//==================================================================== 

	void SkeletalMeshComponent::PostConstruction( )
	{
		// Add default mesh and material for renderable
		const AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );
		mRenderable.SetMesh( am->GetDefaultAsset< SkeletalMesh >( ) );

		// Set default materials for all material elements
		for ( u32 i = 0; i < mRenderable.GetMesh( )->GetSubMeshCount( ); ++i ) 
		{
			mRenderable.SetMaterial( am->GetDefaultAsset< Material >( ), i ); 
		}
 
		// Get graphics context from world
		World* world = mEntity->GetWorld( )->ConstCast< World >( );
		GraphicsScene* gs = world->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( ); 

		// Add renderable to scene
		gs->AddSkeletalMeshRenderable( &mRenderable );

		// Set id of renderable to entity id
		mRenderable.SetRenderableID( mEntity->GetID( ) );
	}

	//==================================================================== 

	void SkeletalMeshComponent::Update( )
	{
		mRenderable.SetTransform(mEntity->GetWorldTransform());
	}
	
	//====================================================================
		
	Vec3 SkeletalMeshComponent::GetPosition() const
	{ 
		return mRenderable.GetPosition(); 
	}
	
	//====================================================================

	Vec3 SkeletalMeshComponent::GetScale() const
	{ 
		return mRenderable.GetScale(); 
	}

	//====================================================================

	Quaternion SkeletalMeshComponent::GetRotation() const 
	{ 
		return mRenderable.GetRotation(); 
	}

	//====================================================================

	AssetHandle< Material > SkeletalMeshComponent::GetMaterial( const u32& idx ) const
	{ 
		return mRenderable.GetMaterial( idx ); 
	}

	//====================================================================

	AssetHandle<Mesh> SkeletalMeshComponent::GetMesh() const
	{ 
		return mRenderable.GetMesh(); 
	}

	//====================================================================

	GraphicsScene* SkeletalMeshComponent::GetGraphicsScene() const
	{ 
		return mRenderable.GetGraphicsScene(); 
	}

	//====================================================================

	Transform SkeletalMeshComponent::GetTransform() const 
	{ 
		return mRenderable.GetTransform(); 
	} 

	//====================================================================

	SkeletalMeshRenderable* SkeletalMeshComponent::GetRenderable()
	{ 
		return &mRenderable; 
	}

	//====================================================================
	
	void SkeletalMeshComponent::SetTransform( const Transform& transform )
	{
		mRenderable.SetTransform( transform );
	}

	//====================================================================

	void SkeletalMeshComponent::SetPosition(const Vec3& position)
	{
		mRenderable.SetPosition(position);
	}

	//====================================================================

	void SkeletalMeshComponent::SetScale(const Vec3& scale)
	{
		mRenderable.SetScale(scale);
	}

	//====================================================================

	void SkeletalMeshComponent::SetScale(const f32& scale)
	{
		mRenderable.SetScale(scale);
	}

	//====================================================================

	void SkeletalMeshComponent::SetRotation(const Quaternion& rotation)
	{
		mRenderable.SetRotation(rotation);
	}
 
	//====================================================================

	void SkeletalMeshComponent::SetMaterial( const AssetHandle< Material >& material, const u32& idx ) 
	{
		mRenderable.SetMaterial( material, idx );
	}

	//====================================================================

	void SkeletalMeshComponent::SetMesh(const AssetHandle<SkeletalMesh>& mesh)
	{
		mRenderable.SetMesh( mesh );
	}

	//====================================================================

	void SkeletalMeshComponent::SetGraphicsScene(GraphicsScene* scene)
	{
		mRenderable.SetGraphicsScene(scene);
	} 

	//====================================================================

	Result SkeletalMeshComponent::SerializeData( ByteBuffer* buffer ) const
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

	Result SkeletalMeshComponent::DeserializeData( ByteBuffer* buffer )
	{
		// Get asset manager
		AssetManager* am = EngineSubsystem( AssetManager );

		// Set mesh
		mRenderable.SetMesh( am->GetAsset< SkeletalMesh >( buffer->Read< UUID >( ) ) );

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

	Result SkeletalMeshComponent::OnEditorUI( )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		// Debug dump renderable
		igm->InspectObject( &mRenderable );

		// Reset renderable mesh
		mRenderable.SetMesh( mRenderable.GetMesh( ) );

		return Result::SUCCESS;
	}

	//==================================================================== 

	// This is assuming NO skeletal animation component
	void SkeletalMeshComponent::UpdateAndCalculateTransforms( )
	{ 
		// Grab renderable from skeletal mesh component
		SkeletalMeshRenderable* renderable = &mRenderable;
		const Skeleton* skeleton = renderable->GetSkeleton( ).Get();

		if ( !skeleton )
		{
			return;
		} 

		// If not valid skeleton, then return
		s32 rootID = skeleton->GetRootID( );
		if ( !skeleton->GetNumberOfJoints( ) || rootID == -1 )
		{
			return;
		}

		// Grab matrices to affect from renderable
		skeleton->GetBindJointTransforms( renderable->mFinalJointTransforms );
	}

	//==================================================================== 
}
