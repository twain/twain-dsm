/***************************************************************************
 * Copyright © 2007 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 *
 ***************************************************************************/

/**
* @file dsm.h
* System dependend macros.
* Defines and typedefs to accommodate differences with different systems.
* @author JFL Peripheral Solutions Inc.
* @date October 2005
*/


#ifndef __DSM_H__
#define __DSM_H__
/**
* @def LOADLIBRARY(lib)
* Call system loadibrary function.  OS abstraction macro that tries to load a library.
* @param lib path and name of library
* 
* @def LOADFUNCTION(lib, func)
* Call system GetProcAddress function.  OS abstraction macro that tries to locate the addess of a funtion name.
* @param lib path and name of library
* @param func name of the funtion
*
* @def UNLOADLIBRARY(lib)
* Call system FreeLibrary function.  OS abstraction macro that tries to release the library.
* @param lib library modual to unload
* 
* @def READ
* OS abstraction macro that calls system _read function.
* 
* @def CLOSE
* OS abstraction macro that calls system _close function.
* 
* @def SNPRINTF
* OS abstraction macro that calls system _snprintf function.
* 
* @def UNLINK
* OS abstraction macro that calls system _unlink function.
*/


#ifdef _WIN32
  #define LOADLIBRARY(lib) LoadLibrary(lib) 
  #define LOADFUNCTION(lib, func) GetProcAddress(lib, func)
  #define UNLOADLIBRARY(lib) FreeLibrary(lib)
  #define READ _read
  #define CLOSE _close
  #define SNPRINTF _snprintf
  #define UNLINK _unlink
#else
  #define LOADLIBRARY(lib) dlopen(lib, RTLD_NOW)
  #define LOADFUNCTION(lib, func) dlsym(lib, func)
  #define UNLOADLIBRARY(lib) dlclose(lib)
  #define READ read
  #define CLOSE close
  #define SNPRINTF snprintf
  #define UNLINK unlink
/**
* @def max(a,b)
* return the maximum of a and b.
*
* @typedef unsigned int
* create UINT
*/
  typedef unsigned int UINT;
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#endif // __DSM_H__
