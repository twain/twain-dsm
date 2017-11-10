/*
* [Overview]
*   The contents of this module allow the new TWAINDSM.DLL (tied to the 2.x version
*   of the TWAIN Specifiction) to hook functions that might try to access the 1.x
*   TWAIN_32.DLL.  We need this because the DG_CONTROL/DAT_NULL/MSG_* triplets are
*   sent from the data source (driver) to the application through the data source
*   manager (DSM).  The technique used was for a driver to issue a LoadLibrary or
*   a GetModuleHandle, which would result in it attaching to the same TWAIN_32.DLL
*   first loaded by the application.  We have to force the driver to attach to the
*   new TWAINDSM.DLL instead.
*
*
* [Diagram]
*   This is the problem we have...
*   +-----+           +--------------+           +--------------+
*   | app | <-------> | TWAINDSM.DLL | --------> |              |
*   +-----+           +--------------+           |              |
*                                                | driver (1.x) |
*                     +--------------+           |              |
*        error <----- | TWAIN_32.DLL | <-------- |              |
*                     +--------------+           +--------------+
*
*   By hooking the appropriate function, we change the driver's request to get
*   the DSM_Entry function from TWAIN_32.DLL into the DSM_Entry function from
*   DSM_ TWAINDSM.DLL
*   +-----+           +--------------+           +--------------+
*   | app | <-------> | TWAINDSM.DLL | <-------> |              |
*   +-----+           +--------------+           |              |
*                                                | driver (1.x) |
*                     +--------------+           |              |
*                     | TWAIN_32.DLL |           |              |
*                     +--------------+           +--------------+
*
*   Note that TWAIN_32.DLL is still in the picture.  This is intentional,
*   because we're (indirectly) hooking GetProcAddress.  The driver still
*   does a LoadLibrary or GetModuleHandle on TWAIN_32.DLL.  This design
*   was selected because it results in the smallest possible hook code,
*   and makes the smallest possible change to the 1.x driver.  Everything
*   runs the same as a TWAIN_32.DLL session, we just return a pointer to
*   a different DSM_Entry function.
*
*
* [Hooked Functions]
*   This is the ntdll.dll function we're hooking from kernel32.dll...
*     LdrGetProcedureAddress (or LdrGetProcedureAddressForCaller)
*   This gives us full coverage for the following function...
*     GetProcAddress
*
*
* [Solution]
*    The hook code is derived from the BugSlayer HookImportedFunctionByName function
*    described in the August 1998 MSJ.  The code has been considerably simplified to
*    meet the limited needs of the DSM.
*
*
* [Risks]
*    32-bit Windows applications using TWAINDSM.DLL will never be able to access
*    the TWAIN_32.DLL DSM_Entry function.  Since this is by design, it's not so
*    much a risk as "the plan", but it still deserves to be mentioned.
*
*    We're not fully loading the TWAIN_32.DLL, because we don't have control over
*    it, and we can't be sure what it's doing in DllMain.  This could be a problem
*    with a very badly behaved application.  The scenerio is so convoluted, though,
*    that it seems a good risk to keep TWAIN_32.DLL as uninvolved as possible.
*
*    The application will crash if it does a FreeLibrary() on the DSM without first
*    doing all the necessary MSG_CLOSEDS and MSG_CLOSEDSM calls.  This can be
*    mitigated by hooking LdrUnloadDll() and watching for an attempt to unload the
*    DSM.  But this adds complexity that rewards extremely bad coding behavior, so
*    it's not going to be added unless we have a lot of bad actors we have to deal
*    with.
*
*    Hooks should be avoided whenever possible.  Therefore the DSM code only installs
*    the hook if the application attempts to DG_CONTROL/DAT_IDENTITY/MSG_OPENDS a 1.x
*    driver on a 32-bit Windows system.
*
*    This system is only designed to work with Window 2000 and higher, there is no
*    intention of supporting either Windows NT or any of the Windows 9x platforms.
*    Nor is there a reason to, since they don't have a file protection scheme (save for
*    WinME, but hopefully nobody is still using that)...
*/

#include "dsm.h"


/**
* This entire file is only used for 32-bit Windows systems, so there is no need to
* compile it for anything else...
*/
#if TWNDSM_OS_64BIT
#pragma message( "hook code disabled for 64-bit builds..." )
#elif TWNDSM_OS_32BIT
#pragma message( "hook code enabled for 32-bit builds..." )



