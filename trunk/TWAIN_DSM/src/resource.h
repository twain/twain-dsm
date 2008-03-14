//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by dsm.rc
//

// Determine what bit verison of OS we are using
#if defined(WIN64) || defined(__x86_64__) || defined(__LP64__) || defined(_M_X64) || defined(_M_IA64)
  #define TWNDSM_OS_BIT_STR   "64"
#else 
  #define TWNDSM_OS_BIT_STR   "32"
#endif

//
// Identity information that we'll toss into the log and into the version
// resource on Windows...
//


#define TWNDSM_ORGANIZATION     "TWAIN Working Group"
#define TWNDSM_DESCRIPTION      "TWAIN " TWNDSM_OS_BIT_STR " Source Manager (Image Acquisition Interface)"
#define TWNDSM_VERSIONMAJOR     "2"
#define TWNDSM_VERSIONMINOR     "0"

#define ID_LST_SOURCES          10
#define IDC_STATIC              11
#define IDD_DLG_SOURCE          101

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        102
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1003
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
