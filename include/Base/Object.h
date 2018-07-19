// @file Object.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.
#pragma once
#ifndef ENJON_OBJECT_H
#define ENJON_OBJECT_H 

#include "MetaClass.h"
#include "System/Types.h"
#include "Base/ObjectDefines.h"
#include "Engine.h"
#include "Defines.h"

#include <limits>
#include <assert.h>
#include <functional>
#include <iterator>

// Forward Declarations
namespace Enjon
{ 
	class EntityArchiver;
	class AssetArchiver;
	class ObjectArchiver;
	class ByteBuffer;
}

// TODO(): Clean up this file!

namespace Enjon
{
	enum class MetaPropertyType
	{
		Object,
		Bool,
		ColorRGBA32,
		F32,
		F64,
		U8,
		U16,
		U32,
		U64,
		S8,
		S16,
		S32,
		S64,
		String,
		Array,
		HashMap,
		Vec2,
		Vec3,
		Vec4,
		Mat4x4,
		Quat,
		Enum,
		UUID,
		Transform,
		AssetHandle,
		EntityHandle,
		iVec3
	};

	enum MetaPropertyFlags : u32
	{
		None = 0x00,
		IsPointer = 0x01,
		IsDoublePointer = 0x02
	};

	inline MetaPropertyFlags operator|( MetaPropertyFlags a, MetaPropertyFlags b )
	{
		return static_cast< MetaPropertyFlags >( static_cast< u32 >( a ) | static_cast< u32 >( b ) );
	}

	inline MetaPropertyFlags operator&( MetaPropertyFlags a, MetaPropertyFlags b )
	{
		return static_cast< MetaPropertyFlags >( static_cast< u32 >( a ) & static_cast< u32 >( b ) );
	}

	inline MetaPropertyFlags operator^( MetaPropertyFlags a, MetaPropertyFlags b )
	{
		return static_cast< MetaPropertyFlags >( static_cast< u32 >( a ) ^ static_cast< u32 >( b ) );
	}

	inline void operator^=( MetaPropertyFlags& a, MetaPropertyFlags b )
	{
		a = a ^ b;
	}

	inline void operator|=( MetaPropertyFlags& a, MetaPropertyFlags b )
	{
		a = a | b;
	}

	inline void operator&=( MetaPropertyFlags& a, MetaPropertyFlags b )
	{
		a = a & b;
	} 

	// Forward Declarations
	class MetaFunction;
	class MetaProperty;
	class MetaClass;
	class Object;
	
	// Don't really like this, but, ya know... wha ya gon' do?
	struct MetaPropertyTraits
	{
		MetaPropertyTraits( u32 isEditable = false, f32 uiMin = 0.0f, f32 uiMax = 1.0f, u32 isPointer = false, bool isVisible = true )
			: mIsEditable( isEditable ), mUIMin( uiMin ), mUIMax( uiMax ), mIsPointer( isPointer ), mIsVisible( isVisible )
		{ 
		}

		~MetaPropertyTraits( )
		{ 
		}

		/*
		* @brief
		*/
		f32 GetUIMax( ) const;
		
		/*
		* @brief
		*/
		f32 GetUIMin( ) const; 

		/*
		* @brief
		*/
		bool UseSlider( ) const;

		/*
		* @brief
		*/
		bool IsPointer( ) const;

		/*
		* @brief
		*/
		bool IsVisible( ) const;

		bool mIsEditable = false;
		bool mIsPointer = false;
		bool mIsVisible = true;
		f32 mUIMin = 0.0f;
		f32 mUIMax = 1.0f;
	};

	class MetaProperty
	{
		friend MetaClass;
		friend Object;

		public:
			/*
			* @brief
			*/
			MetaProperty( ) = default; 

			/*
			* @brief
			*/
			MetaProperty( MetaPropertyType type, const std::string& name, u32 offset, u32 propIndex, MetaPropertyTraits traits, const Vector<MetaFunction*>& accessors = Vector<MetaFunction*>(), const Vector<MetaFunction*>& mutators = Vector<MetaFunction*>() )
				: mType( type ), mName( name ), mOffset( offset ), mIndex( propIndex ), mTraits( traits ), mAccessorCallbacks( accessors ), mMutatorCallbacks( mutators )
			{
			}

			/*
			* @brief
			*/
			virtual ~MetaProperty( ) = default;
		
