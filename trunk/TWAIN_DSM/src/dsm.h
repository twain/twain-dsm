/***************************************************************************
 * Copyright � 2007 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 *
 ***************************************************************************/

/**
* @file dsm.h
* Everything we need to make our .cpp files happy.
* 
* @author TWAIN Working Group
* @date March 2007
*/

#ifndef __DSM_H__
#define __DSM_H__


/**
* First off, figure out what compiler we're running and on which
* platform we think we're running it.  We assume that you're building
* on the same platform you intend to run, so if you are cross compiling
* you will likely have a bit of work to do here...
*/

/**
* Compilers we support...
*/
#define	TWNDSM_CMP_VISUALCPP	0x1001 // Preferably 2005+
#define TWNDSM_CMP_GNUGPP		0x1002 // Preferably v4.x+

/**
* Platforms we support...
*/
#define TWNDSM_OS_WINDOWS		0x2001 // Preferably Win2K+
#define TWNDSM_OS_MACOSX		0x2002 // Preferably 10.4+
#define TWNDSM_OS_LINUX			0x2003 // Preferably 2.6+ kernel

/**
* If the user defines TWNDSM_CMP in their make file or project,
* then we'll assume they want to take responsibility for picking
* how we'll build the system.  At this point it seems like the
* compiler definition is used to select which native library calls
* we're dealing with, while the os definition is more about
* where we'll expect to find stuff on the running system, like
* directories...
*/
#ifndef TWNDSM_CMP

  // GNU g++
  #if defined(__GNUC__)
    #define TWNDSM_CMP				TWNDSM_CMP_GNUGPP
    #define TWNDSM_CMP_VERSION		__GNUC__
    #if defined(__APPLE__)
      #define TWNDSM_OS			    TWNDSM_OS_MACOSX
    #else
      #define TWNDSM_OS             TWNDSM_OS_LINUX
    #endif

  // Visual Studio C++
  #elif defined(_MSC_VER)
    #define TWNDSM_CMP				TWNDSM_CMP_VISUALCPP
    #define TWNDSM_CMP_VERSION		_MSC_VER
    #define TWNDSM_OS				TWNDSM_OS_WINDOWS

  // ruh-roh...
  #else
    #error Sorry, we don't recognize this system...
  #endif
#endif



/**
*  Pull in the system specific headers...
*/
#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
  #endif
  #include <windows.h>
  #include <direct.h>

#elif  (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
  #include <dirent.h>
  #include <dlfcn.h>
  #include <unistd.h>
  #include <errno.h>
  #include <stdarg.h>
  #include <time.h>
  #include <sys/syscall.h>
  #include <sys/time.h>
  #define gettid() syscall(SYS_gettid)

#else
  #error Sorry, we don't recognize this system...
#endif



/**
* We use resource.h to specify version info on all platforms...
*/
#include "resource.h"



/**
* These headers are available on all platforms...
*/
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>



/**
* Don't forget to include TWAIN...
*/
#include "twain.h"



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
#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
  #define DllExport __declspec( dllexport )
  #define NCHARS(s) sizeof(s)
  #define LOADLIBRARY(lib) LoadLibrary(lib) 
  #define LOADFUNCTION(lib, func) GetProcAddress((HMODULE)lib, func)
  #define UNLOADLIBRARY(lib) FreeLibrary((HMODULE)lib)
  #define READ _read
  #define CLOSE _close
  #if (TWNDSM_CMP_VERSION >= 1400)
    #define SNPRINTF _snprintf_s
  #else
    #define SNPRINTF _snprintf
  #endif
  #define UNLINK _unlink
  #define STRNICMP _strnicmp
  #define DSMENTRY TW_UINT16 FAR PASCAL
  #define GETTHREADID ::GetCurrentThreadId
  #if (TWNDSM_CMP_VERSION >= 1400)
    #define FOPEN(pf,name,mode) (void)fopen_s(&pf,name,mode)
  #else
    #define FOPEN(pf,name,mode) pf = fopen(name,mode)
  #endif
  #ifndef kTWAIN_DS_DIR
    #define kTWAIN_DS_DIR "c:/windows/twain_32"
  #endif

#elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
  #define DllExport
  #define NCHARS(s) sizeof(s)
  #define LOADLIBRARY(lib) dlopen(lib, RTLD_LAZY)
  #define LOADFUNCTION(lib, func) dlsym(lib, func)
  #define UNLOADLIBRARY(lib) dlclose(lib)
  #define READ read
  #define CLOSE close
  #define SNPRINTF snprintf
  #define UNLINK unlink
  #define STRNICMP strncasecmp
  #define GETTHREADID gettid
  #define FOPEN(pf,name,mode) pf = fopen(name,mode)
  #ifndef kTWAIN_DS_DIR
    #define kTWAIN_DS_DIR "/usr/local/lib/twain"
  #endif
  typedef unsigned int UINT;
  typedef void* HINSTANCE;
  typedef void* HWND;
  #define DSMENTRY FAR PASCAL TW_UINT16

#else
  #error Sorry, we don't recognize this system...
#endif



/**
* We want to use secure string functions whenever possible, if g++
* every includes a set I think it would be excellent to switch over
* to it, but at least with Windows using them we stand a better
* chance of finding boo-boos...
*/
#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP) && (TWNDSM_CMP_VERSION >= 1400)
  #define SSTRCPY(d,z,s) strcpy_s(d,z,s)
  #define SSTRCAT(d,z,s) strcat_s(d,z,s)
  #define SSTRNCPY(d,z,s,m) strncpy_s(d,z,s,m)
  #define SGETENV(d,z,n) ::GetEnvironmentVariable(n,d,z)
  inline int SSNPRINTF(char *d, size_t z, size_t c, const char *f,...)
  {
      int result;
      va_list valist;
      va_start(valist,f);
      result = _vsnprintf_s(d,z,c,f,valist);
      va_end(valist);
      return result;
  }

/**
* These functions are insecure, but everybody has them, so we
* don't need an else/error section like we use everywhere else...
*/
#else
  #define SSTRCPY(d,z,s) strcpy(d,s)
  #define SSTRCAT(d,z,s) strcat(d,s)
  #define SSTRNCPY(d,z,s,m) strncpy(d,s,m)
  #define SGETENV(d,z,n) strcpy(d,getenv(n)?getenv(n):"")
  inline int SSNPRINTF(char *d, size_t, size_t c, const char *f,...)
  {
      int result;
      va_list valist;
      va_start(valist,f);
      #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
        result = _vsnprintf(d,c,f,valist);
      #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
        result = vsnprintf(d,c,f,valist);
      #else
        #error Sorry, we don't recognize this system...
      #endif
      va_end(valist);
      return result;
  }
#endif



/**
* These aren't logging levels, these are definitions to tell us whether
* or not to assert.
* @see kLOG
*/
#define kLOGINFO   0,__FILE__,__LINE__
#define kLOGERR    1,__FILE__,__LINE__

/**
* Define to write messages to LogFile.
* @see CTwnDsmLog
*/
#define kLOG(a) if (g_ptwndsmlog) g_ptwndsmlog->Log a

/**
* Display message to user.  Use this if logging is not an
* option, and this is the only way to track a problem!!!
* @see kLOG
*/
#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
  #define kPANIC(msg) ::MessageBox(NULL,msg,"TWAIN Data Source Manager",MB_OK);
#elif  (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
  #define kPANIC(msg) fprintf(stderr,"TWAIN Data Source Manager: %s\r\n",msg);
#else
  #error Sorry, we don't recognize this system...
#endif


/**
* Maximum number of Data Sources that can be opened under one
* application.  This item seems useful, though the number
* seems rather high.
*/
#define MAX_NUM_DS 50

/**
* Maximum number of Applications that can be opened in one session.
* This item smacks of legacy.  Under the Windows 3.1 and 9x
* platforms it was possible for one DLL to be shared among several
* programs.  Today there is no easy way to do this, but more
* importantly, no desireable reason to do this.  However, we'll
* keep it, because backwards compatability is a good thing.  We're
* not going to support a lot of them, though.
*/
#define MAX_NUM_APPS 8



/**
* Possible States of the DSM.
* The three possible states of the Data Source Manager.  We don't
* want to know about the other states, because that would add
* needless complexity.
*/
typedef enum
{
  dsmState_PreSession = 1, /**< Source Manager not loaded. */
  dsmState_Loaded     = 2, /**< Source Manager is loaded, but not open. */
  dsmState_Open       = 3  /**< Source Manager is open. */
} DSM_State;



