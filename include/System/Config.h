#ifndef ENJON_CONFIG_H
#define ENJON_CONFIG_H

///////////////
// SYSTEM OS //
///////////////


// Note(John): Identify the operating system being used
#if defined(_WIN32) || defined(_WIN64) 
	// Windows
	#define ENJON_SYSTEM_WINDOWS
	
	// Just in case
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

#elif defined(__APPLE__) && defined(__MACH__) 
	// Mac	
	#define ENJON_SYSTEM_OSX

// UNIX systems
#elif defined(__linux__) 
	// Linux
	#define ENJON_SYSTEM_LINUX

#elif defined(__FreeBSD__) || defined (__FreeBSD_kernel__)
	// FreeBSD
	#define ENJON_SYSTEM_FREEBSD

	#else
		#error This UNIX operating system is not supported by Enjon
#endif
// #else
	
// 	// Unsupported OS
// 	#error This operating system is not supported by OS 

// #endif

//////////////////////////
// Environment Bit Size //
//////////////////////////

#if defined(_WIN32) || defined(_WIN64)
	#if defined (_WIN64)
		#define ENJON_64_BIT
	#else 
		#define ENJON_32_BIT
	#endif
#endif

// TODO(John): Check if ENJON_ENVIRONMENT works on clang
#if defined(__GNUC__)
	#if defined(__x86_64__) || defined(__ppc64__)
		#define ENJON_64_BIT
	#else
		#define ENJON_32_BIT
	#endif
#endif 

//////////////
// Compiler //
//////////////

#if defined(_MSC_VER)
	// Microsoft VS
	#define ENJON_COMPILER_MSVC
#elif defined(__clang__)
	// Clang
	#define ENJON_COMPILER_CLANG
#elif defined(__GNUC__) || defined (__GNUG) && !(defined(__clang__) || defined (__INTEL_COMPILER))
	// GNU GCC/G++ Compiler
	#define ENJON_COMPILER_GNU_GCC
#elif defined(__INTEL_COMPILER)
	// Intel C++ Compiler
	#define ENJON_COMPILER_INTEL
#endif 


#endif