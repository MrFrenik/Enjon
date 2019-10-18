#ifndef ENJON_RENDER_PASS_H
#define ENJON_RENDER_PASS_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Defines.h"

namespace Enjon
{
	class GraphicsSubsystem;
	class GraphicsSubsystemContext;

	ENJON_CLASS( Abstract )
	class RenderPass : public Object
	{ 
		friend GraphicsSubsystem;

		ENJON_CLASS_BODY( RenderPass )

		protected: 
			virtual void Render() = 0;
	};
} 

#endif