			/*
			* @brief
			*/
			bool HasFlags( MetaPropertyFlags flags );
			
			/*
			* @brief
			*/
			bool IsEditable( ) const;

			/*
			* @brief
			*/
			std::string GetName( ) const;

			/*
			* @brief
			*/
			MetaPropertyType GetType( ) const;

			/*
			* @brief
			*/
			MetaPropertyTraits GetTraits( ) const; 
 
			/*
			* @brief
			*/
			template <typename T>
			const T* Cast( ) const
			{
				static_assert( std::is_base_of< MetaProperty, T >::value, "Must inherit from MetaProperty." );
				return static_cast< const T* >( this );
			}

			/*
			* @brief
			*/
			u32 GetOffset( ) const
			{
				return mOffset;
			}

			/*
			* @brief
			*/
			u32 GetIndex( ) const
			{
				return mIndex;
			}

		protected:
			MetaPropertyType mType;
			String mName;
			u32 mOffset;
			u32 mIndex;
			MetaPropertyTraits mTraits;
			Vector<MetaFunction*> mAccessorCallbacks;
			Vector<MetaFunction*> mMutatorCallbacks;
	};

	class MetaPropertyPointerBase : public MetaProperty 
	{
		friend MetaClass; 
		friend Object;

		public:
			MetaPropertyPointerBase( ) = default;
			~MetaPropertyPointerBase( ) = default; 

			virtual const Object* GetValueAsObject( const Object* obj ) const = 0;
	};

	template <typename BaseType, typename ValueType>
	class MetaPropertyPointer : public MetaPropertyPointerBase
	{
		public:
			MetaPropertyPointer() = default;
			MetaPropertyPointer( MetaPropertyType type, const std::string& name, u32 offset, u32 propIndex, MetaPropertyTraits traits, ValueType* BaseType::*memberPointer )
			{
				mType = type;
				mName = name;
				mOffset = offset;
				mIndex = propIndex;
				mTraits = traits;
				mMemberPointer = memberPointer;
			}
			~MetaPropertyPointer() = default;

			// Need to be able to access the type like this: 
			const ValueType* GetValue( const Object* obj )
			{
				return reinterpret_cast< const BaseType* >( obj )->*mMemberPointer;
			}

			virtual const Object* GetValueAsObject( const Object* obj ) const override
			{
				return reinterpret_cast< const BaseType* >( obj )->*mMemberPointer;
			}

		private:
			ValueType* BaseType::*mMemberPointer;
	};

	class MetaPropertyEnum;
	class MetaPropertyEnumElement
	{
		friend MetaClass;
		friend MetaPropertyEnum;
		friend Object;
		public:
			MetaPropertyEnumElement( ) = default;
			~MetaPropertyEnumElement( ) = default;

			MetaPropertyEnumElement( const String& identifier, s32 value )
				: mIdentifier( identifier ), mValue( value )
			{ 
			}

			/*
			* @brief
			*/
			String Identifier( ) const
			{
				return mIdentifier;
			}

			/*
			* @brief
			*/
			s32 Value( ) const
			{
				return mValue;
			}
 
		protected:
			String mIdentifier;
			s32 mValue;
	};

	class MetaPropertyEnum : public MetaProperty 
	{
		public:

			/*
			* @brief
			*/
			MetaPropertyEnum( ) = default;

			/*
			* @brief
			*/
			MetaPropertyEnum( MetaPropertyType type, const String& name, u32 offset, u32 propIndex, MetaPropertyTraits traits, const Vector< MetaPropertyEnumElement >& elements, const String& enumName )
				: mElements( elements ), mEnumTypeName( enumName )
			{
				mType = type;
				mName = name;
				mOffset = offset;
				mIndex = propIndex;
				mTraits = traits; 
			} 

			/*
			* @brief
			*/
			~MetaPropertyEnum( ) = default;

			/*
			* @brief
			*/
			const Vector< MetaPropertyEnumElement >& GetElements( ) const
			{
				return mElements;
			}

			String GetEnumName( ) const
			{
				return mEnumTypeName;
			}

		private:
			Vector< MetaPropertyEnumElement > mElements; 
			String mEnumTypeName;
	};

	class MetaPropertyTemplateBase : public MetaProperty
	{
		public:
			virtual const MetaClass* GetClassOfTemplatedArgument( ) const = 0;
	}; 

