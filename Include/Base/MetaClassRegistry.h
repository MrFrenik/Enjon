// @file MetaClassRegistry.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.
#pragma once
#ifndef ENJON_META_CLASS_REGISTRY_H
#define ENJON_META_CLASS_REGISTRY_H 

#include "System/Types.h"
#include "Defines.h" 
#include "Base/MetaClass.h"
#include "Graphics/Texture.h"
#include "Serialize/ByteBuffer.h"
#include "Asset/AssetManager.h"
#include "Engine.h"

#include <unordered_map> 

namespace Enjon
{
	const u32 kConstantValue = 5;

	ENJON_ENUM( )
	enum TestEnum
	{
		Bob,
		Bill, 
		Frank,
		Jill,
		John,
		Mary
	}; 

	namespace TestNamespace
	{
		ENJON_CLASS( Namespace = [ TestNamespace ], Construct )
		class PointLight : public Enjon::Object
		{
			ENJON_CLASS_BODY( ) 

		public:

			ENJON_PROPERTY( Editable )
			f32 mFloatValue;

			ENJON_PROPERTY( Editable )
			u32 mUintValue; 

			ENJON_PROPERTY( )
			s32 mIntValue;

			ENJON_PROPERTY( Editable )
			AssetHandle< Texture > mTexture;

			ENJON_PROPERTY( )
			UUID mID;

			ENJON_PROPERTY( )
			String mName;

			ENJON_PROPERTY( Editable )
			Enjon::Vector< u32 > mDynamicArray;

			ENJON_PROPERTY( )
			f32 mStaticArrayConstant[32];

			ENJON_PROPERTY( )
			Enjon::AssetHandle<Enjon::Texture> mStaticArrayEnumIntegral[(usize)TextureFileExtension::TGA]; 

			ENJON_PROPERTY( )
			f32 mStaticArrayConstVariable[kConstantValue];

			ENJON_PROPERTY( )
			TestEnum mEnum;

			ENJON_PROPERTY( )
			TestEnum mAnotherEnumProp;
		}; 
	} 

	class EnjonObjectSerializer
	{
		public:
			EnjonObjectSerializer( ) = default;
			~EnjonObjectSerializer( ) = default;

#define WRITE_VAL( valType )\
	writeBuffer.Write< valType >( *cls->GetValueAs< valType >( object, prop) );

			void Serialize( ByteBuffer& writeBuffer, Object* object )
			{
				MetaClass* cls = const_cast< MetaClass* >( object->Class( ) );
				const PropertyTable& pt = cls->GetProperties( );

				// Iterate through properties and write to buffer
				for ( auto& prop : pt ) 
				{ 
					switch ( prop->GetType( ) )
					{
						case MetaPropertyType::U32:
						{ 
							WRITE_VAL( u32 )
						} break;

						case MetaPropertyType::F32:
						{ 
							WRITE_VAL( f32 )
						} break; 

						case MetaPropertyType::S32:
						{
							WRITE_VAL( s32 )
						} break;

						case MetaPropertyType::String:
						{
							WRITE_VAL( String )
						} break;

						case MetaPropertyType::AssetHandle:
						{
							// Get value of asset 
							AssetHandle<Asset> val; 
							cls->GetValue( object, prop, &val );
							if ( val )
							{
								writeBuffer.Write( val.GetUUID() );
							} 

						} break; 

						case MetaPropertyType::UUID:
						{
							WRITE_VAL( UUID )
						} break;
					}
				}
			}

# define READ_SET_VAL(valType)\
	valType val = readBuffer.Read< valType >();\
	cls->SetValue(object, prop, val);

			void Deserialize( ByteBuffer& readBuffer, Object* object )
			{
				MetaClass* cls = const_cast< MetaClass* > ( object->Class( ) );
				const PropertyTable& pt = cls->GetProperties( ); 

				// Iterate through properties and get from read buffer
				for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
				{
					// Grab property at index from metaclass
					MetaProperty* prop = const_cast<MetaProperty*> ( cls->GetProperty( i ) );

					switch ( prop->GetType( ) )
					{
						case MetaPropertyType::U32:
						{
							// Set value of object from read buffer
							READ_SET_VAL( u32 )
						} break;
						
						case MetaPropertyType::F32:
						{
							// Set value of object from read buffer
							READ_SET_VAL( f32 )
						} break;

						case MetaPropertyType::String:
						{
							READ_SET_VAL( String )
						} break;

						case MetaPropertyType::S32:
						{
							READ_SET_VAL( s32 )
						} break;

						case MetaPropertyType::UUID:
						{
							READ_SET_VAL( UUID )
						} break;
 
						case MetaPropertyType::AssetHandle:
						{
							// Grab asset manager
							MetaPropertyTemplateBase* base = static_cast< MetaPropertyTemplateBase* >( prop );
							AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );
							AssetHandle<Asset> val;
							cls->GetValue( object, prop, &val ); 

							// Get meta class of the asset
							MetaClass* assetCls = const_cast< MetaClass* >( base->GetClassOfTemplatedArgument( ) );

							// Get uuid from read buffer
							UUID id = readBuffer.Read< UUID >( );

							// Get asset
							const Asset* asset = am->GetAsset( assetCls, id ); 

							// If valid asset
							if ( asset )
							{
								// Set asset handle to default asset
								val.Set( const_cast< Asset*>( asset ) ); 
							} 
							// Otherwise get default asset for this class type
							else
							{
								val.Set( am->GetDefaultAsset( assetCls ) );
							}

							// Set value of object
							cls->SetValue( object, prop, val );

						} break;
					}
				}
			}
	};
}

