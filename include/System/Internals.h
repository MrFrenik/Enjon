#ifndef ENJON_INTERNALS_H
#define ENJON_INTERNALS_H

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "Math/Vec4.h"
#include "System/Types.h"
#include "Defines.h"

namespace Enjon { namespace Internals {

	enum InternalType
	{
		UINT, 
		INT, 
		FLOAT, 
		VEC2, 
		VEC3, 
		VEC4, 
		BOOL
	};

	template <typename T>
	struct Internal
	{
		Internal(InternalType _Type) : Type(_Type)
		{}
		T Value;
		InternalType Type;
	};

	// Default uint32 constructor
	template <> 
	struct Internal<Enjon::uint32> 
	{
		Internal()
		{
			Type = InternalType::UINT;
		}

		Enjon::uint32 Value;
		InternalType Type;
	};

	template <> 
	struct Internal<Enjon::f32> 
	{
		Internal()
		{
			Type = InternalType::FLOAT;
		}

		Enjon::f32 Value;
		InternalType Type;
	};

	template <> 
	struct Internal<Enjon::bool32> 
	{
		Internal()
		{
			Type = InternalType::BOOL;
		}

		Enjon::bool32 Value;
		InternalType Type;
	};

	template <> 
	struct Internal<EM::Vec2> 
	{
		Internal()
		{
			Type = InternalType::VEC2;
		}

		EM::Vec2 Value;
		InternalType Type;
	};

	template <>
	struct Internal<EM::Vec3> 
	{
		Internal()
		{
			Type = InternalType::VEC3;
		}

		EM::Vec3 Value;
		InternalType Type;
	};

	template <>
	struct Internal<EM::Vec4> 
	{
		Internal()
		{
			Type = InternalType::VEC4;
		}

		EM::Vec4 Value;
		InternalType Type;
	};

	// Default int32 constructor
	using EUint = Internal<Enjon::uint32>;

	// Default f32 constructor
	using EFloat = Internal<Enjon::f32>;

	// Default bool32 constructor
	using EBool = Internal<Enjon::bool32>;

	// Default vec2 constructor
	using EVec2 = Internal<EM::Vec2>;

	// Default vec3 constructor
	using EVec3 = Internal<EM::Vec3>;

	// Default vec4 constructor
	using EVec4 = Internal<EM::Vec4>;

}}


#endif