/**
* We use this to build pointers from the various data structures we
* have to navigate to set up the hooks...
*/
#define MakePtr(cast,ptr,AddValue) (cast)((DWORD_PTR)(ptr)+(DWORD_PTR)(AddValue))


/**
* Things we do with our Hook function...
*/
enum EHOOK
{
  HOOK_ATTACH = 0,
  HOOK_DETACH = 1
};


/**
* Need this for our Ldr functions...
*/
typedef struct _ANSI_STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PSTR    Buffer;
} ANSI_STRING, *PANSI_STRING;


/**
* typedefs of our hooked functions, so we can cast them nice when we make
* our calls...
*/
typedef NTSYSAPI DWORD (NTAPI *LdrGetProcedureAddress_t)
(
  __in     HMODULE       ModuleHandle,
  __in_opt PANSI_STRING  FunctionName,
  __in_opt WORD          Oridinal,
  __out    PVOID        *FunctionAddress
);
typedef NTSYSAPI DWORD (NTAPI *LdrGetProcedureAddressForCaller_t)
(
  __in     HMODULE       ModuleHandle,
  __in_opt PANSI_STRING  FunctionName,
  __in_opt WORD          Oridinal,
  __out    PVOID        *FunctionAddress,
  __in     BOOL          bValue,
  __in     PVOID        *CallbackAddress
);

/**
* Forward declarations for our functions, so we can build our m_proc table...
*/
DWORD NTAPI LocalLdrGetProcedureAddress
(
  __in     HMODULE       ModuleHandle,
  __in_opt PANSI_STRING  FunctionName,
  __in_opt WORD          Oridinal,
  __out    PVOID        *FunctionAddress
);
DWORD NTAPI LocalLdrGetProcedureAddressForCaller
(
  __in     HMODULE       ModuleHandle,
  __in_opt PANSI_STRING  FunctionName,
  __in_opt WORD          Oridinal,
  __out    PVOID        *FunctionAddress,
  __in     BOOL          bValue,
  __in     PVOID        *CallbackAddress
);

/**
* The entry point we want to return to the caller, instead of the one they
* thought they were getting from TWAIN_32.DLL...
*/
extern DSMENTRY DSM_Entry
(
  TW_IDENTITY  *_pOrigin,
  TW_IDENTITY  *_pDest,
  TW_UINT32     _DG,
  TW_UINT16     _DAT,
  TW_UINT16     _MSG,
  TW_MEMREF     _pData
);


/**
* The hook class...
*/
class CTwHook
{
  public:

    // Initialize our piece-of-data...
    CTwHook()
    {
      memset(&pod,0,sizeof(pod));
    }

    // The destructor cleans us all up (but Hook does the real work)...
    ~CTwHook()
    {
      Hook(HOOK_DETACH);
      memset(&pod,0,sizeof(pod));
    }

    // The workhorse, this is where the hooking fun really takes place...
    bool Hook
    (
      EHOOK _ehook
    );

    // Determine if this DS ID has been hooked
    bool DSID_Is_Hooked
    (
      TW_UINT32 DSID
    );

    // Add this DS ID as being hooked
    void Hook_Add_DSID
    (
      TW_UINT32 DSID
    );

    // Remove this DS ID as being hooked
    bool Hook_Remove_DSID
    (
      TW_UINT32 DSID
    );

  private:

    // We use a pod so that we don't have to worry about initialization
    // issues...
    struct Pod
    {
      PROC  pOriginal;  // The original procedure we found
      TW_UINT32 HookedDSs[MAX_NUM_DS];
    } pod;
};


/**
* This is where we're keeping our stuff, it has global scope in this module so we can
* use it in our hook functions.  Even though this is static, every attempt has been
* made to make it thread safe.  We also have a reference counter, since the application
* may choose to load more than one 1.x driver...
*/
static int s_iHookCount = 0;
static CTwHook *s_ptwhook = (CTwHook*)NULL;


