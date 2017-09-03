#pragma once
#ifndef ENJON_POINT_LIGHT_COMPONENT_H
#define ENJON_POINT_LIGHT_COMPONENT_H

#include "Entity/Component.h"
#include "Graphics/PointLight.h"

namespace Enjon
{
	ENJON_CLASS( )
	class PointLightComponent : public Component
	{
		ENJON_CLASS_BODY( PointLightComponent )

		public:
			PointLightComponent();
			~PointLightComponent();

			virtual void Update(float dt) override;
			virtual void Destroy() override;

			PointLight* GetLight() { return &mLight; }

			Vec3& GetPosition() { return mLight.GetPosition(); }
			ColorRGBA16& GetColor() { return mLight.GetColor(); }
			float GetIntensity() { return mLight.GetIntensity(); }
			float GetRadius() { return mLight.GetRadius(); }
			float GetAttenuationRate() { return mLight.GetAttenuationRate(); }

			void SetAttenuationRate(float rate);
			void SetPosition(Vec3& position);
			void SetColor(ColorRGBA16& color);
			void SetIntensity(float intensity);
			void SetRadius(float radius);

		private:
			PointLight mLight;	
	};
}

#endif