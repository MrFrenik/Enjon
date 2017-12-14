#pragma once
#ifndef ENJON_SCENE_H
#define ENJON_SCENE_H

#include "Defines.h"
#include "System/Types.h"
#include "Graphics/Color.h"
#include "Base/Object.h"

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
			mColor = RGBA32_White();
			mIntensity = 1.0f;
		}

		AmbientSettings(ColorRGBA32& color, float intensity)
			: mColor(color), mIntensity(intensity)
		{
		}

		ColorRGBA32 mColor;	
		float mIntensity;
	};

	using RenderableID = u32;

	ENJON_CLASS( )
	class Scene : public Enjon::Object
	{
		ENJON_CLASS_BODY( )

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
			void SetAmbientColor(ColorRGBA32& color);

			/*
			* @brief
			*/
			void SetSun(DirectionalLight* light) { mSun = light; }
			
			/*
			* @brief
			*/
			DirectionalLight* GetSun() const { return mSun; }

			/*
			* @brief
			*/
			const std::vector<Renderable*>& GetRenderables() const;

			/*
			* @brief
			*/
			const std::set<QuadBatch*>& GetQuadBatches() const { return mQuadBatches; }

			/*
			* @brief
			*/
			const std::set<DirectionalLight*>& GetDirectionalLights() const { return mDirectionalLights; }

			/*
			* @brief
			*/
			const std::set<PointLight*>& GetPointLights() const { return mPointLights; }

			/*
			* @brief
			*/
			const std::set<SpotLight*>& GetSpotLights() const { return mSpotLights; }

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

			/*
			* @brief
			*/
			static bool CompareMaterial(Renderable* a, Renderable* b);

			/*
			* @brief
			*/
			static bool CompareDepth(Renderable* a, Renderable* b);

		private:

			ENJON_PROPERTY( )
			Vector< Renderable* > mSortedRenderables; 

			std::set<Renderable*> mRenderables;
			std::set<QuadBatch*> mQuadBatches;
			std::set<DirectionalLight*> mDirectionalLights;
			std::set<PointLight*> mPointLights;
			std::set<SpotLight*> mSpotLights; 
			AmbientSettings mAmbientSettings; 
			DirectionalLight* mSun = nullptr;


			// Scene should own all the data for renderables, lights, etc. and should hand those out on request from components
			// Or could have a graphics subsystem which holds all of these objects?
			/*
				template <typename T, typename... Args>
				const T* GraphicsFactory::Construct( Args&&... args )
				{
						
				}

				// Should scene own its own camera? How will this work, exactly?
			*/

	};
}

#endif