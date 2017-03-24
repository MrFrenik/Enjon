#pragma once
#ifndef ENJON_SCENE_NODE_H
#define ENJON_SCENE_NODE_H

#include "Graphics/Scene.h"
#include <vector>

namespace Enjon
{
	class EntityHandle;

	class SceneNode
	{
		friend Enjon::Scene;

		public:
			SceneNode();
			~SceneNode();

			SceneNode& AddChild(SceneNode* child);
			void DetachChild(SceneNode* child);	

		protected:
			void SetParent(SceneNode* parent);
			SceneNode* RemoveParent();

		private:
			SceneNode* mParent;
			std::vector<SceneNode*> mChildren;
			EntityHandle* mEntity;
	};
}

#endif