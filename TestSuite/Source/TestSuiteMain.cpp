// @file TestSuiteMain.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include <Enjon.h>
 
#include <filesystem> 
#include <iostream>

#define ENJON_MEMORY_LEAK_DETECTION 1
#if ENJON_MEMORY_LEAK_DETECTION
	#include <vld.h> 
#endif

namespace FS = std::experimental::filesystem; 

using namespace Enjon; 

#ifdef main
	#undef main
#endif
int main(int argc, char** argv)
{ 
	// Test suite for things
	
	return 0;
}