	class MetaPropertyArrayBase;
	struct MetaArrayPropertyProxy
	{
		MetaArrayPropertyProxy( ) = default;
		MetaArrayPropertyProxy( const MetaPropertyArrayBase* base, const MetaProperty* prop )
			: mBase( base ), mArrayPropertyTypeBase( prop )
		{ 
		} 

		const MetaPropertyArrayBase* mBase = nullptr;
		const MetaProperty* mArrayPropertyTypeBase = nullptr;
	};

	enum class ArraySizeType
	{
		Fixed,
		Dynamic
	};

	class MetaPropertyArrayBase : public MetaProperty
	{
		public: 
			virtual usize GetSize( const Object* object ) const = 0;
			virtual usize GetCapacity( const Object* object ) const = 0;
			virtual ArraySizeType GetArraySizeType( ) const = 0;
			virtual MetaPropertyType GetArrayType( ) const = 0; 
			virtual MetaArrayPropertyProxy GetProxy( ) const = 0;
			virtual void Resize( const Object* object, const usize& arraySize ) const = 0;
			virtual usize GetSizeInBytes( const Object* object ) const = 0;

			/*
			* @brief
			*/
			virtual ~MetaPropertyArrayBase( )
			{
				delete mArrayProperty;
				mArrayProperty = nullptr;
			}

		protected:
			MetaProperty* mArrayProperty = nullptr;
	}; 

	template <typename T>
	class MetaPropertyArray : public MetaPropertyArrayBase
	{
		public:

			/*
			* @brief
			*/
			MetaPropertyArray( MetaPropertyType type, const std::string& name, u32 offset, u32 propIndex, MetaPropertyTraits traits, ArraySizeType arraySizeType, MetaPropertyType arrayType, MetaProperty* arrayProp, usize arraySize = 0 )
				: mSize( arraySize ), mArraySizeType( arraySizeType ), mArrayType( arrayType )
			{ 
				// Default meta property member variables
				mType = type;
				mName = name;
				mOffset = offset;
				mIndex = propIndex;
				mTraits = traits; 
				mArrayProperty = arrayProp;
			}

			/*
			* @brief
			*/
			virtual ~MetaPropertyArray( )
			{
				delete mArrayProperty;
				mArrayProperty = nullptr;
			}

			/*
			* @brief
			*/
			virtual usize GetSize( const Object* object ) const override
			{
				switch ( mArraySizeType )
				{
					case ArraySizeType::Fixed:
					{
						return mSize; 
					} break;

					case ArraySizeType::Dynamic:
					{ 
						return ( ( Vector<T>* )( usize( object ) + mOffset ) )->size( ); 
					} break;
				}

				// Shouldn't get here
				return 0;
			}

			/*
			* @brief
			*/
			virtual usize GetSizeInBytes( const Object* object ) const override
			{
				// Get number of elements in array * size of templated argument type
				return GetSize( object ) * sizeof( T );
			}

			/*
			* @brief 
			*/
			virtual usize GetCapacity( const Object* object ) const override
			{
				switch ( mArraySizeType )
				{
					case ArraySizeType::Fixed:
					{
						return mSize;
					} break;

					case ArraySizeType::Dynamic:
					{
						return ( ( Vector< T >* )( usize( object ) + mOffset ) )->capacity( );
					} break;
				}

				// Shouldn't get here
				return 0;
			}

			/*
			* @brief 
			* @note Can ONLY reserve/resize space for dynamic arrays
			*/
			void Resize( const Object* object, const usize& arraySize ) const override 
			{
				if ( mArraySizeType == ArraySizeType::Dynamic )
				{
					( ( Vector<T>* )( usize( object ) + mOffset ) )->resize( (usize)arraySize ); 
				}
			} 

			/*
			* @brief
			*/
			void GetValueAt( const Object* object, usize index, T* out ) const
			{
				assert( index < GetCapacity( object ) );

				T* rawArr = GetRaw( object );
				*out = rawArr[index];
			} 

			T GetValueAs( const Object* object, usize index ) const
			{
				assert( index < GetCapacity( object ) ); 

				T* rawArr = GetRaw( object );
				return rawArr[ index ]; 
			}

			/*
			* @brief
			*/
			void SetValueAt( const Object* object, usize index, const T& value ) const
			{
				assert( index < GetCapacity( object ) );

				// Grab raw array
				T* rawArr = GetRaw( object );
				rawArr[index] = value;
			}
 