/**
* @class CTwnDsmLog
* Our logging class.  We use the impl to encapsulate the private
* portions of the class, which doesn't matter for this class so
* much as it does for the next one.  Then we give ourselves an
* extern, because life is easier if we treat this object as globally
* accessible (think of it like a service).
*/
class CTwnDsmLogImpl;
class CTwnDsmLog
{
  public:

	/**
	* The CTwnDsmLog constructor.
	*/
    CTwnDsmLog();

	/**
	* The CTwnDsmLog destructor.
	*/
	~CTwnDsmLog();

	/**
	* The logging function.  This should only be access through
	* the kLOG macro...
	* @param[in] _doassert decides if we assert or not
	* @param[in] _file the source file of the message 
	* @param[in] _line the source line of the message 
	* @param[in] _format the format of the message (same as sprintf) 
	* @param[in] ... arguments to the format of the message 
	*/
	void Log(int  _doassert,
		     char *_file,
			 int  _line,
			 char *_format,
			 ...);

  private:

	/**
	* The implementation pointer helps with encapulation.
	*/
    CTwnDsmLogImpl *m_ptwndsmlogimpl;
};
extern CTwnDsmLog *g_ptwndsmlog;



/**
* @class CTwnDsmApps
* Class to hold list of connected applications.
* In 32bit enviroments each application will connect to a seperate
* instance of DSM data but with this list it allows ONE application
* to connect several time, as long as it uses a different name with
* each connection.  I'm still not sure why you'd want to do that,
* but there it is.  This class is intended to hide the gory details
* of how we're storing the data, so an impl is used.
*/
class CTwnDsmAppsImpl;
class CTwnDsmApps
{
  public:

	/**
	* The CTwnDsmApps constructor.
	*/
    CTwnDsmApps();

	/**
	* The CTwnDsmApps destructor.
	*/
    ~CTwnDsmApps();

	/**
	* Add an application.
	* This supports MSG_OPENDSM.
	* @param[out] _pAppId Origin of message
	* @param[in] _MemRef the HWND on Window, null otherwise
	* @return a valid TWRC_xxxx return code
	*/
    TW_UINT16 AddApp(TW_IDENTITY *_pAppId,
		             TW_MEMREF _MemRef);

	/**
	* Remove an application.
	* This supports MSG_CLOSEDSM.
	* @param[in] _pAppId Origin of message
	* @return a valid TWRC_xxxx return code
	*/
    TW_UINT16 RemoveApp(TW_IDENTITY *_pAppId);

	/**
	* Loads a DS from disk and adds it to a global list of DS's.
	* @param[in] _pAppId Origin of message
	* @param[in] _DsId the source index of the library to open
	* @return a valid TWRC_xxxx return code
	*/
	TW_INT16 LoadDS(TW_IDENTITY *_pAppId,
					TW_UINT32   _DsId);

	/**
	* Unloads a DS and frees all its resources...
	* @param[in] _pAppId Origin of message
	* @param[in] _DsId the source index
	*/
	void UnloadDS(TW_IDENTITY *_pAppId,
				  TW_UINT32   _DsId);

	/**
	* Validate that an id is in range...
	* @param[in] _pAppId id of App to test
	* @return TRUE if valid, else FALSE
	*/
	TW_BOOL AppValidateId(TW_IDENTITY *_pAppId);

	/**
	* Return a pointer to the application's identity.
	* Yeah, I know, this sorta violates encapsulation, but we don't
	* want to get silly about this...
	* @param[in] _pAppId id of identity to get
	* @return pointer to identity or NULL
	*/
	TW_IDENTITY *AppGetIdentity(TW_IDENTITY *_pAppId);

	/**
	* Get the condition code, then reset it internally to TWCC_SUCCESS,
	* so you can only get it once, per the specification...
	* @param[in] _pAppId id of app, or NULL if we have no apps
	* @return TWCC_ value
	*/
	TW_UINT16 AppGetConditionCode(TW_IDENTITY *_pAppId);

	/**
	* Set the condition code
	* @param[in] _pAppId id of app, or NULL if we have no apps
	* @param[in] _conditioncode code to use
	*/
	void AppSetConditionCode(TW_IDENTITY *_pAppId,
							 TW_UINT16 _conditioncode);

	/**
	* Set the state of the DSM for the specified application
	* @param[in] _pAppId id of app
	* @return DSM_State of the application
	*/
	DSM_State AppGetState(TW_IDENTITY *_pAppId);

