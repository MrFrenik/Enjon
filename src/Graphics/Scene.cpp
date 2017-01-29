#include "Graphics/Scene.h"
#include "Graphics/Renderable.h"
#include "Graphics/DirectionalLight.h"
#include "Graphics/SpotLight.h"
#include "Graphics/PointLight.h"
#include "Graphics/QuadBatch.h"
#include "Graphics/GLTexture.h"
#include "Graphics/Material.h"

#include <algorithm>

namespace Enjon { namespace Graphics {

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	std::vector<EG::Renderable*> Scene::GetRenderables(RenderableSortType type)
	{
		std::vector<EG::Renderable*> renderables(mRenderables.begin(), mRenderables.end());	

		switch(type)
		{
			case RenderableSortType::MATERIAL:
			{
				std::stable_sort(renderables.begin(), renderables.end(), CompareMaterial);
			} 
			break;

			case RenderableSortType::NONE: break;
			default: break;
		}

		return renderables;
	}

	void Scene::AddRenderable(EG::Renderable* renderable)
	{
		auto query = mRenderables.find(renderable);
		if (query == mRenderables.end())
		{
			mRenderables.insert(renderable);
			renderable->SetScene(this);
		}
	}

	void Scene::RemoveRenderable(EG::Renderable* renderable)
	{
		auto query = mRenderables.find(renderable);
		if (query != mRenderables.end())
		{
			renderable->SetScene(nullptr);
			mRenderables.erase(renderable);
		}
	}

	void Scene::AddQuadBatch(EG::QuadBatch* batch)
	{
		auto query = mQuadBatches.find(batch);
		if (query == mQuadBatches.end())
		{
			batch->SetScene(this);
			mQuadBatches.insert(batch);
		}
	}

	void Scene::RemoveQuadBatch(EG::QuadBatch* batch)
	{
		auto query = mQuadBatches.find(batch);
		if (query != mQuadBatches.end())
		{
			batch->SetScene(nullptr);
			mQuadBatches.erase(batch);
		}
	}

	void Scene::AddDirectionalLight(EG::DirectionalLight* light)
	{
		auto query = mDirectionalLights.find(light);
		if (query == mDirectionalLights.end())
		{
			mDirectionalLights.insert(light);
			light->SetScene(this);
		}
	}

	void Scene::RemoveDirectionLight(EG::DirectionalLight* light)
	{
		auto query = mDirectionalLights.find(light);
		if (query != mDirectionalLights.end())
		{
			mDirectionalLights.erase(light);
			light->SetScene(nullptr);
		}
	}

	void Scene::AddPointLight(EG::PointLight* light)
	{
		auto query = mPointLights.find(light);
		if (query == mPointLights.end())
		{
			mPointLights.insert(light);
			light->SetScene(this);
		}
	}

	void Scene::RemovePointLight(EG::PointLight* light)
	{
		auto query = mPointLights.find(light);
		if (query != mPointLights.end())
		{
			mPointLights.erase(light);
			light->SetScene(nullptr);
		}
	}

	void Scene::AddSpotLight(EG::SpotLight* light)
	{
		auto query = mSpotLights.find(light);
		if (query == mSpotLights.end())
		{
			mSpotLights.insert(light);
			light->SetScene(this);
		}
	}

	void Scene::RemoveSpotLight(EG::SpotLight* light)
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

	void Scene::SetAmbientColor(EG::ColorRGBA16& color)
	{
		mAmbientSettings.mColor = color;
		mAmbientSettings.mIntensity = color.a;
	}

	bool Scene::CompareMaterial(EG::Renderable* a, EG::Renderable* b)
	{
		// TODO(): Set this up to where materials have a unique id and sort by that
		GLTexture texA = a->GetMaterial()->GetTexture(EG::TextureSlotType::ALBEDO);
		GLTexture texB = b->GetMaterial()->GetTexture(EG::TextureSlotType::ALBEDO);

		return texA.id < texB.id;
	}

}}

