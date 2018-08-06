//#include "E:/Development/Enjon/Include/Asset/Asset.h"
//#include "E:/Development/Enjon/Include/Graphics/PointLight.h"
//#include "E:/Development/Enjon/Include/Base/Object.h"
//#include "E:/Development/Enjon/Include/Entity/Component.h"
//#include "E:/Development/Enjon/Include/Graphics/Material.h"
//#include "E:/Development/Enjon/Include/Entity/Components/PointLightComponent.h"
//#include "E:/Development/Enjon/Include/Entity/EntityManager.h"
//#include "E:/Development/Enjon/Include/Graphics/Mesh.h"
//#include "E:/Development/Enjon/Include/Graphics/SpotLight.h"
//#include "E:/Development/Enjon/Include/Graphics/Texture.h"
//
//using namespace Enjon;
//
//// Asset
//template <>
//MetaClass* Object::ConstructMetaClass< Asset >( )
//{
//	MetaClass* cls = new MetaClass( );
//
//	// Construct properties
//
//	cls->mProperties[ "mUUID" ] = Enjon::MetaProperty( MetaPropertyType::UUID, "mUUID", ( u32 )&( ( Asset* )0 )->mUUID );
//
//	return cls;
//}
//
//// PointLight
//template <>
//MetaClass* Object::ConstructMetaClass< PointLight >( )
//{
//	MetaClass* cls = new MetaClass( );
//
//	// Construct properties
//
//	cls->mProperties[ "mPosition" ] = Enjon::MetaProperty( MetaPropertyType::Vec3, "mPosition", ( u32 )&( ( PointLight* )0 )->mPosition );
//	cls->mProperties[ "mAttenuationRate" ] = Enjon::MetaProperty( MetaPropertyType::Float_32, "mAttenuationRate", ( u32 )&( ( PointLight* )0 )->mAttenuationRate );
//	cls->mProperties[ "mRadius" ] = Enjon::MetaProperty( MetaPropertyType::Float_32, "mRadius", ( u32 )&( ( PointLight* )0 )->mRadius );
//	cls->mProperties[ "mIntensity" ] = Enjon::MetaProperty( MetaPropertyType::Float_32, "mIntensity", ( u32 )&( ( PointLight* )0 )->mIntensity );
//	cls->mProperties[ "mColor" ] = Enjon::MetaProperty( MetaPropertyType::ColorRGBA32, "mColor", ( u32 )&( ( PointLight* )0 )->mColor );
//
//	return cls;
//}
//
//// Component
//template <>
//MetaClass* Object::ConstructMetaClass< Component >( )
//{
//	MetaClass* cls = new MetaClass( );
//
//	// Construct properties
//
//
//	return cls;
//}
//
//// Material
//template <>
//MetaClass* Object::ConstructMetaClass< Material >( )
//{
//	MetaClass* cls = new MetaClass( );
//
//	// Construct properties
//
//	cls->mProperties[ "mTwoSided" ] = Enjon::MetaProperty( MetaPropertyType::Bool, "mTwoSided", ( u32 )&( ( Material* )0 )->mTwoSided );
//
//	return cls;
//}
//
//// GraphicsComponent
//template <>
//MetaClass* Object::ConstructMetaClass< GraphicsComponent >( )
//{
//	MetaClass* cls = new MetaClass( );
//
//	// Construct properties
//
//
//	return cls;
//}
//
//// PointLightComponent
//template <>
//MetaClass* Object::ConstructMetaClass< PointLightComponent >( )
//{
//	MetaClass* cls = new MetaClass( );
//
//	// Construct properties
//
//
//	return cls;
//}
//
//// Entity
//template <>
//MetaClass* Object::ConstructMetaClass< Entity >( )
//{
//	MetaClass* cls = new MetaClass( );
//
//	// Construct properties
//
//
//	return cls;
//}
//
//// Mesh
//template <>
//MetaClass* Object::ConstructMetaClass< Mesh >( )
//{
//	MetaClass* cls = new MetaClass( );
//
//	// Construct properties
//
//
//	return cls;
//}
//
//// SpotLight
//template <>
//MetaClass* Object::ConstructMetaClass< SpotLight >( )
//{
//	MetaClass* cls = new MetaClass( );
//
//	// Construct properties
//
//	cls->mProperties[ "mPosition" ] = Enjon::MetaProperty( MetaPropertyType::Vec3, "mPosition", ( u32 )&( ( SpotLight* )0 )->mPosition );
//	cls->mProperties[ "mIntensity" ] = Enjon::MetaProperty( MetaPropertyType::Float_32, "mIntensity", ( u32 )&( ( SpotLight* )0 )->mIntensity );
//	cls->mProperties[ "mColor" ] = Enjon::MetaProperty( MetaPropertyType::ColorRGBA32, "mColor", ( u32 )&( ( SpotLight* )0 )->mColor );
//
//	return cls;
//}
//
//// Texture
//template <>
//MetaClass* Object::ConstructMetaClass< Texture >( )
//{
//	MetaClass* cls = new MetaClass( );
//
//	// Construct properties
//
//
//	return cls;
//}
//
