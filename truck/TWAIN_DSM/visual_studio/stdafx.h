// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Windows Header Files:
#include <windows.h>

// TODO: reference additional headers your program requires here
#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif
