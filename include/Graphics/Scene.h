#pragma once
#ifndef ENJON_SCENE_H
#define ENJON_SCENE_H

#include "Defines.h"
#include "System/Types.h"

#include <set>
#include <vector>

namespace Enjon { namespace Graphics { 

	class Renderable;
	class DirectionalLight;
	class PointLight;
	class SpotLight;
	class QuadBatch;

	enum class RenderableSortType
	{
		MATERIAL,
		NONE
	};

	class Scene
	{
		public:
			Scene();
			~Scene();

			void AddRenderable(EG::Renderable* renderable);
			void RemoveRenderable(EG::Renderable* renderable);

			std::vector<EG::Renderable*> GetRenderables(RenderableSortType type = RenderableSortType::MATERIAL);
			std::set<EG::DirectionalLight*>* GetDirectionalLights() { return &mDirectionalLights; }
			std::set<EG::PointLight*>* GetPointLights() { return &mPointLights; }
			std::set<EG::SpotLight*>* GetSpotLights() { return &mSpotLights; }

		private:
			std::set<EG::Renderable*> 		mRenderables;
			std::set<EG::QuadBatch*> 		mQuadBatches;
			std::set<EG::DirectionalLight*> mDirectionalLights;
			std::set<EG::PointLight*> 		mPointLights;
			std::set<EG::SpotLight*> 		mSpotLights;

			static bool CompareMaterial(EG::Renderable* a, EG::Renderable* b);
	};
}}

#endif