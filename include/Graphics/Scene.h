#pragma once
#ifndef ENJON_SCENE_H
#define ENJON_SCENE_H

#include "Defines.h"
#include "System/Types.h"
#include "Graphics/Color.h"

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

	struct AmbientSettings
	{
		AmbientSettings()
		{
			mColor = EG::RGBA16_White();
			mIntensity = 1.0f;
		}

		AmbientSettings(EG::ColorRGBA16& color, float intensity)
			: mColor(color), mIntensity(intensity)
		{
		}

		EG::ColorRGBA16 mColor;	
		float mIntensity;
	};

	class Scene
	{
		public:
			Scene();
			~Scene();

			void AddRenderable(EG::Renderable* renderable);
			void RemoveRenderable(EG::Renderable* renderable);

			void AddQuadBatch(EG::QuadBatch* batch);
			void RemoveQuadBatch(EG::QuadBatch* batch);

			void AddDirectionalLight(EG::DirectionalLight* light);
			void RemoveDirectionLight(EG::DirectionalLight* light);

			void AddPointLight(EG::PointLight* light);
			void RemovePointLight(EG::PointLight* light);

			void AddSpotLight(EG::SpotLight* light);
			void RemoveSpotLight(EG::SpotLight* light);

			AmbientSettings* GetAmbientSettings() { return &mAmbientSettings; }
			void SetAmbientSettings(AmbientSettings& settings);
			void SetAmbientColor(EG::ColorRGBA16& color);

			void SetSun(EG::DirectionalLight* light) { mSun = light; }
			EG::DirectionalLight* GetSun() { return mSun; }

			std::vector<EG::Renderable*> GetRenderables(RenderableSortType type = RenderableSortType::MATERIAL);
			std::set<EG::QuadBatch*>* GetQuadBatches() { return &mQuadBatches; }
			std::set<EG::DirectionalLight*>* GetDirectionalLights() { return &mDirectionalLights; }
			std::set<EG::PointLight*>* GetPointLights() { return &mPointLights; }
			std::set<EG::SpotLight*>* GetSpotLights() { return &mSpotLights; }

		private:
			std::set<EG::Renderable*> 		mRenderables;
			std::set<EG::QuadBatch*> 		mQuadBatches;
			std::set<EG::DirectionalLight*> mDirectionalLights;
			std::set<EG::PointLight*> 		mPointLights;
			std::set<EG::SpotLight*> 		mSpotLights;

			AmbientSettings 				mAmbientSettings;

			EG::DirectionalLight* 			mSun = nullptr;

			static bool CompareMaterial(EG::Renderable* a, EG::Renderable* b);
	};
}}

#endif