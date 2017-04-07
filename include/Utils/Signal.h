#pragma once
#ifndef ENJON_SIGNAL_H
#define ENJON_SIGNAL_H

#include "System/Types.h"

#include <functional>
#include <unordered_map>

// A signal object may call multiple slots with the
// same signature. You can connect functions to the signal
// which will be called when the emit() method on the
// signal object is invoked. Any argument passed to emit()
// will be passed to the given functions.  

namespace Enjon
{ 
	class SignalBase
	{
		public:
			SignalBase( ) {}
			~SignalBase( ) {}

		private:
			virtual void Override( ) = 0;
	}; 

	template < typename... Args >
	class Signal
	{ 
		public:

			Signal() : 
			mCurrentId( 0 ) {}

			// copy creates new signal
			Signal( Signal const& other ) 
				: mCurrentId(0) 
			{}

			// connects a member function of a given object to this Signal
			template < typename F, typename... A >
			s32 ConnectMember( F&& f, A&& ... a ) const 
			{
				mSlots.insert( {++mCurrentId, std::bind( f, a... )} );
				return mCurrentId;
			}

			// connects a std::function to the signal. The returned
			// value can be used to disconnect the function again
			s32 Connect( std::function< void( Args... ) > const& slot ) const 
			{
				mSlots.insert( std::make_pair( ++mCurrentId, slot ) );
				return mCurrentId;
			}

			// disconnects a previously connected function
			void Disconnect( s32 id ) const 
			{
				mSlots.erase(id);
			}

			  // disconnects all previously connected functions
			void DisconnectAll() const 
			{
				mSlots.clear();
			}

			// calls all connected functions
			void Emit( Args... p ) 
			{
				for( auto it : mSlots ) 
				{
					it.second( p... );
				}
			}

			// assignment creates new Signal
			Signal& operator=( Signal const& other ) 
			{
				DisconnectAll();
			}

		private:
			mutable std::unordered_map< s32, std::function< void( Args... ) > > mSlots;
			mutable s32 mCurrentId; 
	}; 
}

#endif /* SIGNAL_H */