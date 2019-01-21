#include "Graphics/GraphicsScene.h"
#include "Graphics/Renderable.h"
#include "Graphics/DirectionalLight.h"
#include "Graphics/SpotLight.h"
#include "Graphics/PointLight.h"
#include "Graphics/QuadBatch.h"
#include "Graphics/Material.h"
#include "Graphics/Camera.h"
#include "Graphics/StaticMeshRenderable.h"
#include "Graphics/SkeletalMeshRenderable.h"
#include "Graphics/GraphicsSubsystem.h" 
#include "Asset/AssetManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <algorithm>

namespace Enjon 
{ 
	//=========================================================================================

	void GraphicsScene::ExplicitConstructor( )
	{
	}

	void GraphicsScene::ExplicitDestructor()
	{
		for ( auto& r : mStaticMeshRenderables )
		{
			r->SetGraphicsScene( nullptr );
		}

		for ( auto& r : mSkeletalMeshRenderables )
		{
			r->SetGraphicsScene( nullptr );
		}

		for ( auto& r : mNonDepthTestedStaticMeshRenderables )
		{
			r->SetGraphicsScene( nullptr );
		}

		for ( auto& q : mQuadBatches )
		{
			q->SetGraphicsScene( nullptr );
		}

		for ( auto& d : mDirectionalLights )
		{
			d->SetGraphicsScene( nullptr );
		}

		for ( auto& p : mPointLights )
		{
			p->SetGraphicsScene( nullptr );
		}

		for ( auto& s : mSpotLights )
		{
			s->SetGraphicsScene( nullptr );
		}

		mStaticMeshRenderableSlotArray.clear( );
		mPointLightSlotArray.clear( );
		mDirectionalLightSlotArray.clear( );

		// Free all memory
		mStaticMeshRenderables.clear( );
		mNonDepthTestedStaticMeshRenderables.clear( ); 
		mQuadBatches.clear();
		mDirectionalLights.clear();
		mPointLights.clear();
		mSpotLights.clear(); 
	}

	//====================================================================================================

	void GraphicsScene::SortStaticMeshRenderables( RenderableSortType type )
	{
		switch( type )
		{
			case RenderableSortType::MATERIAL:
			{
				std::stable_sort(mSortedStaticMeshRenderables.begin(), mSortedStaticMeshRenderables.end(), CompareMaterial);
			} 
			break;

			case RenderableSortType::DEPTH:
			{
				std::stable_sort(mSortedStaticMeshRenderables.begin(), mSortedStaticMeshRenderables.end(), CompareDepth);
			}
			break;

			default:
			case RenderableSortType::NONE: break;
		} 
	}

	//====================================================================================================

	Camera* GraphicsScene::GetActiveCamera( )
	{
		if ( !mActiveCamera )
		{
			return &mDefaultCamera;
		}

		return mActiveCamera;
	}

	//====================================================================================================

	void GraphicsScene::SetActiveCamera( Camera* camera )
	{
		mActiveCamera = camera;
	}

	//====================================================================================================

	void GraphicsScene::DeallocateDirectionalLight( const ResourceHandle< DirectionalLight >& light )
	{
		mDirectionalLightSlotArray.erase( light );
	}

	//====================================================================================================

	void GraphicsScene::DeallocatePointLight( const ResourceHandle< PointLight >& light )
	{
		mPointLightSlotArray.erase( light );
	}

	//====================================================================================================

	void GraphicsScene::DeallocateStaticMeshRenderable( const ResourceHandle< StaticMeshRenderable >& renderable )
	{
		mStaticMeshRenderableSlotArray.erase( renderable );
	}

	//====================================================================================================

	ResourceHandle< PointLight > GraphicsScene::AllocatePointLight( )
	{
		ResourceHandle< PointLight > pl = mPointLightSlotArray.emplace( );
		pl->SetGraphicsScene( this );
		return pl;
	}

	//====================================================================================================

	ResourceHandle< DirectionalLight > GraphicsScene::AllocateDirectionalLight( )
	{
		ResourceHandle< DirectionalLight > dl = mDirectionalLightSlotArray.emplace( ); 
		dl->SetGraphicsScene( this );
		return dl;
	}

	//====================================================================================================

	ResourceHandle< StaticMeshRenderable > GraphicsScene::AllocateStaticMeshRenderable( const u32& entityID ) 
	{ 
		// Construct and new static mesh renderable
		AssetManager* am = EngineSubsystem( AssetManager );

		auto handle = mStaticMeshRenderableSlotArray.emplace( ); 

		handle->SetGraphicsScene( this );
		handle->SetRenderableID( entityID );
		handle->SetMesh( am->GetDefaultAsset< Mesh >( ) ); 
 
		// Set default materials for all material elements
		for ( u32 i = 0; i < handle->GetMesh( )->GetSubMeshCount( ); ++i ) 
		{
			handle->SetMaterial( am->GetDefaultAsset< Material >( ), i ); 
		} 

		return handle;
	} 