			/*
			* @brief
			*/
			ArraySizeType GetArraySizeType( ) const override
			{
				return mArraySizeType;
			} 

			/*
			* @brief
			*/
			MetaPropertyType GetArrayType( ) const
			{
				return mArrayType;
			}

			/*
			* @brief
			*/
			virtual MetaArrayPropertyProxy GetProxy( ) const override
			{
				return MetaArrayPropertyProxy( this, mArrayProperty ); 
			} 

		private: 

			/*
			* @brief
			*/
			T* GetRaw( const Object* object ) const
			{ 
				switch ( mArraySizeType )
				{
					case ArraySizeType::Dynamic:
					{ 
						return ( T* )( &( ( ( Vector< T >* )( usize( object ) + mOffset ) )->front() ) );
					} break;

					default:
					case ArraySizeType::Fixed:
					{
						return ( reinterpret_cast< T* >( usize( object ) + mOffset ) );
					} break;
				}
			}

		private:
			T mClass; 
			usize mSize;
			ArraySizeType mArraySizeType;
			MetaPropertyType mArrayType;
	};

	class MetaPropertyHashMapBase : public MetaProperty
	{
		public: 
			virtual usize GetSize( const Object* object ) const = 0; 

			MetaPropertyType GetKeyType( ) const
			{
				return mKeyType;
			}

			MetaPropertyType GetValueType( ) const 
			{
				return mValueType;
			} 

			virtual usize GetSizeInBytes( const Object* object ) const = 0;

			~MetaPropertyHashMapBase( )
			{ 
				delete mKeyProperty;
				delete mValueProperty;
				mKeyProperty = nullptr;
				mValueProperty = nullptr;
			} 

		protected:
			MetaProperty* mKeyProperty = nullptr;
			MetaProperty* mValueProperty = nullptr;
			MetaPropertyType mKeyType;
			MetaPropertyType mValueType;
	}; 

	template <typename K, typename V>
	class MetaPropertyHashMap : public MetaPropertyHashMapBase
	{
		public:

			/*
			* @brief
			*/
			MetaPropertyHashMap( MetaPropertyType type, const std::string& name, u32 offset, u32 propIndex, MetaPropertyTraits traits, MetaPropertyType keyType, MetaPropertyType valType, MetaProperty* keyProp, MetaProperty* valProp )
			{ 
				// Default meta property member variables
				mType = type;
				mName = name;
				mOffset = offset;
				mIndex = propIndex;
				mTraits = traits; 
				mKeyProperty = keyProp;
				mValueProperty = valProp;
				mKeyType = keyType;
				mValueType = valType;
			}

			/*
			* @brief
			*/
			~MetaPropertyHashMap( ) = default; 

			/*
			* @brief
			*/
			usize GetSize( const Object* object ) const 
			{
				return ( ( HashMap<K, V>* )( usize( object ) + mOffset ) )->size( ); 
			}

			/**
			* @brief
			*/
			usize GetSizeInBytes( const Object* object ) const override 
			{
				return ( usize )( GetSize( object ) * ( sizeof( K ) + sizeof( V ) ) );
			}

			/*
				Have to be able to iterate over the map - have to be able to list the keys as well as the values that are being iterated over 
			*/

			/*
			* @brief
			*/
			void GetValueAt( const Object* object, const K& key, V* out ) const
			{ 
				HashMap<K, V>* rawMap = GetRaw( object );
				*out = rawMap[key];
			} 

			/*
			* @brief
			*/
			V GetValueAs( const Object* object, const K& key ) const
			{ 
				HashMap<K, V>* rawMap = GetRaw( object );
				return rawMap[key];
			} 

			/*
			* @brief
			*/
			V GetValueAs( const Object* object, const typename HashMap< K, V >::iterator& iter ) const
			{ 
				return iter->second;
			} 

			/*
			* @brief
			*/
			typename HashMap< K, V >::iterator Begin( const Object* object ) const
			{
				HashMap< K, V >* rawMap = GetRaw( object );
				return rawMap->begin( );
			}

			/*
			* @brief
			*/
			typename HashMap< K, V >::iterator End( const Object* object ) const
			{
				HashMap< K, V >* rawMap = GetRaw( object );
				return rawMap->end( );
			} 

