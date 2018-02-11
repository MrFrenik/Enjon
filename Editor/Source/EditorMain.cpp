#include <iostream>

#include <Engine.h>

#include "EditorApp.h"

#include <filesystem> 
#include <vld.h>

#include <Physics/RigidBody.h>
#include <Physics/BoxCollisionShape.h>

namespace FS = std::experimental::filesystem; 

//template < typename T > 
//const T* GetValueAs( const Object* object, const MetaProperty* prop ) const
//{
//	if ( HasProperty( prop ) )
//	{
//		const T* val = reinterpret_cast< const T* >( usize( object ) +  prop->mOffset );
//		return val;
//	}
//
//	return nullptr;
//} 

using namespace Enjon;

Enjon::CollisionShape* Enjon::RigidBody::*ptrShape = &Enjon::RigidBody::mShape; 

int main(int argc, char** argv)
{ 
	Enjon::EditorApp mApp;
	Enjon::Engine mEngine; 
	Enjon::EngineConfig mConfig; 

	// TODO(): Hate doing it this way... just generate a config file or something...
	Enjon::String curDir = FS::current_path( ).string( );
	Enjon::String enjonDir = curDir + "/../../"; 

	// Set root path to engine
	mConfig.SetRootPath( enjonDir ); 

	// Startup engine
	Enjon::Result res = mEngine.StartUp( &mApp, mConfig ); 

	// Don't know how to do this shit...
	//Enjon::RigidBody mBody( Enjon::CollisionShapeType::Box );
	//Enjon::CollisionShape* Enjon::RigidBody::*ptrShape = &Enjon::RigidBody::mShape;
	//Enjon::Object* val = mBody.*ptrShape;
	//auto cls = val->Class( );

	//Enjon::ImGuiManager::DebugDumpObject( val );

	// If successful startup, then run the engine
	if ( res == Enjon::Result::SUCCESS )
	{
		mEngine.Run( );
	} 
	else 
	{ 
		// Print error...
	}
	
	return 0;
}
