/***************************************************************************
 * Copyright © 2006 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 *
 ***************************************************************************/

/**
* @file dsmdefs.h
* System dependend macros.
* Defines and typedefs to accommodate differences with different systems.
* @author JFL Peripheral Solutions Inc.
* @date October 2005
*/

#ifndef __DSMDEFS_H__
#define __DSMDEFS_H__

#include "twain/twain.h"

#ifndef FALSE
 #define FALSE 0
 #define TRUE (!FALSE)
#endif // FALSE
/**
* @def DllExport
* set system dll export configuration __declspec( dllexport )
* 
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
* @def SNPRINTF
* OS abstraction macro that calls system _snprintf function.
*/

#ifdef _WIN32
  #define DllExport   __declspec( dllexport )
  #define LOADLIBRARY(lib) LoadLibrary(lib) 
  #define LOADFUNCTION(lib, func) GetProcAddress(lib, func)
  #define UNLOADLIBRARY(lib) FreeLibrary(lib)
  #define SNPRINTF _snprintf
#else
  #define DllExport
  #define LOADLIBRARY(lib) dlopen(lib, RTLD_NOW)
  #define LOADFUNCTION(lib, func) dlsym(lib, func)
  #define UNLOADLIBRARY(lib) dlclose(lib)
  #define SNPRINTF snprintf
#endif

/**
* correctly set the id of incoming systems IDs.
* The application ID is always +1 greater then the array index it resides in.
*/
#define kIN(id) ((id)-1)
/**
* correctly set the id of outgoing systems IDs.
* The application ID is always +1 greater then the array index it resides in.
*/
#define kOUT(id) ((id)+1)

/**
* Maximun number of Data Souces that can be opened
*/
#define MAX_NUM_DS 50

/**
* Maximun number of Applications that can be opened
*/
#define MAX_NUM_APPS 50

/**
* possible States of the DSM.
* The three possible states of the Data Source Manager
*/
typedef enum
{
  dsmState_PreSession = 1,/**< Source Manager not loaded. */
  dsmState_Loaded,        /**< Source Manager is loaded, but not open. */
  dsmState_Open           /**< Source Manager is open, and ready to list, open, and close sources. */
} DSM_State;

/**
* callback Info.  
*/
typedef struct
{
  TW_CALLBACK  callback;         /**< callback structure */
  TW_BOOL      bCallbackPending; /**< True if an application is old style and a callback was supposed to be made to it */
} CallBackInfo;

/**
* Describes the Data Source.  
*/
typedef struct
{
  TW_INT16    bOpen;      /**< Is this data source open */
  TW_IDENTITY Identity;   /**< Identity info for data source */
  
 #ifdef _WIN32
  HMODULE 
 #else
  void*
 #endif
  pHandle;                /**< returned by LOADLIBRARY(...) */
  
  DSENTRYPROC DS_Entry;   /**< function pointer to the DS_Entry function -- set by dlsym(...) */
  char szPath[MAX_PATH];  /**< location of the DS */
} DS_INFO,
  *pDS_INFO;

/**
* Structure to hold a list of Data Sources.
*/
typedef struct
{
  TW_UINT16 NumFiles;                 /**< Number of items in list */
  TW_BOOL   Initialized;              /**< has the list been initialized */
  DS_INFO   DSInfo[MAX_NUM_DS];       /**< array of Data Sources */
} DS_LIST,
 *pDS_LIST;                           /**< Pointer to structure to hold a list of Data Sources. */

/**
* Structure to hold data about a connected application
*/
typedef struct
{
  pTW_IDENTITY pOrigin;               /**< the applications identity */
  TW_INT16     ConditionCode;         /**< the apps condition code */
  DSM_State    CurrentState;          /**< the current state of the DSM for this app */
  TW_INT16     OpenSource;            /**< the index into DSInfo of the source selected by the application DG_CON/DAT_IDENT/MSG_OPENDS */
  pDS_LIST     pDSList;               /**< Each Application has a list of DS that it discovers each time the app opens the DSM */
  CallBackInfo Callback;              /**< each application can register a callback */
} APP_INFO,
 *pAPP_INFO;                          /**< Pointer to structure to hold data about a connected application */

/**
* Structure to hold list of connected applications.
* In 32bit enviroment Each application will connect to a seperate instance of DSM data
* but with this list it allows ONE applicaiton to connect several time, as long as it 
* uses a different name with each connection.
*/
typedef struct
{
  TW_UINT16   NumApps;                /**< the number of connected apps */
  APP_INFO    AppInfo[MAX_NUM_APPS];  /**< a list of applications */
} APP_LIST;

#endif // __DSMDEFS_H__