			/*
			* @brief
			*/
			void SetValueAt( const Object* object, const typename HashMap< K, V >::iterator& iter, const V& value ) const
			{ 
				iter->second = value;
			}

			/*
			* @brief
			*/
			void SetValueAt( const Object* object, const K& key, const V& value ) const
			{ 
				HashMap< K, V >* rawMap = GetRaw( object );
				rawMap->insert_or_assign( key, value );
			} 

			/*
			* @brief
			*/
			MetaPropertyType GetKeyType( ) const
			{
				return mKeyType;
			}

			/*
			* @brief
			*/
			MetaPropertyType GetValueType( ) const
			{
				return mValueType;
			} 

		private: 

			/*
			* @brief
			*/
			HashMap<K, V>* GetRaw( const Object* object ) const
			{ 
				return ( HashMap<K, V>* )( usize( object ) + mOffset );
			} 
	};

	template <typename T>
	class MetaPropertyAssetHandle : public MetaPropertyTemplateBase
	{ 
		public: 
			MetaPropertyAssetHandle( MetaPropertyType type, const std::string& name, u32 offset, u32 propIndex, MetaPropertyTraits traits )
			{
				Object::AssertIsObject<T>( );

				mType = type;
				mName = name;
				mOffset = offset;
				mIndex = propIndex;
				mTraits = traits;
			}

			~MetaPropertyAssetHandle( ) = default;

			virtual const MetaClass* GetClassOfTemplatedArgument() const override
			{
				return Object::GetClass<T>();	
			}

			T mClass;
	};

#define META_FUNCTION_IMPL( )\
	friend MetaClass;\
	friend Object;\
	virtual void Base( ) override\
	{\
	} 

	class MetaFunction
	{
		friend MetaClass;
		friend Object;

		public:
			MetaFunction( )
			{
			}

			~MetaFunction( )
			{ 
			}

			template < typename RetVal, typename T, typename... Args >
			RetVal Invoke( T* obj, Args&&... args )
			{
				Object::AssertIsObject<T>( );
				return static_cast< MetaFunctionImpl< T, RetVal, Args&&... >* >( this )->mFunc( obj, std::forward<Args>( args )... );
			}

			const Enjon::String& GetName( ) const
			{
				return mName;
			}

		protected:
			virtual void Base( ) = 0;

		protected:
			Enjon::String mName = "";
	};

	template < typename T, typename RetVal, typename... Args >
	struct MetaFunctionImpl : public MetaFunction
	{ 
		META_FUNCTION_IMPL( )

		MetaFunctionImpl( std::function< RetVal( T*, Args&&... ) > function, const Enjon::String& name )
			: mFunc( function )
		{ 
			mName = name;
		}

		~MetaFunctionImpl( )
		{ 
		}

		std::function< RetVal( T*, Args&&... ) > mFunc;
	};

	template < typename T >
	struct MetaFunctionImpl< T, void, void > : public MetaFunction
	{ 
		META_FUNCTION_IMPL( )
		
		MetaFunctionImpl( std::function< void( T* ) > function, const Enjon::String& name )
			: mFunc( func )
		{ 
			mName = name;
		}

		~MetaFunctionImpl( )
		{ 
		}
		std::function< void( T* ) > mFunc;
	};

	template < typename T, typename RetVal >
	struct MetaFunctionImpl< T, RetVal, void > : public MetaFunction
	{
		META_FUNCTION_IMPL( )

		MetaFunctionImpl( std::function< RetVal( T* ) > function, const Enjon::String& name )
			: mFunc( func )
		{ 
			mName = name;
		}

		std::function< RetVal( T* ) > mFunc;
	};

	typedef Vector< MetaProperty* > PropertyTable;
	typedef HashMap< Enjon::String, MetaFunction* > FunctionTable;
	typedef std::function< Object*( void ) > ConstructFunction;

	enum class MetaClassType
	{
		Object,
		Application,
		Component
	};

	class MetaClass
	{
		friend Object;

		public:

			/*
			* @brief
			*/
			MetaClass( ) = default;

			/*
			* @brief
			*/
			virtual ~MetaClass( )
			{
				// Delete all functions
				for ( auto& f : mFunctions )
				{
					delete f.second;
					f.second = nullptr;
				}


				for ( auto& p : mProperties )
				{
					delete p;
					p = nullptr;
				}

				// Clear properties and functions
				mProperties.clear( );
				mFunctions.clear( );

				// Any further destruction that needs to occur
				this->Destroy( ); 
			} 

