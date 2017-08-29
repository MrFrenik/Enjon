#include "Base/MetaProperty.h"

#include "Base/MetaClass.h"
#include "Base/Object.h"
 
namespace Enjon
{
	std::string MetaProperty::GetName( )
	{
		return mName;
	}

	MetaPropertyType MetaProperty::GetType( )
	{
		return mType;
	} 
}

