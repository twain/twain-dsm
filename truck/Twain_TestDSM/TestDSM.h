/***************************************************************************
 * Copyright © 2007 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 *
 ***************************************************************************/

/**
* @file TestDSM.h 
* Test the Windows version of the TWAIN DSM.
* Copy the twain_32.dll into the Windows directory and hold it open to prevent 
* winodws from replacing it back with the original.
* Main header file for the PROJECT_NAME application.
* @author JFL Peripheral Solutions Inc.
* @date October 2006
*/

#pragma once

#ifndef __AFXWIN_H__
  #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"  // main symbols


// CTestDSMApp:
// See TestDSM.cpp for the implementation of this class
//

class CTestDSMApp : public CWinApp
{
public:
  CTestDSMApp();

// Overrides
  public:
  virtual BOOL InitInstance();

// Implementation

  DECLARE_MESSAGE_MAP()
};

extern CTestDSMApp theApp;