			MetaClassType GetMetaClassType( ) const 
			{
				return mMetaClassType;
			}

			u32 PropertyCount( ) const
			{
				return ( u32 )mProperties.size( );
			}

			u32 FunctionCount( ) const
			{
				return ( u32 )mFunctions.size( );
			} 

			bool PropertyExists( const u32& index ) const
			{
				return index < mPropertyCount;
			}

			bool FunctionExists( const Enjon::String& name ) const
			{
				return ( mFunctions.find( name ) != mFunctions.end( ) ); 
			}

			const MetaFunction* GetFunction( const Enjon::String& name ) const
			{
				if ( FunctionExists( name ) )
				{
					return const_cast<MetaClass*>(this)->mFunctions[ name ];
				}

				return nullptr;
			}

			usize GetPropertyCount( ) const
			{
				return mPropertyCount;
			} 

			s32 FindPropertyIndexByName( const String& propertyName ) const
			{
				for ( usize i = 0; i < mPropertyCount; ++i )
				{
					if ( mProperties.at( i )->mName.compare( propertyName ) == 0 )
					{
						return i;
					}
				}

				return -1;
			}

			const MetaProperty* GetPropertyByName( const String& propertyName ) const
			{
				s32 index = FindPropertyIndexByName( propertyName );
				if ( index >= 0 && index < (s32)mPropertyCount )
				{
					return mProperties[ index ];
				}

				return nullptr;
			}

			const MetaProperty* GetProperty( const u32& index ) const
			{
				if ( PropertyExists( index ) )
				{
					return mProperties.at( index );
				}

				return nullptr;
			} 

			bool HasProperty( const MetaProperty* prop ) const
			{
				return ( ( prop->mIndex < mPropertyCount ) && ( mProperties.at( prop->mIndex ) == prop ) ); 
			}

			template < typename T >
			void GetValue( const Object* object, const MetaProperty* prop, T* out ) const
			{
				if ( HasProperty( prop ) )
				{
					T* val = reinterpret_cast< T* >( usize( object ) + prop->mOffset );
					*out = *val; 
				}
			} 

			template < typename T > 
			const T* GetValueAs( const Object* object, const MetaProperty* prop ) const
			{
				if ( HasProperty( prop ) )
				{
					if ( prop->GetTraits( ).IsPointer( ) )
					{
						return nullptr;
					}
					else
					{
						const T* val = reinterpret_cast< const T* >( usize( object ) +  prop->mOffset );
						return val; 
					}
				}

				return nullptr;
			} 

			template < typename T >
			void SetValue( const Object* object, const MetaProperty* prop, const T& value ) const
			{
				if ( HasProperty( prop ) )
				{
					T* dest = reinterpret_cast< T* >( usize( object ) + prop->mOffset );
					*dest = value;
					
					// Call mutator callbacks
					for ( auto& mutator : prop->mMutatorCallbacks )
					{
						if ( mutator )
						{
							mutator->Invoke<void>( object, value ); 
						}
					} 
				}
			} 

			template < typename T >
			void SetValue( Object* object, const MetaProperty* prop, const T& value )
			{
				if ( HasProperty( prop ) )
				{
					T* dest = reinterpret_cast< T* >( usize( object ) + prop->mOffset );
					*dest = value;

					// Call mutator callbacks
					for ( auto& mutator : prop->mMutatorCallbacks )
					{
						if ( mutator )
						{
							mutator->Invoke<void>( object, value ); 
						}
					} 
				}
			} 

			template < typename T >
			bool InstanceOf( ) const
			{
				MetaClassRegistry* mr = const_cast< MetaClassRegistry* >( Engine::GetInstance( )->GetMetaClassRegistry( ) );
				const MetaClass* cls = mr->Get< T >( );
				if ( !cls )
				{
					cls = mr->RegisterMetaClass< T >( );
				} 

				return ( cls && cls == this ); 
			} 

			// Method for getting type id from MetaClass instead of Object
			virtual u32 GetTypeId( ) const 
			{
				return mTypeId;
			}

			String GetName( ) const
			{
				return mName;
			}

			const PropertyTable& GetProperties( ) const { return mProperties; }

