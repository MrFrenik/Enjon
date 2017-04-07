#ifndef ENJON_PROPERTY_H
#define ENJON_PROPERTY_H

#include "Signal.h"

#include <iostream>

// A Property is a encpsulates a value and may inform
// you on any changes applied to this value.
namespace Enjon 
{ 
	template <typename T>
	class Property 
	{ 
		public:
			typedef T mValuetype;

			// Properties for built-in types are automatically
			// initialized to 0. See template spezialisations
			// at the bottom of this file
			Property()
				: mConnection(nullptr) , mConnectionId(-1) 
			{}

			Property(T const& val) 
				: mValue(val) , mConnection(nullptr) , mConnectionId(-1) 
			{}

			Property(T&& val) 
				: mValue(std::move(val)) , mConnection(nullptr) , mConnectionId(-1) 
			{}

			Property(Property<T> const& to_copy)
				: mValue(to_copy.mValue) , mConnection(nullptr) , mConnectionId(-1) 
			{}

			Property(Property<T>&& to_copy) 
				: mValue(std::move(to_copy.mValue)) , mConnection(nullptr) , mConnectionId(-1) 
			{}

			// returns a Signal which is fired when the internal value
			// will be changed. The old value is passed as parameter.
			virtual Signal<T> const& BeforeChange() const { return mBeforeChange; }

			// returns a Signal which is fired when the internal value
			// has been changed. The new value is passed as parameter.
			virtual Signal<T> const& OnChange() const { return mOnChange; }

			// sets the Property to a new value. before_change() and
			// on_change() will be emitted.
			virtual void Set(T const& value) 
			{
				if (value != mValue) 
				{
					mBeforeChange.Emit(mValue);
					mValue = value;
					mOnChange.Emit(mValue);
				}
			}

			// sets the Property to a new value. before_change() and
			// on_change() will not be emitted
			void SetWithNoEmit(T const& value) 
			{
				mValue = value;
			}

			// emits before_change() and on_change() even if the value
			// did not change
			void Touch() 
			{
				mBeforeChange.Emit(mValue);
				mOnChange.Emit(mValue);
			}

			// returns the internal value
			virtual T const& Get() const { return mValue; }

			// connects two Properties to each other. If the source's
			// value is changed, this' value will be changed as well
			virtual void ConnectFrom(Property<T> const& source) 
			{
				Disconnect();
				mConnection = &source;
				mConnectionId = source.OnChange().Connect( [this]( T const& value )
				{
					Set( value );
				} );

				Set(source.Get());
			}

				// if this Property is connected from another property,
				// it will e disconnected
			virtual void Disconnect() 
			{
				if (mConnection) 
				{
					mConnection->OnChange().Disconnect(mConnectionId);
					mConnectionId = -1;
					mConnection = nullptr;
				}
			}

			// if there are any Properties connected to this Property,
			// they won't be notified of any further changes
			virtual void DisconnectAuditors() 
			{
				mOnChange.DisconnectAll();
				mBeforeChange.DisconnectAll();
			}

			// assigns the value of another Property
			virtual Property<T>& operator=( Property<T> const& rhs ) 
			{
				Set( rhs.mValue );
				return *this;
			}

			// assigns a new value to this Property
			virtual Property<T>& operator=( T const& rhs ) 
			{
				Set(rhs);
				return *this;
			}

			// compares the values of two Properties
			bool operator==( Property<T> const& rhs ) const 
			{
				return Property<T>::Get() == rhs.Get();
			}

			bool operator!=( Property<T> const& rhs ) const 
			{
				return Property<T>::Get() != rhs.Get();
			}

			// compares the values of the Property to another value
			bool operator==(T const& rhs) const { return Property<T>::Get() == rhs; }
			bool operator!=(T const& rhs) const { return Property<T>::Get() != rhs; }

			// returns the value of this Property
			T const& operator()() const { return Property<T>::Get(); }

		private:

			Signal<T> mOnChange;
			Signal<T> mBeforeChange;

			Property<T> const* mConnection;
			s32 mConnectionId;
			T mValue;
	};

	// specialization for built-in default contructors
	template<> inline Property<double>::Property()
		: mConnection(nullptr), mConnectionId(-1), mValue(0.0) 
	{}

	template<> inline Property<float>::Property()
		: mConnection(nullptr), mConnectionId(-1), mValue(0.f) 
	{}

	template<> inline Property<short>::Property()
		: mConnection(nullptr), mConnectionId(-1), mValue(0) 
	{}

	template<> inline Property<int>::Property()
		: mConnection(nullptr), mConnectionId(-1), mValue(0) 
	{}

	template<> inline Property<char>::Property()
		: mConnection(nullptr), mConnectionId(-1), mValue(0) 
	{}

	template<> inline Property<unsigned>::Property()
		: mConnection(nullptr), mConnectionId(-1), mValue(0) 
	{}

	template<> inline Property<bool>::Property()
		: mConnection(nullptr), mConnectionId(-1), mValue(false) 
	{}

	// stream operators
	template<typename T>
	std::ostream& operator<<(std::ostream& out_stream, Property<T> const& val) 
	{
		out_stream << val.Get();
		return out_stream;
	}

	template<typename T>
	std::istream& operator>>(std::istream& in_stream, Property<T>& val) 
	{
		T tmp;
		in_stream >> tmp;
		val.Set( tmp );
		return in_stream;
	} 
}

#endif /* PROPERTY_H */
