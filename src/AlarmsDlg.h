// AlarmsDlg.h : header file
//

#pragma once

class Alarm : public CObject
{
public:
	Alarm();
	Alarm(const Alarm& rhs);
	virtual ~Alarm();

	Alarm& operator=(const Alarm& rhs);

	CString TextMessage;

	enum
	{
		VF_TIMEOFDAY   = 1,
		VF_DAYOFWEEK   = 2,
		VF_DAYOFMONTH  = 4,
		VF_MONTHOFYEAR = 8
	};
	int ValidFieldFlags;

	int TimeOfDayHour;
	int TimeOfDayMinute;

	int DayOfWeek;

	int DayOfMonth;

	int MonthOfYear;

	void CheckAndTrigger();

	CTime TimeLastTriggered;
};

// CAlarmsDlg dialog
class CAlarmsDlg : public CDialog
{
// Construction
public:
	CAlarmsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ALARMS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	CString alarmsFile;
	CString alarmsSoundFile;

	UINT_PTR timerTrayIconRefresh;
	void OnTimerTrayIconRefresh(DWORD msg);

	CTime alarmsFileLastModifiedTime;
	UINT_PTR timerAlarmsFileReload;
	void OnTimerAlarmsFileReload();

	UINT_PTR timerTriggerAlarms;
	void OnTimerTriggerAlarms();

	CMenu popupMenu;
	CMenu* popupSubMenu;

	CTypedPtrArray<CObArray,Alarm*> Alarms;
	void ClearAlarms();

	void Tokenize(LPCTSTR inStr, LPCTSTR delim, CStringArray& toks, BOOL trimEachToken = TRUE);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnTrayMsg(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	friend class Alarm;

public:
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEditalarms();
	afx_msg void OnExit();
};
