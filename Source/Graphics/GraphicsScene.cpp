#include "Graphics/GraphicsScene.h"
#include "Graphics/Renderable.h"
#include "Graphics/DirectionalLight.h"
#include "Graphics/SpotLight.h"
#include "Graphics/PointLight.h"
#include "Graphics/QuadBatch.h"
#include "Graphics/GLTexture.h"
#include "Graphics/Material.h"
#include "Graphics/GraphicsSubsystem.h" 
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <algorithm>

namespace Enjon { 

	GraphicsScene::GraphicsScene()
	{
	}

	GraphicsScene::~GraphicsScene()
	{
		for ( auto& r : mRenderables )
		{
			r->SetGraphicsScene( nullptr );
		}

		for ( auto& r : mNonDepthTestedRenderables )
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

		// Free all memory
		mRenderables.clear( );
		mNonDepthTestedRenderables.clear( ); 
		mQuadBatches.clear();
		mDirectionalLights.clear();
		mPointLights.clear();
		mSpotLights.clear(); 
	}

	//====================================================================================================

	void GraphicsScene::SortRenderables( RenderableSortType type )
	{
		switch( type )
		{
			case RenderableSortType::MATERIAL:
			{
				std::stable_sort(mSortedRenderables.begin(), mSortedRenderables.end(), CompareMaterial);
			} 
			break;

			case RenderableSortType::DEPTH:
			{
				std::stable_sort(mSortedRenderables.begin(), mSortedRenderables.end(), CompareDepth);
			}
			break;

			default:
			case RenderableSortType::NONE: break;
		} 
	}

	//====================================================================================================

	const Vector<Renderable*>& GraphicsScene::GetRenderables( ) const
	{ 
		return mSortedRenderables;
	}

	const Vector<Renderable*>& GraphicsScene::GetNonDepthTestedRenderables( )
	{
		// Sort by depth to camera
		std::stable_sort( mNonDepthTestedRenderables.begin( ), mNonDepthTestedRenderables.end( ), CompareDepth );

		return mNonDepthTestedRenderables;
	} 

	void GraphicsScene::AddRenderable(Renderable* renderable)
	{
		auto query = mRenderables.find(renderable);
		if (query == mRenderables.end())
		{
			mRenderables.insert(renderable);
			renderable->SetGraphicsScene(this);

			// Add to sorted renderables
			mSortedRenderables.push_back( renderable );

			// Sort renderables
			SortRenderables( );
		}
	}

	void GraphicsScene::RemoveRenderable(Renderable* renderable)
	{
		auto query = mRenderables.find(renderable);
		if (query != mRenderables.end())
		{
			renderable->SetGraphicsScene(nullptr);
			mRenderables.erase(renderable);

			// Remove renderable from sorted list
			mSortedRenderables.erase( std::remove( mSortedRenderables.begin( ), mSortedRenderables.end( ), renderable ), mSortedRenderables.end( ) );

			// Sort renderables
			SortRenderables( );
		}
	}

	void GraphicsScene::AddNonDepthTestedRenderable( Renderable* renderable )
	{
		auto query = std::find( mNonDepthTestedRenderables.begin( ), mNonDepthTestedRenderables.end( ), renderable );
		if ( query == mNonDepthTestedRenderables.end( ) )
		{
			mNonDepthTestedRenderables.push_back( renderable );
			renderable->SetGraphicsScene( this );
		}
	}

	void GraphicsScene::RemoveNonDepthTestedRenderable( Renderable* renderable )
	{
		mNonDepthTestedRenderables.erase( std::remove( mNonDepthTestedRenderables.begin( ), mNonDepthTestedRenderables.end( ), renderable ), mNonDepthTestedRenderables.end( ) );
	} 

	void GraphicsScene::AddQuadBatch(QuadBatch* batch)
	{
		auto query = mQuadBatches.find(batch);
		if (query == mQuadBatches.end())
		{
			batch->SetGraphicsScene(this);
			mQuadBatches.insert(batch);
		}
	}

