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
		BOOL, 
		DOUBLE
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
			Value = 1;
		}

		Internal(const Enjon::uint32& _Value)
		{
			Type = InternalType::UINT;
			Value = _Value;
		}

		Enjon::uint32 Value;
		InternalType Type;
	};

	template <> 
	struct Internal<Enjon::int32> 
	{
		Internal()
		{
			Type = InternalType::UINT;
			Value = 1;
		}

		Internal(const Enjon::int32& _Value)
		{
			Type = InternalType::UINT;
			Value = _Value;
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
			Value = 1.0f;
		}

		Internal(const Enjon::f32& _Value)
		{
			Type = InternalType::FLOAT;
			Value = _Value;
		}

		Enjon::f32 Value;
		InternalType Type;
	};

	template <> 
	struct Internal<Enjon::f64> 
	{
		Internal()
		{
			Type = InternalType::DOUBLE;
			Value = 1.0f;
		}

		Internal(const Enjon::f64& _Value)
		{
			Type = InternalType::DOUBLE;
			Value = _Value;
		}

		Enjon::f64 Value;
		InternalType Type;
	};

	template <> 
	struct Internal<Enjon::bool8> 
	{
		Internal()
		{
			Type = InternalType::BOOL;
			Value = 1;
		}

		Internal(const Enjon::bool8& _Value)
		{
			Type = InternalType::BOOL;
			Value = _Value;
		}

		Enjon::bool8 Value;
		InternalType Type;
	};

	template <> 
	struct Internal<Enjon::Vec2> 
	{
		Internal()
		{
			Type = InternalType::VEC2;
			Value = Enjon::Vec2(1.0f);
		}

		Internal(const Enjon::Vec2& _Value)
		{
			Type = InternalType::VEC2;
			Value = _Value;
		}

		Enjon::Vec2 Value;
		InternalType Type;
	};

	template <>
	struct Internal<EM::Vec3> 
	{
		Internal()
		{
			Type = InternalType::VEC3;
			Value = EM::Vec3(1.0f, 1.0f, 1.0f);
		}

		Internal(const EM::Vec3& _Value)
		{
			Type = InternalType::VEC3;
			Value = _Value;
		}

		Internal(const Enjon::f32& _x, const Enjon::f32& _y, const Enjon::f32& _z)
		{
			Type = InternalType::VEC3;
			Value = EM::Vec3(_x, _y, _z);
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
			Value = EM::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		Internal(const EM::Vec4& _Value)
		{
			Type = InternalType::VEC4;
			Value = _Value;
		}

		EM::Vec4 Value;
		InternalType Type;
	};

	// Default int32 constructor
	using EUint = Internal<Enjon::uint32>;

	// Default int32 constructor
	using EInt = Internal<Enjon::int32>;

	// Default f32 constructor
	using EFloat = Internal<Enjon::f32>;

	// Default f32 constructor
	using EDouble = Internal<Enjon::f64>;

	// Default bool32 constructor
	using EBool = Internal<Enjon::bool8>;

	// Default vec2 constructor
	using EVec2 = Internal<Enjon::Vec2>;

	// Default vec3 constructor
	using EVec3 = Internal<EM::Vec3>;

	// Default vec4 constructor
	using EVec4 = Internal<EM::Vec4>;

}}


#endif