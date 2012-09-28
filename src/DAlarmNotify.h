#pragma once


// DAlarmNotify dialog

class DAlarmNotify : public CDialog
{
	DECLARE_DYNAMIC(DAlarmNotify)

public:
	DAlarmNotify(CWnd* pParent = NULL);   // standard constructor
	virtual ~DAlarmNotify();

// Dialog Data
	enum { IDD = IDD_ALARMNOTIFY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString TextMessage;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
