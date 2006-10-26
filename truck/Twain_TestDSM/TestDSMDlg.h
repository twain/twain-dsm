/***************************************************************************
 * Copyright © 2006 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 *
 ***************************************************************************/

/**
* @file TestDSMDlg.h
* Test the Windows version of the TWAIN DSM.
* Copy the twain_32.dll into the Windows directory and hold it open to prevent 
* winodws from replacing it back with the original.
* Header file.
* @author JFL Peripheral Solutions Inc.
* @date October 2006
*/

#pragma once
#include "afxwin.h"


// CTestDSMDlg dialog
class CTestDSMDlg : public CDialog
{
// Construction
public:
	CTestDSMDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTDSM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
  FILE *m_stream;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
  CString m_sTest;
  CButton m_btn_Browse;
  CButton m_btn_Test;
  afx_msg void OnBnClickedBtnBrowse();
  afx_msg void OnBnClickedBtnTest();
protected:
  virtual void OnCancel();
public:
  afx_msg void OnClose();
};
