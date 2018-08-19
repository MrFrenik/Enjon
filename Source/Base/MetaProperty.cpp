#include "Base/MetaProperty.h" 
#include "Base/MetaClass.h"
#include "Base/Object.h"
 
namespace Enjon
{ 
	std::string MetaProperty::GetName( ) const
	{
		return mName;
	}

	MetaPropertyType MetaProperty::GetType( ) const
	{
		return mType;
	} 

	bool MetaProperty::HasFlags( const MetaPropertyFlags& flags ) const
	{
		return ( ( mTraits.mFlags & flags ) == flags );
	}
			
	bool MetaProperty::IsEditable( ) const
	{
		return HasFlags( MetaPropertyFlags::ReadOnly );
	}

	bool MetaProperty::IsSerializeable( ) const
	{
		return ( mTraits.IsSerializeable( ) );
	}
			
	MetaPropertyTraits MetaProperty::GetTraits( ) const
	{
		return mTraits;
	}

	bool MetaPropertyTraits::HasFlags( const MetaPropertyFlags& flags ) const
	{
		return ( ( mFlags & flags ) == flags );
	}
			
	f32 MetaPropertyTraits::GetUIMax( ) const
	{
		return mUIMax;
	}
		
	f32 MetaPropertyTraits::GetUIMin( ) const
	{
		return mUIMin;
	}
		
	bool MetaPropertyTraits::UseSlider( ) const
	{
		return ( mUIMin != mUIMax );
	}

	bool MetaPropertyTraits::IsPointer( ) const
	{
		return HasFlags( MetaPropertyFlags::IsPointer );
	}

	bool MetaPropertyTraits::IsVisible( ) const
	{
		return !HasFlags( MetaPropertyFlags::HideInEditor );
	}
 
	bool MetaPropertyTraits::IsSerializeable( ) const 
	{
		return !HasFlags( MetaPropertyFlags::NonSerializable );
	}
}

