#include <iostream>

#include <Defines.h>
#include <Utils/FileUtils.h>
#include <Engine.h>

#include "EditorApp.h"
 
#include <filesystem> 

#define ENJON_MEMORY_LEAK_DETECTION 0
#if ENJON_MEMORY_LEAK_DETECTION
	#include <vld.h> 
#endif

namespace FS = std::experimental::filesystem; 

using namespace Enjon; 

#include <limits>

const u32 INVALID_HANDLE = std::numeric_limits< u32 >::max( ); 

template <typename T>
class HandleMap;

template <typename T>
class Handle
{
	public: 
		friend HandleMap< T >;

		Handle( ) = default; 

		inline T* operator ->( ) 
		{
			return const_cast< HandleMap< T >* >( mHandleMap )->Lookup( *this );
		}

		inline T* operator *( ) 
		{
			return const_cast< HandleMap< T >* >( mHandleMap )->Lookup( *this );
		} 

	protected:
		Handle( const u32& handleID, const HandleMap< T >* handleMap )
			: mHandleID( handleID ), mHandleMap( handleMap )
		{ 
		}

	private:
		u32 mHandleID = INVALID_HANDLE;
		const HandleMap< T >* mHandleMap = nullptr; 
};

template <typename T>
class HandleMap
{
	public:

		inline Handle< T > Allocate( )
		{
			// Here's the tricky part. Want to push back a new index? Do I store a free list of indices? Do I iterate to find a free index? ( ideally would not do that last bit )	
			// Free list stack? So you push a free index onto the stack and then pop off to get the newest available index? 
			u32 freeIdx = GetNextAvailableIndex( );

			// Allocate new resource					
			mData.push_back( T() );

			// Push back new indirection index
			mReverseIndirectionIndices.push_back( freeIdx );

			// This gets the available index in the indirection list of indices, not in the actual resource array ( the reason being that the resource array can have its contents shifted around when adding / removing items )
			// If the index is the last item in the list, then push that on to grow the array
			mHandleIndices[ freeIdx ] = mData.size( ) - 1;

			return { freeIdx, this };
		}

		inline void Deallocate( const Handle< T >& res )
		{
			if ( res.mHandleID >= mHandleIndices.size( ) )
			{
				std::cout << "Warning: Invalid handle out of range: " << res.mHandleID << "\n";
				return; 
			}

			// Need to grab the actual resource index from the handle's indirection index
			u32 idx = mHandleIndices[ res.mHandleID ];

			if ( idx == INVALID_HANDLE )
			{
				std::cout << "Warning: Invalid handle operation: " << res.mHandleID << "\n";
				return; 
			} 

			// Set handle indirection index to invalid
			mHandleIndices[ res.mHandleID ] = INVALID_HANDLE; 

			// Need to pop and swap data			
			if ( mData.size( ) > 1 )
			{
				std::iter_swap( mData.begin( ) + idx, mData.end( ) - 1 );
				std::iter_swap( mReverseIndirectionIndices.begin( ) + idx, mReverseIndirectionIndices.end( ) - 1 );
			}

			// Update swapped indirection index
			if ( !mData.empty( ) )
			{
				mHandleIndices[ mReverseIndirectionIndices[ idx ] ] = idx;

				// Pop data and reverse indices after use
				mData.pop_back( );
				mReverseIndirectionIndices.pop_back( );
			} 

			// Push onto free list for handle
			mIndexFreeList.push( res.mHandleID ); 
		}

		inline T* Lookup( const Handle< T >& res )
		{
			// Something like this? Allocating a new resource would look for the next available index? Or would just push a new index onto stack?
			return &mData[ mHandleIndices[ res.mHandleID ] ];
		}

	private:

		u32 GetNextAvailableIndex( )
		{
			// If stack is empty, then simply return the size of the index array, which will be the back index after growing
			if ( mIndexFreeList.empty( ) )
			{
				// Grow the array by 1
				mHandleIndices.push_back( 0 );
				// Return the last index
				return mHandleIndices.size( ) - 1;
			}

			// Otherwise pop off stack and then return index
			u32 idx = mIndexFreeList.front( );
			mIndexFreeList.pop( );
			return idx;
		}

	private:
		Vector< T >		mData;							// Use the index from the handle vector to get index into this array of actual resources
		Vector< u32 >	mHandleIndices;					// Indices into this vector are returned to the user as handles
		Vector< u32 >	mReverseIndirectionIndices;		// Indices into this vector give reverse indirection into handle indices from objects
		Queue< u32 >	mIndexFreeList;					// Free list of most available indices
};

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

	HandleMap< Vec3 > mHandleMap; 
	Vector< Handle< Vec3 > > mHandles;

	for ( usize i = 0; i < 10; ++i  )
	{
		Handle< Vec3 > handle = mHandleMap.Allocate( );
		Vec3* val = *handle;
		val->x = (f32)( i * 20 ); 
		val->y = (f32)( i * 10 ); 
		val->z = (f32)( i * 5 ); 
		mHandles.push_back( handle );
	} 

	Handle< Vec3 >* h_2 = &mHandles[ 2 ];

	// Remove handle 1
	mHandleMap.Deallocate( mHandles[ 1 ] );
	mHandleMap.Deallocate( mHandles[ 5 ] ); 

	Handle< Vec3 >& h0 = mHandles[ 0 ];
	Handle< Vec3 >& h2 = mHandles[ 2 ];
	Handle< Vec3 >& h3 = mHandles[ 3 ];
 
	std::cout << h0->x << ", " << h0->y << ", " << h0->z << "\n";
	std::cout << h2->x << ", " << h2->y << ", " << h2->z << "\n";
	std::cout << h3->x << ", " << h3->y << ", " << h3->z << "\n";

	Handle< Vec3 > h = mHandleMap.Allocate( );
	h->x = 100.0f;
	h->y = 20.0f;
	h->z = 40.0;

	for ( usize i = 0; i < 50; ++i )
	{
		mHandleMap.Allocate( );
	}

	std::cout << h0->x << ", " << h0->y << ", " << h0->z << "\n";
	std::cout << h2->x << ", " << h2->y << ", " << h2->z << "\n";
	std::cout << h3->x << ", " << h3->y << ", " << h3->z << "\n";
	std::cout << h->x << ", " << h->y << ", " << h->z << "\n";

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
