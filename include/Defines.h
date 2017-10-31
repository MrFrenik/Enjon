#pragma once
#ifndef ENJON_DEFINES_H
#define ENJON_DEFINES_H

#define INTROSPECTION(...)
#define ENJON_STRUCT(...)
#define ENJON_CLASS(...)
#define ENJON_FUNCTION(...)
#define ENJON_PROP(...)

#define ER 		Enjon::Random
#define EU 		Enjon::Utils
#define EA		Enjon::Animation
#define EP		Enjon::Physics
#define EGUI	Enjon::GUI 

#include <string>
#include <vector>
#include <unordered_map>

namespace Enjon
{
	enum class Result
	{
		SUCCESS,
		FAILURE,
		PROCESS_RUNNING,
		UNKNOWN_ERROR
	};
	
	using String = std::string; 

	template <typename T>
	using Vector = std::vector<T>;

	template <typename T, typename K>
	using HashMap = std::unordered_map<T, K>;
}

#endif
