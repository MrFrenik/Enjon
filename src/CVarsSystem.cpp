#include "CVarsSystem.h"

#include <iostream>

namespace Enjon
{
	std::unordered_map<std::string, CVarBase*> CVarsSystem::mMap;

	template <typename T>
	bool CVarsSystem::Register(std::string key, T* val, CVarType type)
	{
		auto search = mMap.find(key);
		if (search == mMap.end())
		{
			mMap[key] = new CVar<T>(val, type);
			return true;
		}
		return false;
	}

	template <typename T>
	bool CVarsSystem::Set(std::string key, T val)
	{
		auto search = mMap.find(key);
		if (search != mMap.end())
		{
			auto base = search->second;
			switch(base->mType)
			{
				case TYPE_FLOAT: 
					*(static_cast<CVar<float>*>(base)->value) = val;
					return true;
					break;
				case TYPE_INT: 
					*(static_cast<CVar<int>*>(base)->value) = val;
					return true;
					break;
				case TYPE_BOOL: 
					*(static_cast<CVar<bool>*>(base)->value) = val;
					return true;
					break;
				case TYPE_UINT: 
					if (val < 0) break;
					*(static_cast<CVar<unsigned int>*>(base)->value) = val;
					return true;
					break;
				default:
					*(static_cast<CVar<float>*>(base)->value) = val;
					break;
			}
		}
		return false;
	}

	std::vector<std::string> CVarsSystem::GetRegisteredCommands()
	{
		std::vector<std::string> registeredCommands;

		for (auto it = mMap.begin(); it != mMap.end(); it++)
		{
			registeredCommands.push_back(it->first);
		}

		return registeredCommands;
	}

	

	template bool CVarsSystem::Register<float>(std::string key, float* val, CVarType type);
	template bool CVarsSystem::Register<double>(std::string key, double* val, CVarType type);
	template bool CVarsSystem::Register<int>(std::string key, int* val, CVarType type);
	template bool CVarsSystem::Register<bool>(std::string key, bool* val, CVarType type);
	template bool CVarsSystem::Register<unsigned int>(std::string key, unsigned int* val, CVarType type);

	template bool CVarsSystem::Set<float>(std::string key, float val);
	template bool CVarsSystem::Set<double>(std::string key, double val);
	template bool CVarsSystem::Set<int>(std::string key, int val);
	template bool CVarsSystem::Set<bool>(std::string key, bool val);
	template bool CVarsSystem::Set<unsigned int>(std::string key, unsigned int val);
}