/**
* We start by loading these functions using GetProcAddress, since they are
* wacky NTDLL things that are undocumented.  Later on we change them to the
* actual pointers to the Ldr functions, and this is what allows us to be
* used safely, no matter what the state of the CTwHook object is in...
*/
static LdrGetProcedureAddress_t OriginalLdrGetProcedureAddress = 0;
static LdrGetProcedureAddressForCaller_t OriginalLdrGetProcedureAddressForCaller = 0;


/**
* A static value for the TWAIN_32.DLL we load, which provides us a very
* simple way to see if someone is working with this DLL...
*/
static HMODULE s_hmoduleTWAIN32 = 0;

/**
* A static value for the DSMEntry of the TWAIN_32.DLL we load, 
* which provides us a way to call the original TWAIN_32 if we
* did not want to hook this data souce.
*/
static DSMENTRYPROC TWAIN32_DSMEntry = 0;



/**
* The entry point we want to return to the caller, instead of the one they
* thought they were getting from TWAIN_32.DLL...
*/
DSMENTRY DSM_HookedEntry
(
	TW_IDENTITY	*_pOrigin,
	TW_IDENTITY	*_pDest,
	TW_UINT32	_DG,
	TW_UINT16	_DAT,
	TW_UINT16	_MSG,
	TW_MEMREF	_pData
)
{
	if	(	((_DAT == DAT_NULL)
		 ||  (_DAT == DAT_CALLBACK && _MSG == MSG_INVOKE_CALLBACK))
		 && _pOrigin
		 && s_ptwhook
		 && s_ptwhook->DSID_Is_Hooked(_pOrigin->Id))
	{
		return (DSM_Entry(_pOrigin,_pDest,_DG,_DAT,_MSG,_pData));
	}

	return (TWAIN32_DSMEntry(_pOrigin,_pDest,_DG,_DAT,_MSG,_pData));
}



