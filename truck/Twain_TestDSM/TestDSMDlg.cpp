/***************************************************************************
 * Copyright © 2007 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 *
 ***************************************************************************/

/**
* @file TestDSMDlg.cpp 
* Test the Windows version of the TWAIN DSM.
* Copy the twain_32.dll into the Windows directory and hold it open to prevent 
* winodws from replacing it back with the original.
* Implementation file.
* @author JFL Peripheral Solutions Inc.
* @date October 2006
*/

#include "stdafx.h"
#include "TestDSM.h"
#include "TestDSMDlg.h"
#include ".\testdsmdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
  CAboutDlg();

// Dialog Data
  enum { IDD = IDD_ABOUTBOX };

  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CTestDSMDlg dialog



CTestDSMDlg::CTestDSMDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CTestDSMDlg::IDD, pParent)
  , m_sTest(_T(""))
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestDSMDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_EDT_TEST, m_sTest);
  DDX_Control(pDX, IDC_BTN_BROWSE, m_btn_Browse);
  DDX_Control(pDX, IDC_BTN_TEST, m_btn_Test);
}

BEGIN_MESSAGE_MAP(CTestDSMDlg, CDialog)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  //}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_BTN_BROWSE, OnBnClickedBtnBrowse)
  ON_BN_CLICKED(IDC_BTN_TEST, OnBnClickedBtnTest)
  ON_WM_CLOSE()
END_MESSAGE_MAP()


// CTestDSMDlg message handlers

BOOL CTestDSMDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  // Add "About..." menu item to system menu.

  // IDM_ABOUTBOX must be in the system command range.
  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu != NULL)
  {
    CString strAboutMenu;
    strAboutMenu.LoadString(IDS_ABOUTBOX);
    if (!strAboutMenu.IsEmpty())
    {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
    }
  }

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);     // Set big icon
  SetIcon(m_hIcon, FALSE);    // Set small icon

  // TODO: Add extra initialization here
  m_stream = NULL;

  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestDSMDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if ((nID & 0xFFF0) == IDM_ABOUTBOX)
  {
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
  }
  else
  {
    CDialog::OnSysCommand(nID, lParam);
  }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestDSMDlg::OnPaint() 
{
  if (IsIconic())
  {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else
  {
    CDialog::OnPaint();
  }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestDSMDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

void CTestDSMDlg::OnBnClickedBtnBrowse()
{
  // szFilters is a text string that includes two file name filters:
  // "*.my" for "MyType Files" and "*.*' for "All Files."
  char  szFilters[] = "TWAIN DLL (twain*.dll)|twain*.dll||";

  // Create an Open dialog; the default file name extension is ".my".
  CFileDialog fileDlg (TRUE, "TWAIN", "twain*.dll", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, this);

  // Display the file dialog. When user clicks OK, fileDlg.DoModal() 
  // returns IDOK.
  if( fileDlg.DoModal ()==IDOK )
  {
    m_sTest = fileDlg.GetPathName();
    UpdateData(false);
   //CString fileName = fileDlg.GetFileTitle ();
  }

}
char *pgsTwain  = "c:\\Windows\\twain_32.dll";
void CTestDSMDlg::OnBnClickedBtnTest()
{
  UpdateData(true);

  if(m_stream != NULL)
  {
    fclose(m_stream);
    m_stream = NULL;
    m_btn_Test.SetWindowText("Test");
    return;
  }

  if(m_sTest.IsEmpty())
  {
    Beep(1000, 10);
    return;
  }

  CopyFile(m_sTest, pgsTwain, FALSE);
  
  // Open for reading to lock the file so Windows does not replace it.
  if( (m_stream  = fopen( pgsTwain, "r" )) != NULL )
  {
    m_btn_Test.SetWindowText("Stop");
  }
}

void CTestDSMDlg::OnCancel()
{
  if(m_stream != NULL)
  {
    fclose(m_stream);
    m_stream = NULL;
  }
  
  // Windows will replace it automaticly
  DeleteFile(pgsTwain);
  CDialog::OnCancel();
}

void CTestDSMDlg::OnClose()
{
  if(m_stream != NULL)
  {
    fclose(m_stream);
    m_stream = NULL;
  }
  // Windows will replace it automaticly
  DeleteFile(pgsTwain);
  CDialog::OnClose();
}