	/**
	* Get the number of drivers we found as the result of a
	* successful call to LoadDS with _boolKeepOpen set to
	* false (meaning that we were just browsing)...
	* @param[in] _pAppId id of app
	* @return DSM_State of the application
	*/
    TW_UINT32 AppGetNumDs(TW_IDENTITY *_pAppId);

	/**
	* Poke the application to wake it up when sending a
	* DAT_NULL message to it...
	* @param[in] _pAppId id of app
	*/
    void AppWakeup(TW_IDENTITY *_pAppId);

	/**
	* Get a pointer to the identity of the specified driver...
	* @param[in] _pAppId id of app
	* @param[in] _DsId numeric id of driver
	* @return pointer to drivers identity or NULL
	*/
	TW_IDENTITY *DsGetIdentity(TW_IDENTITY *_pAppId,
							   TW_UINT32   _DsId);

	/**
	* Get a pointer to the DS_Entry function of the specified driver...
	* @param[in] _pAppId id of app
	* @param[in] _DsId numeric id of driver
	* @return pointer to DS_Entry for this driver or NULL
	*/
	DSENTRYPROC	 DsGetEntryProc(TW_IDENTITY *_pAppId,
								TW_UINT32   _DsId);

	/**
	* Get a pointer to the driver file path and name, which is guaranteed to
	* be unique, even if the ProductName's aren't for some horrible
	* reason...
	* @param[in] _pAppId id of app
	* @param[in] _DsId numeric id of driver
	* @return pointer to file path and name for this driver or NULL
	*/
	char *DsGetPath(TW_IDENTITY *_pAppId,
					TW_UINT32   _DsId);

	/**
	* Get a pointer to TW_CALLBACK structure for the specified driver...
	* reason...
	* @param[in] _pAppId id of app
	* @param[in] _DsId numeric id of driver
	* @return pointer to the callback structure for this driver or NULL
	*/
	TW_CALLBACK *DsCallbackGet(TW_IDENTITY *_pAppId,
							   TW_UINT32   _DsId);

	/**
	* Test if the driver has a callback pending for attention...
	* @param[in] _pAppId id of app
	* @param[in] _DsId numeric id of driver
	* @return TRUE if the driver needs its callback called
	*/
	TW_BOOL DsCallbackIsWaiting(TW_IDENTITY *_pAppId,
								TW_UINT32   _DsId);

	/**
	* Set the callback flag for the driver to TRUE if the callback
	* needs to have its callback called, and set it to FALSE after
	* the call has been made...
	* @param[in] _pAppId id of app
	* @param[in] _DsId numeric id of driver
	* @param[in] _Waiting the new state for the waiting flag
	*/
	void DsCallbackSetWaiting(TW_IDENTITY *_pAppId,
							  TW_UINT32   _DsId,
							  TW_BOOL     _Waiting);

  private:

	/**
	* The implementation pointer helps with encapulation.
	*/
    CTwnDsmAppsImpl *m_ptwndsmappsimpl;
};



/**
* This is the main class for the Data Source Manager.  Unlike the
* other classes this one isn't using an impl interface.  The
* rationale is that DSM_Entry is the true interface point, nobody
* who calls the DSM has to know anything about the implementation.
* So there's no benefit (except a programmer's desire for
* consistency) to putting in the impl.  I'm resisting that on the
* theory that if I don't need it, why make things more complex.
* YMMV...
*/
class CTwnDsm
{
	//
	// All of our public functions go here...
	//
	public:

		/**
		* Our CTwnDsm constructor...
		*/
		CTwnDsm();

		/**
		* Our CTwnDsm destructor...
		*/
		~CTwnDsm();

		/**
		* The guts of the DSM_Entry, the resource management portion
		* resides in a our DSM_Entry entry point, which isn't a part
		* of this class.  Hopefully it's not confusing that they have
		* the same name...
		* @param[in] _pOrigin Origin of message in this case a DS
		* @param[in] _pDest destination of message in this case an App
		* @param[in] _DG message id: DG_xxxx
		* @param[in] _DAT message id: DAT_xxxx
		* @param[in] _MSG message id: MSG_xxxx
		* @param[in] _pData the Data
		* @return a valid TWRC_xxxx return code
		*/
		TW_UINT16 DSM_Entry(TW_IDENTITY  *_pOrigin,
							TW_IDENTITY  *_pDest,
							TW_UINT32    _DG,
							TW_UINT16    _DAT,
							TW_UINT16    _MSG,
							TW_MEMREF    _pData);