/**
* Derived from John Robbins' BugSlayer code in MSJ (so the 'I' in the comments in this
* function refers to John), this function hooks or unhooks the stuff we want to examine.
* Note that this code is optimized to hook the Ldr functions in ntdll.dll.  If you want
* to hook something else, especially if it's in a different DLL, you're going to have
* some work to do...
* @param[in] EHOOK _ehook we're hooking or unhooking
* @return true or false
*/
bool CTwHook::Hook
(
	EHOOK _ehook
)
{
	UINT						uResult;
	BOOL                        boolResult;
	PIMAGE_IMPORT_DESCRIPTOR    pImportDesc;
	PIMAGE_THUNK_DATA           pOrigThunk;
	PIMAGE_THUNK_DATA           pRealThunk;
	PIMAGE_IMPORT_BY_NAME       pByName;
	bool                        bDoHook;
	MEMORY_BASIC_INFORMATION    mbi_thunk;
	DWORD_PTR                  *pTemp;
	DWORD                       dwOldProtect;
	PIMAGE_DOS_HEADER           pDOSHeader;
	PIMAGE_NT_HEADERS           pNTHeader;
	PSTR                        szCurrMod;
	HMODULE                     hmodule;
	char                        szTwain32[MAX_PATH];
	const char                 *szFunctionName;

	// Initialize stuff when we're doing an attach...
	if (_ehook == HOOK_ATTACH)
	{
		// If we don't find TWAIN_32.DLL, then we're not going to
		// make life any better by doing the hooks, so bail.  Also,
		// We're not going to allow any flavor of GetProcAddress
		// to access this library, which is why we're using the
		// extra flag, to keep TWAIN_32.DLL from loading anything
		// other than itself.
		//
		// BUG ALERT
		// ~~~~~~~~~
		// There is a potential bug here, but one that depends
		// on really bad behavior.  If the application loads
		// TWAINDSM.DLL, then loads TWAIN_32.DLL, then unloads
		// TWAINDSM.DLL, then does a GetProcAddress for DSM_Entry
		// with TWAIN_32.DLL, it's going to go ka-boom.  If this
		// is a problem, then go back to using ::LoadLibrary()...
		memset(szTwain32,0,sizeof(szTwain32));
		uResult = ::GetWindowsDirectory(szTwain32,sizeof(szTwain32)-1);
		if (!uResult)
		{
			return(false);
		}
		SSTRCAT(szTwain32,sizeof(szTwain32)-1,"\\TWAIN_32.DLL");
		s_hmoduleTWAIN32 = ::LoadLibraryEx(szTwain32,NULL,DONT_RESOLVE_DLL_REFERENCES);
		if (0 == s_hmoduleTWAIN32)
		{
			return(false);
		}

		// Load the undocumented routine, we're going for both, if we find
		// LdrGetProcedureAddressForCaller, then we'll prefer it over LdrGetProcedureAddress...
		szFunctionName = "";
		OriginalLdrGetProcedureAddress = 0;
		OriginalLdrGetProcedureAddressForCaller = 0;
		hmodule = GetModuleHandle("ntdll.dll");
		if (hmodule)
		{
			szFunctionName = "LdrGetProcedureAddressForCaller";
			OriginalLdrGetProcedureAddressForCaller = (LdrGetProcedureAddressForCaller_t)GetProcAddress(hmodule,szFunctionName);
			if (!OriginalLdrGetProcedureAddressForCaller)
			{
				szFunctionName = "LdrGetProcedureAddress";
				OriginalLdrGetProcedureAddress = (LdrGetProcedureAddress_t)GetProcAddress(hmodule,szFunctionName);
			}
		}
	}

	// If we're detaching, then make sure our reference to TWAIN_32.DLL is gone...
	else
	{
		if (s_hmoduleTWAIN32)
		{
			::FreeLibrary(s_hmoduleTWAIN32);
			s_hmoduleTWAIN32 = 0;
		}

		// If we don't have an old pointer, then we're done...
		if (NULL == pod.pOriginal)
		{
			return (true);
		}

		// Pick the name...
		if (OriginalLdrGetProcedureAddressForCaller)
		{
			szFunctionName = "LdrGetProcedureAddressForCaller";
		}
		else
		{
			szFunctionName = "LdrGetProcedureAddress";
		}
	}

	// This is where we'll be hooking into Ldr functions, the calls
	// themselves are in ntdll.dll.  Kernel32 calls them, and that's
	// where we can take advantage of the DLL indirection to do this
	// spiffy DLL injection thingy...
	// Starting with Windows7 the hooking has moved from kernel32.dll 
	// to a new library kernelbase.dll.  Atempt kernelbase first
	// if it fails then we are not Windows7 and try kernel32
	hmodule = GetModuleHandle("kernelbase.dll");
	if ( NULL == hmodule )
	{
		hmodule = GetModuleHandle("kernel32.dll");
	}

	// Get the DOS header...
	pDOSHeader = (PIMAGE_DOS_HEADER)hmodule;

	// Is this the MZ header?
	if (   !pDOSHeader
		|| (TRUE == IsBadReadPtr(pDOSHeader,sizeof(IMAGE_DOS_HEADER)))
		|| (IMAGE_DOS_SIGNATURE != pDOSHeader->e_magic))
	{
		return (false);
	}

	// Get the PE header.
	pNTHeader = MakePtr(PIMAGE_NT_HEADERS,pDOSHeader,pDOSHeader->e_lfanew);

	// Is this a real PE image?
	if (   (TRUE == IsBadReadPtr(pNTHeader,sizeof(IMAGE_NT_HEADERS)))
		|| (IMAGE_NT_SIGNATURE != pNTHeader->Signature))
	{
		return (false);
	}

	// If there is no imports section, leave now.
	if (0 == pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)
	{
		return (false);
	}

	// Get the pointer to the imports section.
	pImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR,pDOSHeader,pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	// Loop through the import module descriptors looking for the
	// ntdll.dll module, which is where the Ldr functions live...
	while (NULL != pImportDesc->Name)
	{
		szCurrMod = MakePtr(PSTR,pDOSHeader,pImportDesc->Name);
		if (0 == _stricmp(szCurrMod,"ntdll.dll"))
		{
			// Found it.
			break;
		}
		// Look at the next one.
		pImportDesc++ ;
	}

	// If the name is NULL, then the module is not imported.
	if (NULL == pImportDesc->Name)
	{
		return (false);
	}

	// Get the original thunk information for this DLL. I can't use
	// the thunk information stored in pImportDesc->FirstThunk
	// because the loader has already changed that array to fix up
	// all the imports. The original thunk gives me access to the
	// function names.
	pOrigThunk = MakePtr(PIMAGE_THUNK_DATA,hmodule,pImportDesc->OriginalFirstThunk);

	// Get the array the pImportDesc->FirstThunk points to because
	// I'll do the actual bashing and hooking there.
	pRealThunk = MakePtr(PIMAGE_THUNK_DATA,hmodule,pImportDesc->FirstThunk);

	// Determines whether I hook the function
	bDoHook = false;

	// Loop through and find the function to hook.
	while (NULL != pOrigThunk->u1.Function)
	{
		// Look only at functions that are imported by name, not those
		// that are imported by ordinal value.
		if (IMAGE_ORDINAL_FLAG != (pOrigThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG))
		{
			// Look at the name of this imported function.
			pByName = MakePtr(PIMAGE_IMPORT_BY_NAME,hmodule,pOrigThunk->u1.AddressOfData);

			// We found it, so scoot...
			if (    pByName->Name
				&&  ('\0' != pByName->Name[0])
				&&  (0 == _stricmp(szFunctionName,(char*)pByName->Name)))
			{
			bDoHook = true;
			break;
			}

			// Increment both tables, and continue...
			pOrigThunk++;
			pRealThunk++;
		}
	}

	// If we found something, then hook or unhook it, as appropriate...
	if (true == bDoHook)
	{
		// I found a function to hook. Now I need to change
		// the memory protection to writable before I overwrite
		// the function pointer. Note that I'm now writing into
		// the real thunk area!
		VirtualQuery(pRealThunk,&mbi_thunk,sizeof(MEMORY_BASIC_INFORMATION));
		if (FALSE == VirtualProtect(mbi_thunk.BaseAddress,mbi_thunk.RegionSize,PAGE_EXECUTE_READWRITE,&mbi_thunk.Protect))
		{
			return(false);
		}

		// Save the original address, if we're hooking...
		if (_ehook == HOOK_ATTACH)
		{
			// This cast should make the compiler happy about the change in the word size...
			pod.pOriginal = (PROC)(INT_PTR)pRealThunk->u1.Function;
			if (OriginalLdrGetProcedureAddressForCaller)
			{
				OriginalLdrGetProcedureAddressForCaller = (LdrGetProcedureAddressForCaller_t)pod.pOriginal;
			}
			else
			{
				OriginalLdrGetProcedureAddress = (LdrGetProcedureAddress_t)pod.pOriginal;
			}
		}

		// Microsoft has two different definitions of the
		// PIMAGE_THUNK_DATA fields as they are moving to
		// support Win64. The W2K RC2 Platform SDK is the
		// latest header, so I'll use that one and force the
		// Visual C++ 6 Service Pack 3 headers to deal with it.

		// Hook the new function if we're hooking, or the original function
		// if we're closing...
		if (_ehook == HOOK_ATTACH)
		{
			pTemp = (DWORD_PTR*)&pRealThunk->u1.Function;
			if (OriginalLdrGetProcedureAddressForCaller)
			{
				*pTemp = (DWORD_PTR)LocalLdrGetProcedureAddressForCaller;
			}
			else
			{
				*pTemp = (DWORD_PTR)LocalLdrGetProcedureAddress;
			}
		}
		else
		{
			pTemp = (DWORD_PTR*)&pRealThunk->u1.Function;
			*pTemp = (DWORD_PTR)(pod.pOriginal);
		}

		// Change the protection back to what it was before I
		// overwrote the function pointer.
		boolResult = VirtualProtect(mbi_thunk.BaseAddress,mbi_thunk.RegionSize,mbi_thunk.Protect,&dwOldProtect);
		if (boolResult == FALSE)
		{
			// Okay, this isn't good, but we're not going to do
			// anything about it, because presumably this isn't
			// the end of the world...
		}
	}

	// All done...
	return(true);
}

