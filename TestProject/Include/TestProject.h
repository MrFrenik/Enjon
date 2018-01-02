#pragma once
#ifndef TEST_PROEJCT_H
#define TEST_PROEJCT_H

#include <Application.h>

namespace Enjon
{
	class ByteBuffer;
} 

ENJON_CLASS( Construct )
class TestProject : public Enjon::Application 
{
	ENJON_CLASS_BODY( )

	public:
		TestProject( ) = default;
		~TestProject( ) = default;

		virtual Enjon::Result Initialize( ) override;

		virtual Enjon::Result Update( Enjon::f32 dt ) override;

		virtual Enjon::Result ProcessInput( Enjon::f32 dt ) override;

		virtual Enjon::Result Shutdown( ) override; 

		virtual Enjon::Result SerializeData( Enjon::ByteBuffer* buffer ) const override;

		virtual Enjon::Result DeserializeData( Enjon::ByteBuffer* buffer ) override;

	private: 
}; 

// Definition for module export
ENJON_MODULE_DEFINE( TestProject ) 

#endif