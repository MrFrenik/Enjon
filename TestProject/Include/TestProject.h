#pragma once
#ifndef TEST_PROEJCT_H
#define TEST_PROEJCT_H

#include <Application.h>
#include <Entity/EntityManager.h>
#include <Asset/AssetManager.h>

#define ENJON_EXPORT __declspec(dllexport) 

extern "C"
{
	namespace Enjon
	{
		class ByteBuffer;
	} 

	namespace Enjon
	{
		ENJON_CLASS( )
		class TestProject : public Enjon::Application 
		{
			ENJON_CLASS_BODY( )

			public:
				TestProject( ) = default;
				~TestProject( ) = default;

				virtual Enjon::Result Initialize( ) override;

				virtual Enjon::Result Update( f32 dt ) override;

				virtual Enjon::Result ProcessInput( f32 dt ) override;

				virtual Enjon::Result Shutdown( ) override;

				virtual Enjon::Result SerializeData( Enjon::ByteBuffer* buffer ) const override;

				virtual Enjon::Result DeserializeData( Enjon::ByteBuffer* buffer ) override;

			private: 

				ENJON_PROPERTY( )
				Enjon::EntityHandle mFloor;

				ENJON_PROPERTY( )
				Enjon::EntityHandle mEntity;

				ENJON_PROPERTY( )
				Enjon::f32 mRotationTime = 0.0f;

				ENJON_PROPERTY( )
				Enjon::Vec3 mRotationAxis;

				ENJON_PROPERTY( )
				Enjon::Vec3 mAnotherVec3 = Vec3( 1.0f, 0.2f, 0.5f );

				ENJON_PROPERTY( )
				Enjon::f32 mRotationSpeed = 10.0f; 

				ENJON_PROPERTY( )
				Enjon::f32 mAnotherFloat = 1.0f; 

				Vector< EntityHandle > mHandles;
		};
	}

	ENJON_EXPORT Enjon::Application* CreateApplication( Enjon::Engine* engine );
	ENJON_EXPORT void DeleteApplication( Enjon::Application* app ); 
	ENJON_EXPORT void SetEngineInstance( Enjon::Engine* engine ); 
}

#endif