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
				
		protected:
			virtual Result SerializeData( ObjectArchiver* archiver ) const override
			{
				std::cout << "Yeah, buddy\n!";
				return Result::INCOMPLETE;
			}

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
			u32 mStaticArrayConstant[10];

			ENJON_PROPERTY( )
			Enjon::AssetHandle<Enjon::Texture> mStaticArrayEnumIntegral[(usize)TextureFileExtension::UNKNOWN]; 

			ENJON_PROPERTY( )
			f32 mStaticArrayConstVariable[kConstantValue];

			ENJON_PROPERTY( )
			TestEnum mEnum; 

			ENJON_PROPERTY( )
			TestEnum mAnotherEnumProp; 

			ENJON_PROPERTY( )
			Enjon::HashMap< String, f32 > mHashMap;
		}; 
	} 
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

// Used for class parsing
class MetaClassVersionInfo
{
	public: 
		MetaClassVersionInfo
	private:
}

// How do I want this to work? umm... not sure, and it's not entirely necessary to think about just this second... but it's important to implement this fairly soon after basic serialization/deserialization is working 

// Basic serialization process

I want to have a CacheRegistry.manifest file that contains all the cached assets by UUID and location on disk

At startup of the assetmanager, this file is loaded in and parsed
All the assets in the directory are then pre-loaded into a "shell state" and waiting to be fully loaded once called by the application

Asset loaders then have a wrapper class around the asset that contain needed information for it to be processed

ENJON_ENUM( )
enum class AssetState
{
	Unloaded,				// When there is no record of the asset at all
	Preloaded,				// Cache record is created
	Loaded					// Fully loaded into memory and ready to use
};

class AssetRecordInfo
{
	public:
		AssetRecordInfo( const Asset* asset )
			: mAsset( asset )
		{
		} 

		~AssetRecordInfo() = default;

	private:
		const Asset* mAsset = nullptr; 
		String mResourcePath = "";
		Signal<> mResourceReloadSignal;
		AssetState mAssetState = AssetState::Unloaded;
}; 



*/ 
 

#endif

























