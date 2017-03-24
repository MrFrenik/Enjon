#pragma once
#ifndef ENJON_SCENE_H
#define ENJON_SCENE_H

#include "Defines.h"
#include "System/Types.h"
#include "Graphics/Color.h"

#include <set>
#include <vector>

namespace Enjon { 

	class Renderable;
	class DirectionalLight;
	class PointLight;
	class SpotLight;
	class QuadBatch;

	enum class RenderableSortType
	{
		MATERIAL,
		DEPTH,
		NONE
	};

	struct AmbientSettings
	{
		AmbientSettings()
		{
			mColor = RGBA16_White();
			mIntensity = 1.0f;
		}

		AmbientSettings(ColorRGBA16& color, float intensity)
			: mColor(color), mIntensity(intensity)
		{
		}

		ColorRGBA16 mColor;	
		float mIntensity;
	};

	using RenderableID = u32;

	class Scene
	{
		public:
			Scene();
			~Scene();

			void AddRenderable(Renderable* renderable);
			void RemoveRenderable(Renderable* renderable);

			void AddQuadBatch(QuadBatch* batch);
			void RemoveQuadBatch(QuadBatch* batch);

			void AddDirectionalLight(DirectionalLight* light);
			void RemoveDirectionLight(DirectionalLight* light);

			void AddPointLight(PointLight* light);
			void RemovePointLight(PointLight* light);

			void AddSpotLight(SpotLight* light);
			void RemoveSpotLight(SpotLight* light);

			AmbientSettings* GetAmbientSettings() { return &mAmbientSettings; }
			void SetAmbientSettings(AmbientSettings& settings);
			void SetAmbientColor(ColorRGBA16& color);

			void SetSun(DirectionalLight* light) { mSun = light; }
			DirectionalLight* GetSun() { return mSun; }

			std::vector<Renderable*> GetRenderables(RenderableSortType type = RenderableSortType::MATERIAL);
			std::set<QuadBatch*>* GetQuadBatches() { return &mQuadBatches; }
			std::set<DirectionalLight*>* GetDirectionalLights() { return &mDirectionalLights; }
			std::set<PointLight*>* GetPointLights() { return &mPointLights; }
			std::set<SpotLight*>* GetSpotLights() { return &mSpotLights; }

		private:
			void AssignRenderableID(Renderable* renderable);

		private:
			std::set<Renderable*> 		mRenderables;
			std::set<QuadBatch*> 		mQuadBatches;
			std::set<DirectionalLight*> mDirectionalLights;
			std::set<PointLight*> 		mPointLights;
			std::set<SpotLight*> 		mSpotLights;

			AmbientSettings 				mAmbientSettings;

			DirectionalLight* 			mSun = nullptr;

			static bool CompareMaterial(Renderable* a, Renderable* b);
			static bool CompareDepth(Renderable* a, Renderable* b);
	};
}

#endif