/**
* Determine if the DS has been hooked by checking the ID
* @param[in] DSID The DS ID to check to see if it has been hooked
* @return true or false
*/
bool CTwHook::DSID_Is_Hooked(TW_UINT32 DSID)
{
	int count = min(MAX_NUM_DS,s_iHookCount);
	for (int i=0; i<count; i++)
	{
		if (pod.HookedDSs[i] == DSID)
		{
			return (true);
		}
	}
	return (false);
}

/**
* Add this DS to the list of being hooked
* @param[in] DSID The DS ID to add to the list
* @note We allow an application to open the DSM several times and each 
* application to open several DSs.  (The application must use a different 
* name each time it loads the DSM).  It is possible for a DS to be opened 
* several times by different applications. (Although most DSs only support 
* one connection.)  Therefore we need to allow multiple instances of the
* same DSID.  No duplicate check needed.
*/
void CTwHook::Hook_Add_DSID(TW_UINT32 DSID)
{
	pod.HookedDSs[s_iHookCount] = DSID;
	s_iHookCount++;
}

/**
* Remove this DS from the list of being hooked
* @param[in] DSID The DS ID to remove from the list
* @return true or false
*/
bool CTwHook::Hook_Remove_DSID(TW_UINT32 DSID)
{
  int count = min(MAX_NUM_DS, s_iHookCount);

  for(int i=0; i<count; i++)
  {
    if(pod.HookedDSs[i] == DSID)
    {
      while(i+1<count)
      {
        pod.HookedDSs[i] = pod.HookedDSs[i+1];
      }
      pod.HookedDSs[i] = 0;
      s_iHookCount--;
      return true;
    }
  }

  // Should never get here.  It means we are trying to 
  // remove a DS that was never hooked.
  kLOG((kLOGERR,"Trying to removing a hook for a DSID (%d) that was never added.", DSID  ));
  return false;
}



