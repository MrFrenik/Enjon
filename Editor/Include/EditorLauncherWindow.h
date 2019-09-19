#ifndef ENJON_EDITOR_LAUNCHER_WINDOW
#define ENJON_EDITOR_LAUNCHER_WINDOW

#include <Base/Object.h>
#include <Defines.h>
#include <System/Types.h> 
#include <Graphics/Window.h>

#include "Project.h"
 
namespace Enjon
{
	class EditorApp;

	enum class EditorLauncherScreenType
	{
		Main,
		ProjectCreation
	};

	ENJON_CLASS( Construct )
	class EditorLauncherWindow : public Window
	{
		ENJON_CLASS_BODY( EditorLauncherWindow )

		public:
			virtual void Init( const WindowParams& params ) override;

		protected:

		private:
			void ConstructGUI(); 
			void ProjectSelectionScreen();
			bool ProjectSelectionBox( const Project& p );
			void ProjectMenuScreen();
			void ProjectCreationScreen();
			void LoadProjectScreen();

		private: 
			EditorApp* mApp = nullptr;
			EditorLauncherScreenType mScreenType = EditorLauncherScreenType::Main;
			Project mNewProject;
			ToolChainDefinition* mToolChainDef = nullptr;
	};
}

#endif


