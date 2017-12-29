#include <Application.h>
#include <Entity/EntityManager.h>

class EnjonEditor : public Enjon::Application
{
	public:

		virtual Enjon::Result Initialize() override;  

		/**
		* @brief Main update tick for application.
		* @return Enjon::Result
		*/
		virtual Enjon::Result Update(f32 dt) override;

		/**
		* @brief Processes input from input class 
		* @return Enjon::Result
		*/
		virtual Enjon::Result ProcessInput(f32 dt) override;

		/**
		* @brief Shuts down application and cleans up any memory that was allocated.
		* @return Enjon::Result
		*/
		virtual Enjon::Result Shutdown() override; 

	private:
		void LoadResourceFromFile( );
		void WorldOutlinerView( );

	private:
		bool mViewBool = true;
		bool mShowCameraOptions = true;
		bool mShowLoadResourceOption = true;
		Enjon::String mResourceFilePathToLoad = "";
		bool mMoveCamera = false; 

		bool mEntitySwitch = true;

		Enjon::EntityHandle mEntity;
};