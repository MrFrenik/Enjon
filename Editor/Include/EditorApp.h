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
		void PlayOptions( );
		void CameraOptions( bool* enable );

	private:
		bool mViewBool = true;
		bool mShowCameraOptions = true;
		bool mShowLoadResourceOption = true;
		Enjon::String mResourceFilePathToLoad = "";
		bool mMoveCamera = false; 

		bool mPlaying = false;
		bool mNeedsStartup = true; 
		bool mNeedsShutdown = false;

		Enjon::f32 mCameraSpeed = 10.0f;
		Enjon::Transform mPreviousCameraTransform;
};