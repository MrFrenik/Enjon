#ifndef ENJON_CORE_TYPES_H
#define ENJON_CORE_TYPES_H

#include "System/Config.h"

#include <cstdint>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <initializer_list>
#include <type_traits>
#include <memory>

#include <cstddef>
#include <cstring>

#define GLOBAL			static // global variables
#define INTERNAL		static // internal linkage
#define LOCAL_PERSIST	static // local persisting variables

#if defined(_MSC_VER)
	#define _ALLOW_KEYWORD_MACROS
#endif

#if !defined(alignof)
	#define alignof(x) __alignof(x)
#endif

namespace Enjon {

	// Type Specification
	// Integer:			[us](8|16|32|64)
	// Floating Point:  f(16|32|64)
	// Integral Size:   [us]size
	// Bool:			b(8|32)		// NOTE(John): 16 bit bool is never used
	// Pointer Integer: u?integer

	// NOTE(John): All "common" platforms use the same size for char, short and int
	// We do not need to include stdint.h/cstdint because it is not always available

	using int8 = signed char;
	using s8 = signed char;
	using uint8 = unsigned char;
	using u8 = uint8;

	using int16 = signed short;
	using uint16 = unsigned short;
	using u16 = uint16;
	using s16 = int16;

	using int32 = signed int;
	using uint32 = unsigned int; 
	using u32l = unsigned long;
	using u32 = uint32;
	using s32 = int32;
	using i32 = int32;

	// Older versions of MSVC do not define long long as 64 bits
#if defined (_MSC_VER)
	using int64 = signed __int64;
	using uint64 = unsigned __int64;
#else
	using int64 = signed long long;
	using uint64 = unsigned long long;
#endif

	using u64 = uint64;
	using s64 = int64;

	// NOTE(John): (std::)size_t is not used because for this platform:
	// sizeof(size_t) == sizeof(usize) == sizeof(ssize)
	
#if defined(ENJON_64_BIT)
	using usize = uint64;
	using ssize = int64;
#elif defined (ENJON_32_BIT)
	using usize = uint32;
	using ssize = int32;
#else
	#error Unknown bit size
#endif

static_assert(sizeof(usize) == sizeof(size_t), 
			  "'usize' is not the same size as 'size_t'");
static_assert(sizeof(ssize) == sizeof(usize), 
			  "'ssize' is not the same size as 'usize'");

using uintptr = uintptr_t;
using intptr  = intptr_t;

using bool8 = int8;
using bool32 = int32;

using b32 = bool32;
using b8 = bool8;
using b1 = bool;

using f32 = float;
using f64 = double;

///////////////////////////////////////////////////////////////////////////////////

	inline int16 f32Tof16(f32 f)
	{
		int16 fint16;
		int32 fint32;
		memcpy(&fint32, &f, sizeof(f32));
		fint16 = ((fint32 & 0x7FFFFFFF) >> 13) - (0x38000000 >> 13);
		fint16 |= ((fint32 & 0x80000000) >> 16);

		return fint16;
	}

	inline f32 f16Tof32(int16 fint16)
	{
		int32 fint32 = ((fint16 & 0x8000) << 16);
		fint32 |=((fint16 & 0x7FFF) << 13) + 0x38000000;

		f32 fRet;
		memcpy(&fRet, &fint32, sizeof(f32));
		return fRet;
	}

	class f16
	{
	public:
		f16() = default;

		f16(f32 f)
		: m_value{f32Tof16(f)}
		{
		}

		f16(const f16& f) = default;

		f16(f16&& f) // NOTE(John): default for move doesn't work on MSVC 2013
		: m_value{std::move(f.m_value)}
		{
		}

		operator f32() { return f16Tof32(m_value); }
		operator f32() const { return f16Tof32(m_value); }

		f16& operator=(const f16& f) { m_value = (int16)f; return *this; }
		f16& operator+=(const f16& f)
		{
			*this = *this + f;
			return *this;
		}
		f16& operator-=(const f16& f)
		{
			*this = *this - f;
			return *this;
		}
		f16& operator*=(const f16& f)
		{
			*this = *this * f;
			return *this;
		}
		f16& operator/=(const f16& f)
		{
			*this = *this / f;
			return *this;
		}
		f16& operator-()
		{
			*this = f16{-static_cast<f32>(*this)};
			return *this;
		}

		friend f16 operator+(const f16& a, const f16& b);
		friend f16 operator-(const f16& a, const f16& b);
		friend f16 operator*(const f16& a, const f16& b);
		friend f16 operator/(const f16& a, const f16& b);

	protected:
		int16 m_value;
	};

	inline f16 operator+(const f16& a, const f16& b)
	{
		return {static_cast<f32>(a) + static_cast<f32>(b)};
	}
	inline f16 operator-(const f16& a, const f16& b)
	{
		return {static_cast<f32>(a) - static_cast<f32>(b)};
	}
	inline f16 operator*(const f16& a, const f16& b)
	{
		return {static_cast<f32>(a) * static_cast<f32>(b)};
	}
	inline f16 operator/(const f16& a, const f16& b)
	{
		return {static_cast<f32>(a) / static_cast<f32>(b)};
	}
} 

typedef Enjon::u32 u32;
typedef Enjon::f32 f32;
typedef Enjon::s32 s32;
typedef Enjon::f64 f64;
typedef Enjon::u64 u64;
typedef Enjon::s64 s64;
typedef Enjon::b32 b32;
typedef Enjon::u16 u16;
typedef Enjon::s16 s16;
typedef Enjon::b8  b8;
typedef Enjon::u8  u8;
typedef Enjon::s8  s8;
typedef Enjon::b1  b1; 
typedef Enjon::usize usize;


#endif