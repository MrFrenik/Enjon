#include "ImGui/ImGuiManager.h"

#include <algorithm>

namespace Enjon
{
	std::vector<std::function<void()>> ImGuiManager::mGuiFuncs;

	void ImGuiManager::Register(std::function<void()> func)
	{
		// TODO(): Search for function first before adding
		mGuiFuncs.push_back(func);
	}

	//---------------------------------------------------
	void ImGuiManager::Render()
	{
		for (auto& func : mGuiFuncs)
		{
			func();
		}
	}
}