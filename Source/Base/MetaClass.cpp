#include "Base/Object.h"

namespace Enjon
{
#define __MetaClassSetValueInternal( type, objA, propA, objB, propB )\
	*( type* )( usize( objA ) + propA->mOffset ) = *( type* )( usize( objB ) + propB->mOffset );	

	void MetaClass::SetValue( Object* objA, const MetaProperty* propA, const Object* objB, const MetaProperty* propB )
	{
		if ( propA->GetType() != MetaPropertyType::Object && ( propA->GetType( ) != propB->GetType( ) ) ) {
			return;
		}

		switch ( propA->GetType( ) )
		{
			case MetaPropertyType::Bool: __MetaClassSetValueInternal( b8, objA, propA, objB, propB ); break;
			case MetaPropertyType::S8: __MetaClassSetValueInternal( s8, objA, propA, objB, propB ); break;
			case MetaPropertyType::U8: __MetaClassSetValueInternal( u8, objA, propA, objB, propB ); break;
			case MetaPropertyType::S16: __MetaClassSetValueInternal( s16, objA, propA, objB, propB ); break;
			case MetaPropertyType::U16: __MetaClassSetValueInternal( u16, objA, propA, objB, propB ); break;
			case MetaPropertyType::S32: __MetaClassSetValueInternal( s32, objA, propA, objB, propB ); break;
			case MetaPropertyType::U32: __MetaClassSetValueInternal( u32, objA, propA, objB, propB ); break;
			case MetaPropertyType::S64: __MetaClassSetValueInternal( s64, objA, propA, objB, propB ); break;
			case MetaPropertyType::U64: __MetaClassSetValueInternal( u64, objA, propA, objB, propB ); break;
			case MetaPropertyType::F32: __MetaClassSetValueInternal( f32, objA, propA, objB, propB ); break;
			case MetaPropertyType::F64: __MetaClassSetValueInternal( f64, objA, propA, objB, propB ); break;
		}
	} 

	const MetaClass* MetaPropertyHashMapBase::GetValueMetaClass() const
	{
		return Object::GetClass( mValueMetaClassName );
	}
}