			/*
			* @brief
			*/
			Object* Construct( ) const
			{
				if ( mConstructor )
				{
					return mConstructor( );
				}
				return nullptr;
			}

		protected:

			virtual void Destroy( )
			{ 
			}

		protected:
			PropertyTable mProperties;
			FunctionTable mFunctions;
			u32 mPropertyCount;
			u32 mFunctionCount;
			u32 mTypeId;
			MetaClassType mMetaClassType = MetaClassType::Object;
			String mName;

			// Not sure if this is the best way to do this, but whatever...
			ConstructFunction mConstructor = nullptr;
	};

	class MetaClassComponent : public MetaClass
	{
		friend Object;

		public: 

			/*
			* @brief
			*/
			MetaClassComponent( ) = default;

			/*
			* @brief
			*/
			const Vector< String >& GetRequiredComponentList( ) const
			{
				return mRequiredComponentList;
			}


		protected:

			virtual void Destroy( ) override
			{
				mRequiredComponentList.clear( );
			}

		protected: 

			Vector< String > mRequiredComponentList;
	};

	class MetaClassRegistry
	{
		public:
			MetaClassRegistry( )
			{
			}

			/*
			* @brief
			*/
			~MetaClassRegistry( )
			{
				// Delete all meta classes from registry
				for ( auto& c : mRegistry )
				{
					delete c.second;
				}

				mRegistry.clear( );
			}

			template <typename T>
			MetaClass* RegisterMetaClass( )
			{
				// Must inherit from object to be able to registered
				static_assert( std::is_base_of<Object, T>::value, "MetaClass::RegisterMetaClass() - T must inherit from Object." );

				// Get id of object
				u32 id = GetTypeId< T >( );

				// If available, then return
				if ( HasMetaClass< T >( ) )
				{
					return mRegistry[ id ];
				}

				// Otherwise construct it and return
				MetaClass* cls = Object::ConstructMetaClass< T >( ); 

				mRegistry[ id ] = cls;
				mRegistryByClassName[cls->GetName()] = cls;

				// Further registration of metaclass
				RegisterMetaClassLate( cls );

				return cls;
			}
 
			/**
			* @brief
			*/
			void RegisterMetaClassLate( const MetaClass* cls );

			/**
			* @brief
			*/
			template <typename T>
			void UnregisterMetaClass( )
			{
				// Must inherit from object to be able to registered
				static_assert( std::is_base_of<Object, T>::value, "MetaClass::RegisterMetaClass() - T must inherit from Object." ); 
				UnregisterMetaClass( Object::GetClass< T >( ) ); 
			}

			/**
			* @brief
			*/
			void UnregisterMetaClass( const MetaClass* cls );

			/**
			* @brief
			*/
			template <typename T>
			u32 GetTypeId( ) const; 

			/**
			* @brief
			*/
			template < typename T >
			bool HasMetaClass( )
			{
				return ( mRegistry.find( GetTypeId< T >( ) ) != mRegistry.end( ) );
			} 

			/**
			* @brief
			*/
			bool HasMetaClass( const String& className )
			{
				return ( mRegistryByClassName.find( className ) != mRegistryByClassName.end( ) );
			}

			/**
			* @brief
			*/
			bool HasMetaClass( const u32& typeId )
			{
				return ( mRegistry.find( typeId ) != mRegistry.end( ) );
			}

			/**
			* @brief
			*/
			template < typename T >
			const MetaClass* Get( )
			{
				//return HasMetaClass< T >( ) ? mRegistry[ Object::GetTypeId< T >( ) ] : nullptr;
				return HasMetaClass< T >( ) ? mRegistry[ GetTypeId< T >( ) ] : nullptr;
			}

			/**
			* @brief
			*/
			const MetaClass* GetClassByName( const String& className )
			{
				if ( HasMetaClass( className ) )
				{
					return mRegistryByClassName[className];
				}

				return nullptr;
			}

			/**
			* @brief
			*/
			const MetaClass* GetClassById( const u32& typeId );

		private:
			HashMap< u32, MetaClass* > mRegistry; 
			HashMap< String, MetaClass* > mRegistryByClassName;
	};

	// Max allowed type id by engine
	const u32 EnjonMaxTypeId = std::numeric_limits<u32>::max( ) - 1;

