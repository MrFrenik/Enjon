#include <iostream>

#include <Defines.h>
#include <Utils/FileUtils.h>
#include <System/Containers.h>
#include <Engine.h>

#include "EditorApp.h" 
 
#include <filesystem> 
#include <chrono>

#define ENJON_MEMORY_LEAK_DETECTION 0
#if ENJON_MEMORY_LEAK_DETECTION
	#include <vld.h> 
#endif

namespace FS = std::experimental::filesystem; 

using namespace Enjon; 

#include <limits>

#define fw(what) std::forward<decltype(what)>(what)

/**
* @ class measure
* @ brief Class to measure the execution time of a callable
*/
template < typename TimeT = std::chrono::milliseconds, class ClockT = std::chrono::system_clock >
struct measure
{
	/**
	* @ fn    execution
	* @ brief Returns the quantity (count) of the elapsed time as TimeT units
	*/
	template<typename F, typename ...Args>
	static typename TimeT::rep execution( F&& func, Args&&... args )
	{
		auto start = ClockT::now( );

		fw( func )( std::forward<Args>( args )... );

		auto duration = std::chrono::duration_cast<TimeT>( ClockT::now( ) - start );

		return duration.count( );
	}

	/**
	* @ fn    duration
	* @ brief Returns the duration (in chrono's type system) of the elapsed time
	*/
	template<typename F, typename... Args>
	static TimeT duration( F&& func, Args&&... args )
	{
		auto start = ClockT::now( );

		fw( func )( std::forward<Args>( args )... );

		return std::chrono::duration_cast<TimeT>( ClockT::now( ) - start );
	}
};

const usize TEST_COUNT = 1;
#define TEST_MEASUREMENT 0
#define TEST_SIZE 0

int main(int argc, char** argv)
{ 
	// TODO(): Hate doing it this way... just generate a config file or something...
	Enjon::String curDir = FS::current_path( ).string( );
	Enjon::String enjonDir = curDir + "/../../"; 

	Enjon::EditorApp mApp;

	for ( u32 i = 0; i < argc; ++i )
	{
		std::cout << argv[ i ] << "\n";
	}

#if TEST_SIZE 

	SlotArray< Vec3 > mSlotArray; 
	SlotArray< Vec3 >::Handle handle;
	for ( usize i = 0; i < TEST_COUNT; ++i )
	{
		handle = mSlotArray.push_back( Vec3( 1.0f, 0.0f, 10.0f ) );
	} 
 
	auto& vec = *handle;

	vec.x = 0.0f;
	vec.y = -20.0f;
	vec.z = 1.34f; 

#endif

#if TEST_MEASUREMENT
	HashMap< u32, Vec3 > mHashMap;
	SlotArray< Vec3 > mslot_array; 
	Vector< ResourceHandle< Vec3 > > mHandles; 
	mHandles.reserve( TEST_COUNT );

	std::cout << "slot_array vs. HashMap:: Element Count:  " << TEST_COUNT << "\n"; 
	std::cout << "\n";

	// Insertion measurement
	auto c = measure<>::execution( [ & ] ( ) 
	{
		for ( usize i = 0; i < TEST_COUNT; ++i )
		{
			mHandles.push_back( mslot_array.insert( Vec3( i, i, i ) ) );
		} 
	} ); 

	// Insertion measurement
	auto f = measure<>::execution( [ & ] ( ) 
	{
		for ( usize i = 0; i < TEST_COUNT; ++i )
		{
			mHashMap[ i ] = Vec3( );
		} 
	} );

	std::cout << "Insertion: \n"; 
	std::cout << "slot_array: " << c << "\n"; 
	std::cout << "HashMap: " << f << "\n"; 
	std::cout << "\n"; 

	// Lookup measurement
	c = measure<>::execution( [ & ] ( )
	{
		for ( usize i = 0; i < TEST_COUNT; ++i )
		{
			auto v = *mHandles[ i ];
		}
	} ); 

	f = measure<>::execution( [ & ] ( ) 
	{
		for ( usize i = 0; i < TEST_COUNT; ++i )
		{
			auto v = mHashMap[ i ];
		} 
	} );

	std::cout << "Lookup: \n"; 
	std::cout << "slot_array: " << c << "\n"; 
	std::cout << "HashMap: " << f << "\n"; 
	std::cout << "\n"; 

	std::cout << "Sizes: \n"; 
	std::cout << "slot_array: " << sizeof( mslot_array ) << "\n";
	std::cout << "HashMap: " << sizeof( mHashMap ) << "\n";
	std::cout << "\n"; 

	// Removal measurement
	c = measure<>::execution( [ & ] ( )
	{
		for ( usize i = 0; i < TEST_COUNT; ++i )
		{
			mslot_array.erase( mHandles[ i ] );
		}
	} ); 

	f = measure<>::execution( [ & ] ( ) 
	{
		for ( usize i = 0; i < TEST_COUNT; ++i )
		{
			mHashMap.erase( i );
		} 
	} );

	std::cout << "Removal: \n"; 
	std::cout << "slot_array: " << c << "\n"; 
	std::cout << "HashMap: " << f << "\n"; 
	std::cout << "\n"; 
#endif 

	// Check for arguments being passed in
	if ( argc > 1 )
	{
		// Read in project file 
		String projFile = argv[ 1 ];
		
		// Read in lines of project file 
		String fileContents = Utils::read_file_sstream( argv[ 1 ] ); 

		// Grab enjon directory from contents
		Vector< String > contentSplit = Utils::SplitString( fileContents, "\n" );
		enjonDir = Utils::SplitString( fileContents, "\n" ).back( ); 

		String projectName = Enjon::Utils::SplitString( Enjon::Utils::SplitString( projFile, "\\" ).back(), "." ).front();
		mApp.SetProjectOnLoad( projectName );

		std::cout << projectName << "\n";
	}

	Enjon::Engine mEngine; 
	Enjon::EngineConfig mConfig; 

	// Set root path to engine
	mConfig.SetRootPath( enjonDir ); 
 
	// Startup engine
	Enjon::Result res = mEngine.StartUp( &mApp, mConfig ); 

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