		/**
		* Selection dialog, for apps that don't want to do GetFirst
		* GetNext.  This is only public because of the way that
		* dialogs are implemented.
		* @param[in] _hWnd Window handle of the dialog
		* @param[in] _Message message
		* @param[in] _wParam wparam
		* @param[in] _lParam lparam
		* @return FALSE if we processed the message
		*/
		#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
			BOOL CALLBACK SelectDlgProc(HWND _hWnd,
										UINT _Message,
										WPARAM _wParam,
										LPARAM _lParam);
		#elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
			// We don't have one of these...
		#else
			#error Sorry, we don't recognize this system...
		#endif



	//
	// All of our private functions go here...
	//
	private:

		/**
		* Handles DAT_NULL calls from DS for Application.
		* @param[in] _pAppId Origin of message
		* @param[in] _pDsId TW_IDENTITY structure
		* @param[in] _MSG message id: MSG_xxxx
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 DSM_Null(TW_IDENTITY *_pAppId,
						  TW_IDENTITY *_pDsId,
						  TW_UINT16 _MSG);

		/**
		* Returns the current DSM status. Resets pod.m_ConditionCode to
		* TWCC_SUCCESS per the specification.
		* @param[in] _pAppId Orgin of message
		* @param[in] _MSG message id: MSG_xxxx
		* @param[out] _pStatus TW_STATUS structure
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 DSM_Status(TW_IDENTITY *_pAppId,
							TW_UINT16  _MSG,
							TW_STATUS *_pStatus);

		/**
		* Initializes or closes the DSM
		* @param[in] _pAppId Orgin of message
		* @param[in] _MSG message id: MSG_xxxx
		* @param[in] _MemRef for Windows during MSG_OPENDSM it is HWND, null otherwise
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 DSM_Parent(TW_IDENTITY *_pAppId,
			                TW_UINT16 _MSG,
							TW_MEMREF _MemRef);

		/**
		* Source operations
		* @param[in] _pAppId Origin of message
		* @param[in] _MSG message id: MSG_xxxx
		* @param[in] _pDsId TW_IDENTITY structure
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 DSM_Identity(TW_IDENTITY *_pAppId,
			                  TW_UINT16 _MSG,
							  TW_IDENTITY *_pDsId);

		/**
		* Register application's callback.
		* @param[in] _pAppId Origin of message
		* @param[in] _pDsId TW_IDENTITY structure
		* @param[in] _MSG message id: MSG_xxxx valid = MSG_REGISTER_CALLBACK
		* @param[in] _pData pointer to a callback struct
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 DSM_Callback(TW_IDENTITY *_pAppId,
							  TW_IDENTITY *_pDsId,
							  TW_UINT16 _MSG,
							  TW_CALLBACK *_pData);

		/**
		* Opens the Data Source specified by pDSIdentity.  
		* pDSIdentity must be valid, but if a null name and id
		* is 0 then open default.
		* @param[in] _pAppId Origin of message
		* @param[in] _pDsId TW_IDENTITY structure
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 OpenDS(TW_IDENTITY *_pAppId,
			            TW_IDENTITY *_pDsId);

		/**
		* Closes the Data Source specified by pDSIdentity.
		* @param[in] _pAppId Origin of message
		* @param[in] _pDsId TW_IDENTITY structure
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 CloseDS(TW_IDENTITY *_pAppId,
			             TW_IDENTITY *_pDsId);

		/**
		* Displays the source select dialog and sets the default source.
		* @param[in] _pAppId Origin of message
		* @param[in,out] _pDsId TW_IDENTITY structure
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 DSM_SelectDS(TW_IDENTITY *_pAppId,
			                  TW_IDENTITY *_pDsId);

		/**
		* Goes through the applications supported data sources looking for one that has
		* the exact same name as product name in the passed in identity. Will update the
		* _pDsId structure to match the name.
		* @param[in] _pAppId Origin of message
		* @param[in,out] _pDsId TW_IDENTITY structure
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 GetDSFromProductName(TW_IDENTITY *_pAppId,
			                          TW_IDENTITY *_pDsId);

		/**
		* Copies the applications first available source into _pDsId.
		* @param[in] _pAppId The origin identity structure
		* @param[out] _pDsId the identity structure to copy data into
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 DSM_GetFirst(TW_IDENTITY *_pAppId,
			                  TW_IDENTITY *_pDsId);

		/**
		* Copies the applications next available source into _pDsId. A call to
		* DSM_GetFirst must have been made at least once before calling this function.
		* @param[in] _pAppId The origin identity structure
		* @param[out] _pDsId the identity structure to copy data into
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 DSM_GetNext(TW_IDENTITY *_pAppId,
			                 TW_IDENTITY *_pDsId);

		/**
		* This routine will check if the current default source matches the
		* applications supported groups.  If it does it will copy it into the default
		* Source's identity (_pDsId), otherwise this routine will search for a source that
		* does match the app's supported groups and copy it into _pDsId.
		* @param[in] _pAppId The application identity
		* @param[in,out] _pDsId A pointer reference that will be set to point to the default identity.
		* @return a valid TWRC_xxxx return code
		*/
		TW_INT16 GetMatchingDefault(TW_IDENTITY *_pAppId,
			                        TW_IDENTITY *_pDsId);