/*
ex. 

ENJON_ENUM( )
enum class TextureFileExtension : u32
{
	PNG,
	TGA,
	JPEG,
	BMP,
	HDR,
	UNKNOWN
};

// Now imagine a class which has this enum as a property...

ENJON_CLASS()
class Texture : public Asset
{
	ENJON_CLASS_BODY()
	...

	ENJON_PROPERTY()
	TextureFileExtension mExtesnsion;
} 

// When iterating through this object's properties, when a metapropertytype == Enum, need to evaluate the property in a special manner, just as 
// the templated properties need to be evaluated 

class MetaPropertyEnum : public MetaProperty
{
	// Need a way to iterate through elements of property
}

EvaluateProperties( const Object* object )
{ 
	const MetaClass* cls = object->Class();
	for ( usize i = 0; i < cls->GetPropertyCount(); ++i )
	{
		const MetaProperty* property = cls->GetProperty( i );
		switch ( property->GetType() )
		{
			...

			case Enum: 
			{
				// Is an enum type, so needs to be cast to enum property
				const MetaPropertyEnum* enumProp = static_cast<const MetaPropertyEnum*>( property );

				// Could do it this way...
				const MetaClassEnum* enumCls = enumProp->GetEnumClass();

				// Should hold what the internal integral value is, which will be either u32, s32, or char

				for ( auto& e : enumCls->GetElements() )
				{
					// Getting string value
					String str = e->Name();  // A).
					
					// Getting u32 value ( this works because it MUST BE an integral value )
					u32 val = e->ToUint();	// B).

					// Getting s32 value
					s32 val = e->ToInt();	// C).

					// Getting char value  ( not sure I like this so much...)
					char val = e->ToChar();	// D).

					// If selected, then set value of enum on object
					if ( ImGui::Selectable( str.c_str() ) )
					{
						cls->SetValue( object, prop, e->Value() );
					}
				}

				TextureFileExtension val;
				cls->GetValue( object, prop, &val );

				template < typename T >
				void GetValue( const Object* obj, const MetaProperty* prop, T* value ) const
				{
					if ( HasProperty( prop ) )
					{
						void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) );
						*value = *( T* )member_ptr;
					}
				} 

				// Example of enum element
				class EnumElement
				{
					public: 
						
					private:
						String mName; 
				} 

				// Need to have a way to be able to return the actual type of the enumeration? That way I can set its value on the object that holds it as a property
				cls->SetValue( object, prop, value ); => value MUST be the enumeration, or else that won't work

				// A).
				String EnumElement::Name()
				{ 
					return mName;
				}

				u32 EnumElement::ToUint()
				{
					return (u32)mValue;  // => mValue must be the actual enumeration type, so how do I show this? Perhaps EnumElement will be templated? 
				} 

				// Can't really set the enumeration type, can I? If I don't know how to cast it to the enumeration type itself...

				Show enum values in editor
				Select from drop down of possible enum values
				Set that value to object

				for ( usize j = 0; j < enumCls->GetPropertyCount(); ++j )
				{
					
				} 


				// But how to actually assign the property? What am I even trying to solve?

			} break;
		}
	} 
} 

// TextureFileExtension
template <>
MetaClass* Object::ConstructMetaClass< TextureFileExtension >( )
{
	MetaClass* cls = new MetaClass( ); 
}
	
	// Need to be able to set the value of the enum 
	// Need to be able to display the names of the enum as string values for editor display

*/ 

//template < typename T >
//void GetValue( const Object* obj, const MetaProperty* prop, T* value ) const
//{
//	if ( HasProperty( prop ) )
//	{
//		void* member_ptr = ( ( ( u8* )&( *obj ) + prop->mOffset ) );
//		*value = *( T* )member_ptr;
//	}
//} 