	// Base model for all Enjon classes that participate in reflection
	class Object
	{
		friend ObjectArchiver;
		friend MetaClassRegistry;
		friend AssetArchiver;
		friend Engine;

		public:

			/**
			*@brief
			*/
			Object( ) 
			{
			}

			/**
			*@brief
			*/
			virtual ~Object( )
			{ 
			}

			/**
			*@brief
			*/
			virtual const MetaClass* Class( ) const
			{
				return nullptr;
			}

			/**
			*@brief
			*/
			template <typename T>
			inline static void AssertIsObject( )
			{
				static_assert( std::is_base_of<Object, T>::value, "T must inherit from Object." ); 
			} 

			/**
			*@brief
			*/
			template <typename T>
			const T* Cast( ) const
			{
				Object::AssertIsObject< T >( ); 
				return static_cast< T* >( const_cast< Object* > ( this ) );
			}

			/**
			*@brief
			*/
			template <typename T>
			T* ConstCast( ) const
			{
				Object::AssertIsObject< T >( );
				return static_cast< T* >( const_cast< Object* >( this ) );
			} 

			/**
			*@brief
			*/
			template <typename T>
			static void BindMetaClass( )
			{
				MetaClassRegistry* registry = const_cast< MetaClassRegistry* >( Engine::GetInstance( )->GetMetaClassRegistry( ) );
				registry->RegisterMetaClass< T >( );
			}

			/**
			*@brief
			*/
			template <typename T>
			static void UnbindMetaClass( )
			{
				MetaClassRegistry* registry = const_cast< MetaClassRegistry* >( Engine::GetInstance( )->GetMetaClassRegistry( ) ); 
				registry->UnregisterMetaClass( Object::GetClass< T >( ) );
			}

			/**
			*@brief
			*/
			static void UnbindMetaClass( const MetaClass* cls )
			{
				MetaClassRegistry* registry = const_cast< MetaClassRegistry* >( Engine::GetInstance( )->GetMetaClassRegistry( ) ); 
				registry->UnregisterMetaClass( cls ); 
			}

			/**
			*@brief
			*/
			u32 GetTypeId( )
			{
				if ( Class( ) )
				{
					return Class( )->GetTypeId( );
				}

				return std::numeric_limits<u32>::max( );
			}

			/**
			*@brief
			*/
			bool TypeValid( ) const; 

			template <typename T>
			static const MetaClass* GetClass( )
			{ 
				MetaClassRegistry* mr = const_cast< MetaClassRegistry* >( Engine::GetInstance()->GetMetaClassRegistry( ) );
				const MetaClass* cls = mr->Get< T >( );
				if ( !cls )
				{
					cls = mr->RegisterMetaClass< T >( );
				}
				return cls;
			}

			/**
			*@brief Could return null!
			*/
			static const MetaClass* GetClass( const u32& typeId );

			/**
			*@brief Could return null!
			*/
			static const MetaClass* GetClass( const String& className )
			{
				MetaClassRegistry* mr = const_cast<MetaClassRegistry*> ( Engine::GetInstance( )->GetMetaClassRegistry( ) );
				return mr->GetClassByName( className );
			} 

		protected:

			// Default to u32 max; If id set to this, then is not set by engine and invalid
			u32 mTypeId = std::numeric_limits<u32>::max();

		protected:
			template <typename T>
			static MetaClass* ConstructMetaClass( );

			static void BindMetaClasses( ); 

			template <typename T>
			static void RegisterMetaClass( )
			{
				MetaClassRegistry* mr = const_cast< MetaClassRegistry* >( Engine::GetInstance()->GetMetaClassRegistry( ) );
				mr->RegisterMetaClass< T >( );
			}

		public:
			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const;

			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ); 

			/*
			* @brief
			*/
			virtual Result DeserializeLateInit( );
 
			/*
			* @brief
			*/
			virtual Result OnEditorUI( );

		private:

			/**
			*@brief
			*/
			static u32 GetUniqueTypeId( ) noexcept
			{
				static u32 lastId { 1 };
				return lastId++;
			} 

		protected:
			virtual const Enjon::MetaClass* GetClassInternal( ) const
			{
				return nullptr;
			}

			virtual void ExplicitConstructor( )
			{ 
				// Do nothing by default
			}

			virtual void ExplicitDestructor( )
			{ 
				// Do nothing by default
			} 
	};
}

#endif

