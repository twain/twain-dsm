/***************************************************************************
 * Copyright © 2007 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 *
 ***************************************************************************/

/**
* @file TestDSM.cpp 
* Test the Windows version of the TWAIN DSM.
* Copy the twain_32.dll into the Windows directory and hold it open to prevent 
* winodws from replacing it back with the original.
* @author JFL Peripheral Solutions Inc.
* @date October 2006
*/

#include "stdafx.h"
#include "TestDSM.h"
#include "TestDSMDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestDSMApp

BEGIN_MESSAGE_MAP(CTestDSMApp, CWinApp)
  ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTestDSMApp construction

CTestDSMApp::CTestDSMApp()
{
  // TODO: add construction code here,
  // Place all significant initialization in InitInstance
}


// The one and only CTestDSMApp object

CTestDSMApp theApp;


// CTestDSMApp initialization

BOOL CTestDSMApp::InitInstance()
{
  CWinApp::InitInstance();

  // Standard initialization
  // If you are not using these features and wish to reduce the size
  // of your final executable, you should remove from the following
  // the specific initialization routines you do not need
  // Change the registry key under which our settings are stored
  // TODO: You should modify this string to be something appropriate
  // such as the name of your company or organization
  SetRegistryKey(_T("Local AppWizard-Generated Applications"));

  CTestDSMDlg dlg;
  m_pMainWnd = &dlg;
  INT_PTR nResponse = dlg.DoModal();
  if (nResponse == IDOK)
  {
    // TODO: Place code here to handle when the dialog is
    //  dismissed with OK
  }
  else if (nResponse == IDCANCEL)
  {
    // TODO: Place code here to handle when the dialog is
    //  dismissed with Cancel
  }

  // Since the dialog has been closed, return FALSE so that we exit the
  //  application, rather than start the application's message pump.
  return FALSE;
}
