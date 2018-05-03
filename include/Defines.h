#pragma once
#ifndef ENJON_DEFINES_H
#define ENJON_DEFINES_H

#define INTROSPECTION(...)
#define ENJON_STRUCT(...)
#define ENJON_CLASS(...)
#define ENJON_FUNCTION(...)
#define ENJON_PROP(...)

#define ENJON_EDITOR

#define ER 		Enjon::Random
#define EU 		Enjon::Utils
#define EA		Enjon::Animation
#define EP		Enjon::Physics
#define EGUI	Enjon::GUI 

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace Enjon
{
	enum class Result
	{
		SUCCESS,
		FAILURE,
		PROCESS_RUNNING,
		INCOMPLETE,
		UNKNOWN_ERROR
	};
	
	using String = std::string; 

	template <typename T>
	using Vector = std::vector<T>;

	template <typename T, typename K>
	using HashMap = std::unordered_map<T, K>;

	template <typename T>
	using HashSet = std::unordered_set<T>;

	template <typename T>
	using Queue = std::queue<T>;
}

#endif
