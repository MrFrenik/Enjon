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
		if (query == mRenderables.end())
		{
			renderable->SetScene(nullptr);
			mRenderables.erase(renderable);
		}
	}

	bool Scene::CompareMaterial(EG::Renderable* a, EG::Renderable* b)
	{
		// TODO(): Set this up to where materials have a unique id and sort by that
		GLTexture texA = a->GetMaterial()->GetTexture(EG::TextureSlotType::ALBEDO);
		GLTexture texB = b->GetMaterial()->GetTexture(EG::TextureSlotType::ALBEDO);

		return texA.id < texB.id;
	}

}}