	//====================================================================================================

	const Vector<StaticMeshRenderable*>& GraphicsScene::GetStaticMeshRenderables( ) const
	{ 
		return mSortedStaticMeshRenderables;
	}

	//====================================================================================================

	const Vector<StaticMeshRenderable*>& GraphicsScene::GetNonDepthTestedStaticMeshRenderables( )
	{
		// Sort by depth to camera
		std::stable_sort( mNonDepthTestedStaticMeshRenderables.begin( ), mNonDepthTestedStaticMeshRenderables.end( ), CompareDepth );

		return mNonDepthTestedStaticMeshRenderables;
	} 

	//====================================================================================================

	const Vector<SkeletalMeshRenderable*>& GraphicsScene::GetSkeletalMeshRenderables( ) const
	{
		return mSortedSkeletalMeshRenderables;
	}

	//====================================================================================================

	void GraphicsScene::AddSkeletalMeshRenderable( SkeletalMeshRenderable* renderable )
	{
		auto query = mSkeletalMeshRenderables.find(renderable);
		if (query == mSkeletalMeshRenderables.end())
		{
			mSkeletalMeshRenderables.insert(renderable);
			renderable->SetGraphicsScene(this);

			// Add to sorted renderables
			mSortedSkeletalMeshRenderables.push_back( renderable );

			// Sort renderables
			//SortRenderables( );
		}
	}

	//====================================================================================================

	void GraphicsScene::RemoveSkeletalMeshRenderable( SkeletalMeshRenderable* renderable )
	{
		auto query = mSkeletalMeshRenderables.find(renderable);
		if (query != mSkeletalMeshRenderables.end())
		{
			renderable->SetGraphicsScene(nullptr);
			mSkeletalMeshRenderables.erase(renderable);

			// Remove renderable from sorted list
			mSortedSkeletalMeshRenderables.erase( std::remove( mSortedSkeletalMeshRenderables.begin( ), mSortedSkeletalMeshRenderables.end( ), renderable ), mSortedSkeletalMeshRenderables.end( ) );

			// Sort renderables
			//SortRenderables( );
		}
	}

	//====================================================================================================

	void GraphicsScene::AddStaticMeshRenderable(StaticMeshRenderable* renderable)
	{
		auto query = mStaticMeshRenderables.find(renderable);
		if (query == mStaticMeshRenderables.end())
		{
			mStaticMeshRenderables.insert(renderable);
			renderable->SetGraphicsScene(this);

			// Add to sorted renderables
			mSortedStaticMeshRenderables.push_back( renderable );

			// Sort renderables
			//SortRenderables( );
		}
	}

	//====================================================================================================

	void GraphicsScene::RemoveStaticMeshRenderable(StaticMeshRenderable* renderable)
	{
		auto query = mStaticMeshRenderables.find(renderable);
		if (query != mStaticMeshRenderables.end())
		{
			renderable->SetGraphicsScene(nullptr);
			mStaticMeshRenderables.erase(renderable);

			// Remove renderable from sorted list
			mSortedStaticMeshRenderables.erase( std::remove( mSortedStaticMeshRenderables.begin( ), mSortedStaticMeshRenderables.end( ), renderable ), mSortedStaticMeshRenderables.end( ) );

			// Sort renderables
			//SortRenderables( );
		}
	}

	//====================================================================================================

	void GraphicsScene::AddNonDepthTestedStaticMeshRenderable( StaticMeshRenderable* renderable )
	{
		auto query = std::find( mNonDepthTestedStaticMeshRenderables.begin( ), mNonDepthTestedStaticMeshRenderables.end( ), renderable );
		if ( query == mNonDepthTestedStaticMeshRenderables.end( ) )
		{
			mNonDepthTestedStaticMeshRenderables.push_back( renderable );
			renderable->SetGraphicsScene( this );
		}
	}

	//====================================================================================================

	void GraphicsScene::RemoveNonDepthTestedStaticMeshRenderable( StaticMeshRenderable* renderable )
	{
		mNonDepthTestedStaticMeshRenderables.erase( std::remove( mNonDepthTestedStaticMeshRenderables.begin( ), mNonDepthTestedStaticMeshRenderables.end( ), renderable ), mNonDepthTestedStaticMeshRenderables.end( ) );
	} 

	//====================================================================================================

	void GraphicsScene::AddQuadBatch(QuadBatch* batch)
	{
		auto query = mQuadBatches.find(batch);
		if (query == mQuadBatches.end())
		{
			batch->SetGraphicsScene(this);
			mQuadBatches.insert(batch);
		}
	}

	//====================================================================================================

	void GraphicsScene::RemoveQuadBatch(QuadBatch* batch)
	{
		auto query = mQuadBatches.find(batch);
		if (query != mQuadBatches.end())
		{
			batch->SetGraphicsScene(nullptr);
			mQuadBatches.erase(batch);
		}
	}

	//==================================================================================================== 