/**
* The undocumented LdrGetProcedureAddress...
* @param[in] PHMODULE ModuleHandle we're using to get a function pointer
* @param[in_opt] PANSI_STRING FunctionName of thing we're trying to find
* @param[in_opt] WORD Ordinal, or the number of the things we're trying to find
* @param[out] PVOID *FunctionAddress being sent back to the caller
* @return DWORD
*/
DWORD NTAPI LocalLdrGetProcedureAddress
(
  __in     HMODULE       ModuleHandle,
  __in_opt PANSI_STRING  FunctionName,
  __in_opt WORD          Ordinal,
  __out    PVOID         *FunctionAddress
)
{
  // See if the caller is asking for TWAIN_32.DLL, and if so, then dive in
  // and return our DSM_Entry.  This works because attempts to load the
  // same DLL more than once just bump up the reference count.  Or put
  // another way, if function A does a LoadLibrary("xyz") and then function
  // B does a LoadLibrary("xzy") the HMODULE values returned will be found
  // to be the same.
  //
  // We don't have to check the FunctionName or the Ordinal, there's only
  // one possible return from TWAIN_32.DLL (thank you initial designers),
  // and I haven't throught of a good security reason why I need to bother
  // checking...
  if (ModuleHandle == s_hmoduleTWAIN32)
  {
    // Get and store the original address in case we need it
    (OriginalLdrGetProcedureAddress(ModuleHandle,FunctionName,Ordinal,(PVOID*)&TWAIN32_DSMEntry));
    // Return the address to our own function
    *FunctionAddress = ::DSM_HookedEntry;
    return (ERROR_SUCCESS);
  }

  // Otherwise let the call continue unmolested...
  return (OriginalLdrGetProcedureAddress(ModuleHandle,FunctionName,Ordinal,FunctionAddress));
}



/**
* The undocumented LdrGetProcedureAddressForCaller...
* @param[in] PHMODULE ModuleHandle we're using to get a function pointer
* @param[in_opt] PANSI_STRING FunctionName of thing we're trying to find
* @param[in_opt] WORD Ordinal, or the number of the things we're trying to find
* @param[out] PVOID *FunctionAddress being sent back to the caller
* @param[in] BOOL bValue
* @param[in] PVOID *CallbackAddress
* @return DWORD
*/
DWORD NTAPI LocalLdrGetProcedureAddressForCaller
(
  __in     HMODULE       ModuleHandle,
  __in_opt PANSI_STRING  FunctionName,
  __in_opt WORD          Ordinal,
  __out    PVOID        *FunctionAddress,
  __in     BOOL          bValue,
  __in     PVOID        *CallbackAddress
)
{
  // See if the caller is asking for TWAIN_32.DLL, and if so, then dive in
  // and return our DSM_Entry.  This works because attempts to load the
  // same DLL more than once just bump up the reference count.  Or put
  // another way, if function A does a LoadLibrary("xyz") and then function
  // B does a LoadLibrary("xzy") the HMODULE values returned will be found
  // to be the same.
  //
  // We don't have to check the FunctionName or the Ordinal, there's only
  // one possible return from TWAIN_32.DLL (thank you initial designers),
  // and I haven't throught of a good security reason why I need to bother
  // checking...
  if (ModuleHandle == s_hmoduleTWAIN32)
  {
    // Get and store the original address in case we need it
    (OriginalLdrGetProcedureAddressForCaller(ModuleHandle,FunctionName,Ordinal,(PVOID*)&TWAIN32_DSMEntry,bValue,CallbackAddress));
    // Return the address to our own function
    *FunctionAddress = ::DSM_HookedEntry;
    return (ERROR_SUCCESS);
  }

  // Otherwise let the call continue unmolested...
  return (OriginalLdrGetProcedureAddressForCaller(ModuleHandle,FunctionName,Ordinal,FunctionAddress,bValue,CallbackAddress));
}



