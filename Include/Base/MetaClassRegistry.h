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



#endif