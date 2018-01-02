#pragma once
#ifndef ENJON_PROJECT_H
#define ENJON_PROJECT_H
 
namespace Enjon
{
	class Application;

	class Project
	{
		public:
			Project( ) = default;
			~Project( ) = default; 

			void SetApplication( Enjon::Application* app ); 
			Enjon::Application* GetApplication( );
			
		private:
			Enjon::Application* mApp = nullptr; 
	};
}

#endif