		/**
		* prints to stdout information about the triplets.
		* @param[in] _DG the Data Group
		* @param[in] _DAT the Data Argument Type
		* @param[in] _MSG the Message
		* @param[in] _pData the Data
		* @return return true if actually printed triplet
		*/
		bool printTripletsInfo(const TW_UINT32 _DG,
							   const TW_UINT16 _DAT,
							   const TW_UINT16 _MSG,
							   const TW_MEMREF _pData);

		/**
		* Translates the _MSG passed in into a string and returns it
		* @param[out] _szMsg string to copy into
		* @param[in] _nChars max chars in _szMsg
		* @param[in] _MSG the TWAIN message to translate
		*/
		void StringFromMsg(char *_szMsg,
						   int _nChars,
						   const TW_UINT16 _MSG);

		/**
		* Translates the _DAT passed in into a string and returns it
		* @param[out] _szDat string to copy into
		* @param[in] _nChars max chars in _szDat
		* @param[in] _DAT the TWAIN data argument type to translate
		*/
		void StringFromDat(char *_szDat,
						   int _nChars,
						   const TW_UINT16 _DAT);

		/**
		* Translates the _DG passed in into a string and returns it
		* @param[out] _szDg string to copy into
		* @param[in] _nChars max chars in _szDg
		* @param[in] _DG the TWAIN data group to translate
		*/
		void StringFromDg(char *_szDg,
						  int _nChars,
						  const TW_UINT32 _DG);

		/**
		* Translates the _Cap passed in into a string and returns it
		* @param[out] _szCap string to copy into
		* @param[in] _nChars max chars in _szCap
		* @param[in] _Cap the TWAIN Capability to translate
		*/
		void StringFromCap(char *_szCap,
						   int _nChars,
						   const TW_UINT16 _Cap);

		/**
		* Translates the rc passed in into a string and returns it
		* @param[out] _szRc string to copy into
		* @param[in] _nChars max chars in szRc
		* @param[in] _rc the TWAIN Return Code to translate
		*/
		void StringFromRC(char *_szRc,
						  int _nChars,
						  const TW_UINT16 _rc);



	//
	// All of our attributes should be private.  Encapsulation
	// is a good thing...  :)
	//
	private:

		/*
		**  If you add a class in future, declare it here and not
		**	in the pod, or the memset we don on pod will ruin your
		**  day...
		*/

		/**
		*  We use a pod system because it help prevents us from
		*  making dumb initialization mistakes.
		*/
		struct _pod
		{
			/**
			* The class taking care of our list of applications and drivers.
			*/
			CTwnDsmApps *m_ptwndsmapps;

			/**
			* The path to the default DS.  The Default DS is identified when
			* the DSM is opened.  A new Default is saved if SelectDlg is used.
			* So this value will be compared against DsGetPath()...
			*/
			char m_DefaultDSPath[FILENAME_MAX];

			/**
			* The next id to test for GetFirst/GetNext...
			*/
		    TW_UINT32 m_nextDsId;

			/**
			* The DS ID we end up with from SelectDlgProc.  This is only
			* used on the Windows platform.
			*/
			TW_IDENTITY *m_pSelectDlgDsId;

			/**
			* The Application ID we're using inside of SelectDlgProc. This
			* is only used on the Windows platform.
			*/
			TW_IDENTITY *m_pSelectDlgAppId;
		} pod;
};


#endif // __DSM_H__
