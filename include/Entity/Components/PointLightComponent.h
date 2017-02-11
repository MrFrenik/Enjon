#pragma once
#ifndef ENJON_POINT_LIGHT_COMPONENT_H
#define ENJON_POINT_LIGHT_COMPONENT_H

#include "Entity/Component.h"
#include "Graphics/PointLight.h"

namespace Enjon
{
	class PointLightComponent : public Component
	{
		public:
			PointLightComponent();
			~PointLightComponent();

			virtual void Update(float dt) override;
			Enjon::Graphics::PointLight* GetLight() { return &mLight; }

			EM::Vec3& GetPosition() { return mLight.GetPosition(); }
			EG::ColorRGBA16& GetColor() { return mLight.GetColor(); }
			float GetIntensity() { return mLight.GetIntensity(); }
			float GetRadius() { return mLight.GetRadius(); }
			float GetAttenuationRate() { return mLight.GetAttenuationRate(); }

			void SetAttenuationRate(float rate);
			void SetPosition(EM::Vec3& position);
			void SetColor(EG::ColorRGBA16& color);
			void SetIntensity(float intensity);
			void SetRadius(float radius);

		private:
			Enjon::Graphics::PointLight mLight;	
	};
}

#endif