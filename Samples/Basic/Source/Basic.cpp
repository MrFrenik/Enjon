
// Project includes
#include "Basic.h"

using namespace Enjon;

// Module implementation definition
ENJON_MODULE_DEFINE( Basic )

//==================================================================

Result Basic::Initialize()
{
	return Result::SUCCESS;
}

//==================================================================

Result Basic::Update( f32 dt )
{
	return Result::PROCESS_RUNNING;
}

//==================================================================

Result Basic::Shutdown()
{
	return Result::SUCCESS;
}

//==================================================================

