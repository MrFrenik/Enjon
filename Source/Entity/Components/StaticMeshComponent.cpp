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

		// Get graphics scene from world graphics context
		Entity* ent = GetEntity( );
		World* world = ent->GetWorld( )->ConstCast< World >( ); 
		GraphicsScene* gs = world->GetContext< GraphicsSubsystemContext >( )->GetGraphicsScene( ); 

		// Allocate new static mesh renderable and capture handle
		mRenderableHandle = gs->AllocateStaticMeshRenderable( ent->GetID() ); 
		mGraphicsScene = gs;

		// Set default mesh for renderable
		mRenderableHandle->SetMesh( am->GetDefaultAsset< Mesh >( ) ); 

		// Set default materials for all material elements
		for ( u32 i = 0; i < mRenderableHandle->GetMesh( )->GetSubMeshCount( ); ++i ) 
		{
			mRenderableHandle->SetMaterial( am->GetDefaultAsset< Material >( ), i ); 
		} 
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
	
	//====================================================================
		
	Vec3 StaticMeshComponent::GetPosition() const
	{ 
		return mRenderableHandle->GetPosition(); 
	}
	
	//====================================================================

	Vec3 StaticMeshComponent::GetScale() const
	{ 
		return mRenderableHandle->GetScale(); 
	}

	//====================================================================

	Quaternion StaticMeshComponent::GetRotation() const 
	{ 
		return mRenderableHandle->GetRotation(); 
	}

	//====================================================================

	AssetHandle< Material > StaticMeshComponent::GetMaterial( const u32& idx ) const
	{ 
		return mRenderableHandle->GetMaterial( idx ); 
	}

	//====================================================================

	AssetHandle<Mesh> StaticMeshComponent::GetMesh() const
	{ 
		return mRenderableHandle->GetMesh(); 
	}

	//====================================================================

	GraphicsScene* StaticMeshComponent::GetGraphicsScene() const
	{ 
		return mRenderableHandle->GetGraphicsScene(); 
	}

	//====================================================================

	Transform StaticMeshComponent::GetTransform() const 
	{ 
		return mRenderableHandle->GetTransform(); 
	} 

	//====================================================================

	StaticMeshRenderable* StaticMeshComponent::GetRenderable()
	{ 
		return &( *mRenderableHandle ); 
	}

	//====================================================================
	
	void StaticMeshComponent::SetTransform( const Transform& transform )
	{
		mRenderableHandle->SetTransform( transform );
	}

	//====================================================================

	void StaticMeshComponent::SetPosition(const Vec3& position)
	{
		mRenderableHandle->SetPosition(position);
	}

	//====================================================================

	void StaticMeshComponent::SetScale(const Vec3& scale)
	{
		mRenderableHandle->SetScale(scale);
	}

	//====================================================================

	void StaticMeshComponent::SetScale(const f32& scale)
	{
		mRenderableHandle->SetScale(scale);
	}

	//====================================================================

	void StaticMeshComponent::SetRotation(const Quaternion& rotation)
	{
		mRenderableHandle->SetRotation( rotation );
	}
 
	//====================================================================

	void StaticMeshComponent::SetMaterial( const AssetHandle< Material >& material, const u32& idx ) 
	{ 
		mRenderableHandle->SetMaterial( material, idx ); 
	}

	//====================================================================

	void StaticMeshComponent::SetMesh( const AssetHandle<Mesh>& mesh)
	{
		mRenderableHandle->SetMesh( mesh );
	} 

	//====================================================================

	Result StaticMeshComponent::SerializeData( ByteBuffer* buffer ) const
	{
		// Write uuid of mesh
		buffer->Write< UUID >( mRenderableHandle->GetMesh( )->GetUUID( ) );

		// Write out renderable material size
		buffer->Write< u32 >( mRenderableHandle->GetMaterialsCount( ) );

		// Write uuid of materials in renderable
		for ( auto& mat : mRenderableHandle->GetMaterials( ) )
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
		mRenderableHandle->SetMesh( am->GetAsset< Mesh >( buffer->Read< UUID >( ) ) );

		// Get count of materials
		u32 matCount = buffer->Read< u32 >( );

		// Deserialize materials
		for ( u32 i = 0; i < matCount; ++i )
		{
			// Grab the material
			AssetHandle< Material > mat = am->GetAsset< Material >( buffer->Read< UUID >( ) );

			// Set material in renderable at index
			mRenderableHandle->SetMaterial( mat, i );
		} 

		return Result::SUCCESS;
	}

	//==================================================================== 

	Result StaticMeshComponent::OnEditorUI( )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		EntityManager* em = EngineSubsystem( EntityManager );

		// Grab renderable from handle
		StaticMeshRenderable& rend = *mRenderableHandle;

		// Debug dump renderable
		igm->InspectObject( &rend );

		// HACK(): Reset renderable mesh
		rend.SetMesh( rend.GetMesh( ) );

		return Result::SUCCESS;
	}

	//==================================================================== 

	void StaticMeshComponentSystem::ExplicitConstructor( )
	{
		mTickState = ComponentTickState::TickAlways;

		// Register for component callback function
		IComponentInstanceData* data = EngineSubsystem( EntityManager )->GetIComponentInstanceData< StaticMeshComponent >( ); 

		// Or do a lambda which captures this with its member function
		//data->RegisterPostConstructionCallback( [ & ]( const u32& id, IComponentInstanceData* d ) 
		//{
		//	return PostComponentConstruction( id, d );
		//} );
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
		//ComponentHandle< StaticMeshComponent >& smc = data->GetComponentHandle< StaticMeshComponent >( id );

		// Allocate new renderable handle
		//smc->mRenderableHandle = gs->AllocateStaticMeshRenderable( ent->GetID( ) );;
		//smc->mGraphicsScene = gs;
			
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
		StaticMeshComponent* compData = iData->Data( );

		// Update all data
		for ( usize i = 0; i < iData->GetDataSize( ); ++i )
		{
			StaticMeshComponent& smc = compData[ i ];
			smc.mRenderableHandle->SetTransform( smc.GetEntity()->GetWorldTransform( ) );
		} 
	} 

	/*
		// Want a more robust internal resource handle system ( mainly for gfx objects like render targets, renderables, shaders, etc. that don't rely on the asset management system ) 

		#include <limits>

		const u32 INVALID_HANDLE = std::numeric_limits< u32 >::max();

		template <typename Resource>
		struct ResourceHandle
		{
			friend ResourceManager< Resource >;

			Resource* operator ->() 
			{
				return mResourceManager->Lookup( mResourceID );
			}

			Resource* operator *()
			{
				return mResourceManager->Lookup( mResourceID );
			}

			private:
				u32 mResourceID;
				ResourceManager< Resource >* mResourceManager;
		}; 

		template <typename Resource> 
		struct ResourceManager
		{
			public:

				inline ResourceHandle< Resource > Add()
				{ 
					// Here's the tricky part. Want to push back a new index? Do I store a free list of indices? Do I iterate to find a free index? ( ideally would not do that last bit )	
					// Free list stack? So you push a free index onto the stack and then pop off to get the newest available index? 
					u32 freeIdx = GetNextAvailableIndex();

					// Allocate new resource					
					mResources.emplace(); 

					// Push back new indirection index
					mReverseIndirectionIndices.push_back( freeIdx );

					// This gets the available index in the indirection list of indices, not in the actual resource array ( the reason being that the resource array can have its contents shifted around when adding / removing items )
					// If the index is the last item in the list, then push that on to grow the array
					mHandleIndices[ freeIdx ] = mResources.size() - 1;

					return { freeIdx, this };
				}

				inline void Remove( const ResourceHandle< Resource >& res )
				{
					// Need to grab the actual resource index from the handle's indirection index
					u32 idx = mHandleIndices[ res.mResourceID ];

					// Need to pop and swap resources, but now also need to make sure that I update the handle index indirection map, which could be tricky
					if ( mResources.size() > 1 )
					{
						std::iter_swap( mResources.begin() + idx, mResources.end() - 1 );
						mResources.pop_back();
					}
					else
					{
						mResources.clear();
					}

					// Have to find a way to update the index in the indirection map...
					
				} 

				inline Resource* Lookup( const ResourceHandle< Resource >& res ) override
				{ 
					// Something like this? Allocating a new resource would look for the next available index? Or would just push a new index onto stack?
					return &mResources[ mIndices[ res.mResourceID ] ];
				}

			private: 

				u32 GetNextAvailableIndex()
				{
					// If stack is empty, then simply return the size of the index array, which will be the back index after growing
					if ( mIndexFreeList.empty() )
					{
						// Grow the array by 1
						mHandleIndices.push_back( 0 );
						// Return the last index
						return mHandleIndices.size() - 1;
					}

					// Otherwise pop off stack and then return index
					u32 idx = mIndexFreeList.top();
					mIndexFreeList.pop();
					return idx;
				}

			private: 
				Vector<Resource> mResources;				// Use the index from the handle vector to get index into this array of actual resources
				Vector<u32> mHandleIndices;					// Indices into this vector are returned to the user as handles
				Vector<u32> mReverseIndirectionIndices;		// Indices into this vector give reverse indirection into handle indices from objects
				Stack<u32> mIndexFreeList;					// Free list of most available indices
		};

	*/
}
