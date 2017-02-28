#pragma once
#ifndef ENJON_APPLICATION_H
#define ENJON_APPLICATION_H

#include "System/Types.h"

namespace Enjon
{
	class Application
	{
		public:
			Application(){};
			~Application(){};

			virtual void Initialize() = 0;  
			virtual void Update(f32 dt) = 0;
			virtual void Shutdown() = 0;

		private:
	};
}

#endif