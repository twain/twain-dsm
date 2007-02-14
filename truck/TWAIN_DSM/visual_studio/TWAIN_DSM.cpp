/***************************************************************************
 * Copyright © 2006 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 *
 ***************************************************************************/

/**
* @file TWAIN_DSM.cpp
* Entry point for the DSM DLL.
* Defines the entry point for the Data Source Manager DLL application.
* @author JFL Peripheral Solutions Inc.
* @date October 2005
*/

#include "stdafx.h"
HINSTANCE g_hinstDLL = NULL;

/**
* Entry point
* @param hModule HANDLE to modual
* @param ul_reason_for_call DWORD of reason for call
* @param lpReserved 
* @return TRUE
*/
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
  if(ul_reason_for_call == DLL_PROCESS_ATTACH)
  {
    g_hinstDLL = (HINSTANCE)hModule;
  }

  return TRUE;
}

