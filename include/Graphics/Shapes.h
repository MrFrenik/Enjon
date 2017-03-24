#ifndef ENJON_SHAPES_H
#define ENJON_SHAPES_H

#include "Math/Maths.h"
#include "Graphics/SpriteBatch.h"
#include "Defines.h"

namespace Enjon {  namespace Shapes {


	inline void DrawSpline(Enjon::SpriteBatch* Batch, EM::Vec4& StartAndEndPoints, EM::Vec4& ControlPoints, float Thickness = 2.0f, u32 NumberOfPoints = 200, ColorRGBA16& Color = RGBA16_White())
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
			auto Difference = Enjon::Vec2(X, Y) - Enjon::Vec2(Px, Py);
			auto Length = Difference.Length();
			auto Direction = Enjon::Vec2::Normalize(Difference);

			// Get angle of direction vector
			auto R = Enjon::Vec2(1, 0);
			float Angle = acos(R.Dot(Direction)) * 180.0f / EM::PI;
			if (Direction.y < 0.0f) Angle *= -1; 

			// Draw point
			Batch->Add(
						EM::Vec4(X, Y, Length + 1, Thickness),
						EM::Vec4(0, 0, 1, 1),
						Enjon::ResourceManager::GetTexture("../Assets/Textures/DefaultNoText.png").id,
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

	inline void DrawLine(SpriteBatch* Batch, EM::Vec4& StartAndEndPoints, float Thickness = 2.0f, ColorRGBA16& Color = RGBA16_White(), float Depth = 0.0f)
	{
		// Get direction vector from Next to previous
		auto Difference = Enjon::Vec2(StartAndEndPoints.z, StartAndEndPoints.w) - Enjon::Vec2(StartAndEndPoints.x, StartAndEndPoints.y);
		auto Length = Difference.Length();
		auto Direction = Enjon::Vec2::Normalize(Difference);

		// Get angle of direction vector
		auto R = Enjon::Vec2(1, 0);
		float Angle = acos(R.Dot(Direction)) * 180.0f / EM::PI;
		if (Direction.y < 0.0f) Angle *= -1; 

		Batch->Add(
					EM::Vec4(StartAndEndPoints.x + Difference.x / 2.0f, StartAndEndPoints.y + Difference.y / 2.0f, Length + 1, Thickness),
					EM::Vec4(0, 0, 1, 1),
					Enjon::ResourceManager::GetTexture("../Assets/Textures/DefaultNoText.png").id,
					Color,
					Depth,
					EM::ToRadians(Angle)
				);
	}

	inline void DrawHollowCircle(SpriteBatch* Batch, Enjon::Vec2& Point, Enjon::Vec2& StartAndEndAngles, float Radius, float Thickness = 2.0f, Enjon::uint32 NumberOfPoints = 360, ColorRGBA16& Color = RGBA16_White(), float Depth = 0.0f)
	{
		for (auto i = StartAndEndAngles.x; i <= StartAndEndAngles.y; i += 0.5)
		{
			// Find line between point and next point
			auto X = Radius * cos(i) + Point.x;
			auto Y = Radius * sin(i) + Point.y;
			auto NextX = Radius * cos(i + 0.5f) + Point.x;
			auto NextY = Radius * sin(i + 0.5f) + Point.y;

			// Get direction vector from Next to previous
			// auto Difference = Enjon::Vec2(NextX, NextY) - Enjon::Vec2(X, X);
			auto Difference = Enjon::Vec2(X, Y) - Enjon::Vec2(Point.x, Point.y);
			auto Length = Difference.Length();
			auto Direction = Enjon::Vec2::Normalize(Difference);

			// Get angle of direction vector
			auto R = Enjon::Vec2(1, 0);
			float Angle = acos(R.Dot(Direction)) * 180.0f / EM::PI;
			if (Direction.y < 0.0f) Angle *= -1; 

			Batch->Add(
						EM::Vec4(Point.x, Point.y, Enjon::Vec2(Length, Thickness)),
						EM::Vec4(0, 0, 1, 1),
						Enjon::ResourceManager::GetTexture("../Assets/Textures/DefaultNoText.png").id,
						Color,
						Depth,
						EM::ToRadians(Angle)
					);
		}

	}

	inline void DrawArrow(SpriteBatch* Batch, Enjon::Vec2& Point, float Size, ColorRGBA16& Color, float Depth = 0.0f, float Angle = 0.0f)
	{
		std::vector<Enjon::Vec2> Points;

		Enjon::Vec2 LOC = Point - Enjon::Vec2(Size / 2.0f, 0.0f);
		Enjon::Vec2 ROC = Point + Enjon::Vec2(Size / 2.0f, 0.0f);
		Enjon::Vec2 TOC = Point + Enjon::Vec2(0.0f, Size / 2.0f);
		Points.push_back(LOC);
		Points.push_back(ROC);
		Points.push_back(TOC);
		Points.push_back(TOC);
		Batch->AddPolygon(
							Points, 
							EM::Vec4(0, 0, 1, 1), 
							Enjon::ResourceManager::GetTexture("../Assets/Textures/Default.png").id,
							Color,
							Depth		
						);	
	}

}}


#endif