/**
* Install the hooks and load the library, if hooks are already installed,
* then just load the library...
*/
HMODULE InstallTwain32DllHooks
(
  const char* const _lib,
  const bool _hook,
  const TW_UINT32 _DSID
)
{
  HMODULE   hmodule;
  CTwHook  *ptwhook;
  DWORD     dwResult;

  // Init stuff...
  ptwhook = (CTwHook*)NULL;

  // We hook before we load the library so that we can intercept calls
  // to the Ldr functions during DllMain.  But we only do the hook if
  // it's the first time we've been here, and we've been asked to do
  // hooking (which we won't be if the DSM is doing GetFirst/GetNext to
  // enumerate the drivers)...
  if (_hook)
  {
    // If we already have a hook in place, then bump up our
    // reference counter...
    if (   (s_iHookCount > 0)
        && ((CTwHook*)NULL != s_ptwhook))
    {
      s_ptwhook->Hook_Add_DSID(_DSID);
    }

    // Otherwise load the beastie...
    else
    {
      // Allocate our object...
      s_iHookCount = 0;
      ptwhook = new CTwHook();
      if (ptwhook)
      {
        // Do the hook...
        if (ptwhook->Hook(HOOK_ATTACH))
        {
          // This activates our hooking functions to look for
          // attempts to get DSM_Entry...
          s_ptwhook = ptwhook;
          s_ptwhook->Hook_Add_DSID(_DSID);
        }
        // No joy, cleanup...
        else
        {
          delete ptwhook;
          ptwhook = (CTwHook*)NULL;
          s_ptwhook = (CTwHook*)NULL;
        }
      }
    }
  }

  // Load the library the caller asked for...
  hmodule = ::LoadLibrary(_lib);

  // If we hooked for this module, and the LoadLibrary failed, then
  // undo the hook...
  if (   (NULL == hmodule)
      && ((CTwHook*)NULL != ptwhook)
      && _hook)
  {
    dwResult = ::GetLastError();
    s_ptwhook->Hook_Remove_DSID(_DSID);
    if (s_iHookCount <= 0)
    {
      s_ptwhook = (CTwHook*)NULL;
      delete ptwhook;
      s_iHookCount = 0;
    }
    ::SetLastError(dwResult);
  }

  // All done...
  return(hmodule);
}



/**
* Uninstall the hooks (if needed), and free the library, if hooks are not currently
* installed, then just free the library...
*/
BOOL UninstallTwain32DllHooks
(
  const HMODULE _hmodule,
  const bool _unhook,
  const TW_UINT32 _DSID
)
{
  if(_unhook)
  {
    if (s_ptwhook)
    {
      // Remove the DS from the list and decrement the count...
      s_ptwhook->Hook_Remove_DSID(_DSID);

      // If we're at zero, then cleanup.  I'm probably being a bit
      // paranoid about the cleanup scheme, but I like being paranoid
      // when it comes to hooks...
      if (s_iHookCount <= 0)
      {
        CTwHook *ptwhook = s_ptwhook;
        s_ptwhook = (CTwHook*)NULL;
        delete ptwhook;
        s_iHookCount = 0;
      }
    }
  }
  // Free the library...
  return(::FreeLibrary(_hmodule));
}



// TWDSM_OS_64BIT/TWNDSM_OS_32BIT
#else
#error error, we need to be either 32-bit or 64-bit...
#endif
