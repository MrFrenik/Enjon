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
			/*
			* @brief
			*/
			Scene();

			/*
			* @brief
			*/
			~Scene();

			/*
			* @brief
			*/
			void AddRenderable(Renderable* renderable);

			/*
			* @brief
			*/
			void RemoveRenderable(Renderable* renderable);

			/*
			* @brief
			*/
			void AddQuadBatch(QuadBatch* batch);

			/*
			* @brief
			*/
			void RemoveQuadBatch(QuadBatch* batch);

			/*
			* @brief
			*/
			void AddDirectionalLight(DirectionalLight* light);
			
			/*
			* @brief
			*/
			void RemoveDirectionLight(DirectionalLight* light);

			/*
			* @brief
			*/
			void AddPointLight(PointLight* light);

			/*
			* @brief
			*/
			void RemovePointLight(PointLight* light);

			/*
			* @brief
			*/
			void AddSpotLight(SpotLight* light);

			/*
			* @brief
			*/
			void RemoveSpotLight(SpotLight* light);

			/*
			* @brief
			*/
			AmbientSettings* GetAmbientSettings() { return &mAmbientSettings; }

			/*
			* @brief
			*/
			void SetAmbientSettings(AmbientSettings& settings);

			/*
			* @brief
			*/
			void SetAmbientColor(ColorRGBA16& color);

			/*
			* @brief
			*/
			void SetSun(DirectionalLight* light) { mSun = light; }
			
			/*
			* @brief
			*/
			DirectionalLight* GetSun() { return mSun; }

			/*
			* @brief
			*/
			std::vector<Renderable*> GetRenderables();

			/*
			* @brief
			*/
			std::set<QuadBatch*>* GetQuadBatches() { return &mQuadBatches; }

			/*
			* @brief
			*/
			std::set<DirectionalLight*>* GetDirectionalLights() { return &mDirectionalLights; }

			/*
			* @brief
			*/
			std::set<PointLight*>* GetPointLights() { return &mPointLights; }

			/*
			* @brief
			*/
			std::set<SpotLight*>* GetSpotLights() { return &mSpotLights; }

		private:
			
			/*
			* @brief
			*/
			void AssignRenderableID(Renderable* renderable);

			/*
			* @brief
			*/
			void SortRenderables( RenderableSortType type = RenderableSortType::MATERIAL );

		private:
			std::vector< Renderable* >	mSortedRenderables;
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