	void GraphicsScene::RemoveQuadBatch(QuadBatch* batch)
	{
		auto query = mQuadBatches.find(batch);
		if (query != mQuadBatches.end())
		{
			batch->SetGraphicsScene(nullptr);
			mQuadBatches.erase(batch);
		}
	}

	void GraphicsScene::AddDirectionalLight(DirectionalLight* light)
	{
		auto query = mDirectionalLights.find(light);
		if (query == mDirectionalLights.end())
		{
			mDirectionalLights.insert(light);
			light->SetGraphicsScene(this);
		}
	}

	void GraphicsScene::RemoveDirectionLight(DirectionalLight* light)
	{
		auto query = mDirectionalLights.find(light);
		if (query != mDirectionalLights.end())
		{
			mDirectionalLights.erase(light);
			light->SetGraphicsScene(nullptr);
		}
	}

	void GraphicsScene::AddPointLight(PointLight* light)
	{
		auto query = mPointLights.find(light);
		if (query == mPointLights.end())
		{
			mPointLights.insert(light);
			light->SetGraphicsScene(this);
		}
	}

	void GraphicsScene::RemovePointLight(PointLight* light)
	{
		auto query = mPointLights.find(light);
		if (query != mPointLights.end())
		{
			mPointLights.erase(light);
			light->SetGraphicsScene(nullptr);
		}
	}

	void GraphicsScene::AddSpotLight(SpotLight* light)
	{
		auto query = mSpotLights.find(light);
		if (query == mSpotLights.end())
		{
			mSpotLights.insert(light);
			light->SetGraphicsScene(this);
		}
	}

	void GraphicsScene::RemoveSpotLight(SpotLight* light)
	{
		auto query = mSpotLights.find(light);
		if (query != mSpotLights.end())
		{
			mSpotLights.erase(light);
			light->SetGraphicsScene(nullptr);
		}
	}

	void GraphicsScene::SetAmbientSettings(AmbientSettings& settings)
	{
		mAmbientSettings = settings;
	}

	void GraphicsScene::SetAmbientColor(ColorRGBA32& color)
	{
		mAmbientSettings.mColor = color;
		mAmbientSettings.mIntensity = color.a;
	}

	bool GraphicsScene::CompareDepth(Renderable* a, Renderable* b)
	{
		// Get camera position
		const GraphicsSubsystem* gfx = Engine::GetInstance( )->GetSubsystemCatalog( )->Get<GraphicsSubsystem>( );
		v3 camPos = gfx->GetGraphicsSceneCamera()->GetPosition();

		// Get a pos
		v3 aPos = a->GetPosition();
		// Get b pos
		v3 bPos = b->GetPosition();

		f32 aDist = (camPos - aPos).Length();
		f32 bDist = (camPos - bPos).Length();

		return aDist < bDist;
	}

	// TODO(): Come up with better way to compare materials
	bool GraphicsScene::CompareMaterial(Renderable* a, Renderable* b)
	{
		// Need a good metric for comparing materials by shader program ids...
		if ( a && b )
		{
			u32 pA = a->GetMaterial( )->GetShaderGraph( )->GetShader( Enjon::ShaderPassType::StaticGeom )->GetProgramID( );
			u32 pB = a->GetMaterial( )->GetShaderGraph( )->GetShader( Enjon::ShaderPassType::StaticGeom )->GetProgramID( );
			return pA > pB;
		}

		return false;

		// TODO(): Set this up to where materials have a unique id and sort by that
		//AssetHandle<Texture> texA = a->GetMaterial()->GetTexture(TextureSlotType::Albedo);
		//AssetHandle<Texture> texB = b->GetMaterial()->GetTexture(TextureSlotType::Albedo);

		//return texA.Get()->GetTextureId() > texB.Get()->GetTextureId();
	}

}

