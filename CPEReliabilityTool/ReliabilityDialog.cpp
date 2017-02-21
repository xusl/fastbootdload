// ReliabilityDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CPEReliabilityTool.h"
#include "ReliabilityDialog.h"
#include "afxdialogex.h"
#include "log.h"

// CReliabilityDialog dialog

IMPLEMENT_DYNAMIC(CReliabilityDialog, CDialogEx)

CReliabilityDialog::CReliabilityDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CReliabilityDialog::IDD, pParent)
{

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

BOOL CReliabilityDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

    StartLogging(_T("CPEReliabilityTool.log"), "all", "all");

	m_TabCtrl.InsertItem(0, _T("MMI"));
	m_TabCtrl.InsertItem(1, _T("Program"));
	m_TabCtrl.SetCurSel(0); 

	m_MMITestDlg.Create(IDD_MMITEST, &m_TabCtrl);
	m_ProgramDlg.Create(IDD_PROGRAM, &m_TabCtrl);

	//设定在Tab内显示的范围
	CRect rc;

	m_TabCtrl.GetClientRect(rc);
	rc.top += 20;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;

	m_MMITestDlg.MoveWindow(&rc);
	m_ProgramDlg.MoveWindow(&rc);
	m_MMITestDlg.ShowWindow(SW_SHOW);
	m_ProgramDlg.ShowWindow(SW_HIDE);
//	EnableActiveTabCloseButton();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

CReliabilityDialog::~CReliabilityDialog()
{
}

void CReliabilityDialog::OnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    int CurSel = m_TabCtrl.GetCurSel();
	
	m_MMITestDlg.ShowWindow(CurSel == 0 ? SW_SHOW : SW_HIDE);
	m_ProgramDlg.ShowWindow(CurSel == 1 ? SW_SHOW : SW_HIDE);
	
    *pResult = 0;
}

void CReliabilityDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_TabCtrl);
}


BEGIN_MESSAGE_MAP(CReliabilityDialog, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CReliabilityDialog::OnSelchangeTab)
END_MESSAGE_MAP()

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CReliabilityDialog::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CReliabilityDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
// CReliabilityDialog message handlers
