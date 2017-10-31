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
			
	bool MetaProperty::IsEditable( ) const
	{
		return mTraits.mIsEditable;
	}
			
	MetaPropertyTraits MetaProperty::GetTraits( ) const
	{
		return mTraits;
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
}

