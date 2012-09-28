// DAlarmNotify.cpp : implementation file
//

#include "stdafx.h"
#include "Alarms.h"
#include "DAlarmNotify.h"


// DAlarmNotify dialog

IMPLEMENT_DYNAMIC(DAlarmNotify, CDialog)

DAlarmNotify::DAlarmNotify(CWnd* pParent /*=NULL*/)
	: CDialog(DAlarmNotify::IDD, pParent)
	, TextMessage(_T(""))
{

}

DAlarmNotify::~DAlarmNotify()
{
}

void DAlarmNotify::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TEXTMESSAGE, TextMessage);
}


BEGIN_MESSAGE_MAP(DAlarmNotify, CDialog)
	ON_BN_CLICKED(IDOK, &DAlarmNotify::OnBnClickedOk)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// DAlarmNotify message handlers

BOOL DAlarmNotify::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

	CenterWindow();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DAlarmNotify::OnBnClickedOk()
{
	DestroyWindow();
}

HBRUSH DAlarmNotify::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	hbr = (HBRUSH)::GetStockObject(BLACK_BRUSH);
	pDC->SetTextColor(RGB(255,0,0));
	pDC->SetBkColor(RGB(0,0,0));

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}