/*
	// Need to be able to get the size of a dynamic array
	// Need to be able to iterate through a static/dynamic array
	// Storing size to static array should be easy enough - need to be able to have consistent way to iterate over the array type however
	//	regardless of whether array is dynamic or static
	// Maybe treat this the same as I do for elements of an enum? Be able to iterate over array elements? 

	ArrayIterator iterator = arrayProp->GetBegin(object); 
	for ( ArrayIterator it = arrayProp->GetBegin(object); it != arrayProp->GetEnd(object); ++it )
	{
		// Do thing with it...					
	}

	// What does the iterator look like?

	template <typename T>
	class MetaPropertyArray : public MetaPropertyTemplateBase
	{
		public:

			ArrayIterator GetBegin( const Object* object )
			{ 
			}

			ArrayIterator GetEnd( const Object* object )
			{
			}

		private:
			T mClass;
			usize mSize;
			ArrayType mType;
	}

	template <typename T>
	class ArrayIterator : public std::iterator<std::random_access_iterator_tag, T, ptrdiff_t, T*, T&>
	{
				
	}

	// To iterate through flat array, very simply done: 

	Ex. 
	// Assume an array of some structure, defined as such: 
	struct TestArrayStruct
	{
		u32 mUintVal;	
		f32 mFloatVal;	
		f64 mDoubleVal;	
		s32 mIntVal;
		AssetHandle<Texture> mTexture;
		bool mBool;
	}; // Not padded, arbitrarily large structure

	// Now an array holding this structure
	const usize arraySize = 6;
	TestArrayStruct testArray[arraySize];
	
	// Great, now to iterate over this and to grab pointer to any member of the array will be as such:
	// Use the base point ( will work for either dynamic or flat sized arrays )
	for ( usize i = 0; i < arraySize; ++i )
	{
		void* memberPtr = (u8*)(&testArray[0] + i);
		TestArrayStruct* val = (TestArrayStruct*)memberPtr;
	}  

	struct ArrayPropertyProxy
	{
		const MetaProperty* mArrayTypeProperty = nullptr;
		u32 mIndex = 0;
	}

	case Enjon::MetaPropertyType::Array:
	{
		// Property is array prop, so need to convert it
		const MetaPropertyArrayBase* arrayProp = static_cast< const MetaPropertyArrayBase* > ( prop );
		if ( arrayProp )
		{
			if ( ImGui::TreeNode( arrayProp->GetName().c_str() )
			{
				for ( usize i = 0; i < arrayProp->GetSize(); ++i )
				{
					PropertyProxy proxy = base->GetArrayPropertyProxy( object, i );

					// What does the proxy give me the ability to do?  
					// Don't really like this, since it's just repetition of code...
					ImGuiManager::DebugDumpArrayProperty( object, proxy );
				}
			}
		}

	} break; 


	ImGuiManager::DebugDumpArrayProperty( const Object* object, const ArrayPropertyProxy& proxy )
	{
		MetaPropertyType propType = proxy.base->GetArrayType();

		switch ( propType )
		{
			case MetaPropertyType::U32:
			{
				u32 val = proxy.base->GetValue				
			} break;

			case MetaPropertyType::AssetHandle:
			{
				// The array property really needs to be able to hold a property for its type...
			} break;
		}
	}

*/ 

// Define ALL shared enum property element structures up here
// Enum property 
/*

struct Enjon_TestNamespace_TestEnum_Struct
{
	Enjon_TestNamespace_TestEnum_Struct()
	{
		mElements.push_back( MetaPropertyEnumElement( "Bob", 0 ) );
		mElements.push_back( MetaPropertyEnumElement( "Bill", 1 ) );
		mElements.push_back( MetaPropertyEnumElement( "Frank", 2 ) );
		mElements.push_back( MetaPropertyEnumElement( "Jill", 3 ) );
		mElements.push_back( MetaPropertyEnumElement( "John", 4 ) );
		mElements.push_back( MetaPropertyEnumElement( "Mary", 5 ) ); 
	}

	const Vector< MetaPropertyEnumElement >& Elements() const 
	{
		return mElements;
	}

	Vector< MetaPropertyEnumElement > mElements; 
};

// Then to use...  
// ex.
struct Enjon_TestNamespace_TestEnum_Struct enumStruct();
cls->mProperties[ 4 ] = new Enjon::MetaPropertyEnum( MetaPropertyType::Enum, "mAnotherEnumProp", ( u32 )&( ( TestNamespace::PointLight* )0 )->mAnotherEnumProp, 4, MetaPropertyTraits( false, 0.000000f, 0.000000f ), enumStruct.Elements(), "TestEnum" );
*/ 
 

#endif

























