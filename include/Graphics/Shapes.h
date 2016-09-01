#ifndef ENJON_SHAPES_H
#define ENJON_SHAPES_H

#include "Math/Maths.h"
#include "Graphics/SpriteBatch.h"
#include "Defines.h"

namespace Enjon { namespace Graphics { namespace Shapes {


	inline void DrawSpline(EG::SpriteBatch* Batch, EM::Vec4& StartAndEndPoints, EM::Vec4& ControlPoints, float Thickness = 2.0f, Enjon::uint32 NumberOfPoints = 200, EG::ColorRGBA16& Color = EG::RGBA16_White())
	{
		auto Ax = StartAndEndPoints.x; 	auto Ay = StartAndEndPoints.y;
		auto Bx = ControlPoints.x; 		auto By = ControlPoints.y;
		auto Cx = ControlPoints.z;		auto Cy = ControlPoints.w;
		auto Dx = StartAndEndPoints.z; 	auto Dy = StartAndEndPoints.w;

		// Points on curve
		float X;
		float Y;

		// Previous Point
		float Px;
		float Py;

		// Variables
		auto a = 1.0f;
		auto b = 1.0f - a;
		float Pa = a;
		float Pb = b;

		// Get first previous point
		Px = Ax*Pa*Pa*Pa + Bx*3*Pa*Pa*Pb + Cx*3*Pa*Pb*Pb + Dx*Pb*Pb*Pb;
		Py = Ay*Pa*Pa*Pa + By*3*Pa*Pa*Pb + Cy*3*Pa*Pb*Pb + Dy*Pb*Pb*Pb;

		// Loop
		for (auto i = 1;  i < NumberOfPoints; i++)
		{
			// Get point on curve
			X = Ax*a*a*a + Bx*3*a*a*b + Cx*3*a*b*b + Dx*b*b*b;
			Y = Ay*a*a*a + By*3*a*a*b + Cy*3*a*b*b + Dy*b*b*b;

			// Get direction vector from Next to previous
			auto Difference = EM::Vec2(X, Y) - EM::Vec2(Px, Py);
			auto Length = Difference.Length();
			auto Direction = EM::Vec2::Normalize(Difference);

			// Get angle of direction vector
			auto R = EM::Vec2(1, 0);
			float Angle = acos(R.DotProduct(Direction)) * 180.0f / M_PI;
			if (Direction.y < 0.0f) Angle *= -1; 

			// Draw point
			Batch->Add(
						EM::Vec4(X, Y, Length + 1, Thickness),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../Assets/Textures/DefaultNoText.png").id,
						Color,
						0.0f,
						EM::ToRadians(Angle)
					);
			a -= 1.0f / static_cast<float>(NumberOfPoints);
			b = 1.0f - a;
			Px = X;
			Py = Y;
		}
	}

	inline void DrawLine(EG::SpriteBatch* Batch, EM::Vec4& StartAndEndPoints, float Thickness = 2.0f, EG::ColorRGBA16& Color = EG::RGBA16_White())
	{
		// Get direction vector from Next to previous
		auto Difference = EM::Vec2(StartAndEndPoints.z, StartAndEndPoints.w) - EM::Vec2(StartAndEndPoints.x, StartAndEndPoints.y);
		auto Length = Difference.Length();
		auto Direction = EM::Vec2::Normalize(Difference);

		// Get angle of direction vector
		auto R = EM::Vec2(1, 0);
		float Angle = acos(R.DotProduct(Direction)) * 180.0f / M_PI;
		if (Direction.y < 0.0f) Angle *= -1; 

		Batch->Add(
					EM::Vec4(StartAndEndPoints.x + Difference.x / 2.0f, StartAndEndPoints.y + Difference.y / 2.0f, Length + 1, Thickness),
					EM::Vec4(0, 0, 1, 1),
					EI::ResourceManager::GetTexture("../Assets/Textures/DefaultNoText.png").id,
					Color,
					0.0f,
					EM::ToRadians(Angle)
				);
	}

}}}


#endif