	void GraphicsScene::AddDirectionalLight(DirectionalLight* light)
	{
		auto query = mDirectionalLights.find(light);
		if (query == mDirectionalLights.end())
		{
			mDirectionalLights.insert(light);
			light->SetGraphicsScene(this);
		}
	}

	//==================================================================================================== 

	void GraphicsScene::RemoveDirectionalLight(DirectionalLight* light)
	{
		auto query = mDirectionalLights.find(light);
		if (query != mDirectionalLights.end())
		{
			mDirectionalLights.erase(light);
			light->SetGraphicsScene(nullptr);
		}
	}

	//==================================================================================================== 

	void GraphicsScene::AddPointLight(PointLight* light)
	{
		auto query = mPointLights.find(light);
		if (query == mPointLights.end())
		{
			mPointLights.insert(light);
			light->SetGraphicsScene(this);
		}
	}

	//==================================================================================================== 

	void GraphicsScene::RemovePointLight(PointLight* light)
	{
		auto query = mPointLights.find(light);
		if (query != mPointLights.end())
		{
			mPointLights.erase(light);
			light->SetGraphicsScene(nullptr);
		}
	}

	//==================================================================================================== 

	void GraphicsScene::AddSpotLight(SpotLight* light)
	{
		auto query = mSpotLights.find(light);
		if (query == mSpotLights.end())
		{
			mSpotLights.insert(light);
			light->SetGraphicsScene(this);
		}
	}

	//==================================================================================================== 

	void GraphicsScene::RemoveSpotLight(SpotLight* light)
	{
		auto query = mSpotLights.find(light);
		if (query != mSpotLights.end())
		{
			mSpotLights.erase(light);
			light->SetGraphicsScene(nullptr);
		}
	} 

	//==================================================================================================

	void GraphicsScene::AddCamera( Camera* camera )
	{
		auto query = mCameras.find( camera );
		if ( query == mCameras.end( ) )
		{
			mCameras.insert( camera );
			camera->SetGraphicsScene( this );
		}
	}

	//==================================================================================================

	void GraphicsScene::RemoveCamera( Camera* camera )
	{
		auto query = mCameras.find( camera );
		if ( query != mCameras.end( ) )
		{
			mCameras.erase( camera );
			camera->SetGraphicsScene( nullptr );

			// Set active camera to next avilable camera
			if ( mCameras.size( ) )
			{
				SetActiveCamera( *mCameras.begin() );
			}
		}
	}

	//==================================================================================================

	void GraphicsScene::SetAmbientSettings(AmbientSettings& settings)
	{
		mAmbientSettings = settings;
	}

	//==================================================================================================

	void GraphicsScene::SetAmbientColor(ColorRGBA32& color)
	{
		mAmbientSettings.mColor = color;
		mAmbientSettings.mIntensity = color.a;
	}

	//==================================================================================================

	bool GraphicsScene::CompareDepth(Renderable* a, Renderable* b)
	{
		// Get camera position
		GraphicsSubsystem* gfx = EngineSubsystem( GraphicsSubsystem );
		v3 camPos = gfx->GetGraphicsSceneCamera()->GetPosition();

		// Get a pos
		v3 aPos = a->GetPosition();
		// Get b pos
		v3 bPos = b->GetPosition();

		f32 aDist = (camPos - aPos).Length();
		f32 bDist = (camPos - bPos).Length();

		return aDist < bDist;
	}

	//==================================================================================================

	// TODO(): Come up with better way to compare materials
	bool GraphicsScene::CompareMaterial(Renderable* a, Renderable* b)
	{
		// Need a good metric for comparing materials by shader program ids...
		if ( a && b )
		{
			u32 pA = a->GetMaterial( 0 )->GetShaderGraph( )->GetShader( Enjon::ShaderPassType::Deferred_StaticGeom )->GetProgramID( );
			u32 pB = a->GetMaterial( 0 )->GetShaderGraph( )->GetShader( Enjon::ShaderPassType::Deferred_StaticGeom )->GetProgramID( );
			return pA > pB;
		}

		return false;

		// TODO(): Set this up to where materials have a unique id and sort by that
		//AssetHandle<Texture> texA = a->GetMaterial()->GetTexture(TextureSlotType::Albedo);
		//AssetHandle<Texture> texB = b->GetMaterial()->GetTexture(TextureSlotType::Albedo);

		//return texA.Get()->GetTextureId() > texB.Get()->GetTextureId();
	}

	//================================================================================================== 

	Vector< StaticMeshRenderable >* GraphicsScene::GetStaticMeshRenderableArray()
	{
		return ( mStaticMeshRenderableSlotArray.data() );
	} 

	//================================================================================================== 

	Vector< PointLight >* GraphicsScene::GetPointLightArray( )
	{
		return mPointLightSlotArray.data( );
	}

	//================================================================================================== 

	Vector< DirectionalLight >* GraphicsScene::GetDirectionalLightArray( )
	{
		return mDirectionalLightSlotArray.data( );
	}

	//================================================================================================== 
}

