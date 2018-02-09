#include "Graphics/Scene.h"
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

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
		// Free all memory
		mRenderables.clear( );
		mNonDepthTestedRenderables.clear( ); 
		mQuadBatches.clear();
		mDirectionalLights.clear();
		mPointLights.clear();
		mSpotLights.clear(); 
	}

	//====================================================================================================

	void Scene::SortRenderables( RenderableSortType type )
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

	const Vector<Renderable*>& Scene::GetRenderables( ) const
	{ 
		return mSortedRenderables;
	}

	const Vector<Renderable*>& Scene::GetNonDepthTestedRenderables( )
	{
		// Sort by depth to camera
		std::stable_sort( mNonDepthTestedRenderables.begin( ), mNonDepthTestedRenderables.end( ), CompareDepth );

		return mNonDepthTestedRenderables;
	} 

	void Scene::AddRenderable(Renderable* renderable)
	{
		auto query = mRenderables.find(renderable);
		if (query == mRenderables.end())
		{
			mRenderables.insert(renderable);
			renderable->SetScene(this);

			// Add to sorted renderables
			mSortedRenderables.push_back( renderable );

			// Sort renderables
			SortRenderables( );
		}
	}

	void Scene::RemoveRenderable(Renderable* renderable)
	{
		auto query = mRenderables.find(renderable);
		if (query != mRenderables.end())
		{
			renderable->SetScene(nullptr);
			mRenderables.erase(renderable);

			// Remove renderable from sorted list
			mSortedRenderables.erase( std::remove( mSortedRenderables.begin( ), mSortedRenderables.end( ), renderable ), mSortedRenderables.end( ) );

			// Sort renderables
			SortRenderables( );
		}
	}

	void Scene::AddNonDepthTestedRenderable( Renderable* renderable )
	{
		auto query = std::find( mNonDepthTestedRenderables.begin( ), mNonDepthTestedRenderables.end( ), renderable );
		if ( query == mNonDepthTestedRenderables.end( ) )
		{
			mNonDepthTestedRenderables.push_back( renderable );
			renderable->SetScene( this );
		}
	}

	void Scene::RemoveNonDepthTestedRenderable( Renderable* renderable )
	{
		mNonDepthTestedRenderables.erase( std::remove( mNonDepthTestedRenderables.begin( ), mNonDepthTestedRenderables.end( ), renderable ), mNonDepthTestedRenderables.end( ) );
	} 

	void Scene::AddQuadBatch(QuadBatch* batch)
	{
		auto query = mQuadBatches.find(batch);
		if (query == mQuadBatches.end())
		{
			batch->SetScene(this);
			mQuadBatches.insert(batch);
		}
	}

	void Scene::RemoveQuadBatch(QuadBatch* batch)
	{
		auto query = mQuadBatches.find(batch);
		if (query != mQuadBatches.end())
		{
			batch->SetScene(nullptr);
			mQuadBatches.erase(batch);
		}
	}

	void Scene::AddDirectionalLight(DirectionalLight* light)
	{
		auto query = mDirectionalLights.find(light);
		if (query == mDirectionalLights.end())
		{
			mDirectionalLights.insert(light);
			light->SetScene(this);
		}
	}

	void Scene::RemoveDirectionLight(DirectionalLight* light)
	{
		auto query = mDirectionalLights.find(light);
		if (query != mDirectionalLights.end())
		{
			mDirectionalLights.erase(light);
			light->SetScene(nullptr);
		}
	}

	void Scene::AddPointLight(PointLight* light)
	{
		auto query = mPointLights.find(light);
		if (query == mPointLights.end())
		{
			mPointLights.insert(light);
			light->SetScene(this);
		}
	}

	void Scene::RemovePointLight(PointLight* light)
	{
		auto query = mPointLights.find(light);
		if (query != mPointLights.end())
		{
			mPointLights.erase(light);
			light->SetScene(nullptr);
		}
	}

	void Scene::AddSpotLight(SpotLight* light)
	{
		auto query = mSpotLights.find(light);
		if (query == mSpotLights.end())
		{
			mSpotLights.insert(light);
			light->SetScene(this);
		}
	}

	void Scene::RemoveSpotLight(SpotLight* light)
	{
		auto query = mSpotLights.find(light);
		if (query != mSpotLights.end())
		{
			mSpotLights.erase(light);
			light->SetScene(nullptr);
		}
	}

	void Scene::SetAmbientSettings(AmbientSettings& settings)
	{
		mAmbientSettings = settings;
	}

	void Scene::SetAmbientColor(ColorRGBA32& color)
	{
		mAmbientSettings.mColor = color;
		mAmbientSettings.mIntensity = color.a;
	}

	bool Scene::CompareDepth(Renderable* a, Renderable* b)
	{
		// Get camera position
		const GraphicsSubsystem* gfx = Engine::GetInstance( )->GetSubsystemCatalog( )->Get<GraphicsSubsystem>( );
		v3 camPos = gfx->GetSceneCamera()->GetPosition();

		// Get a pos
		v3 aPos = a->GetPosition();
		// Get b pos
		v3 bPos = b->GetPosition();

		f32 aDist = (camPos - aPos).Length();
		f32 bDist = (camPos - bPos).Length();

		return aDist < bDist;
	}

	// TODO(): Come up with better way to compare materials
	bool Scene::CompareMaterial(Renderable* a, Renderable* b)
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

