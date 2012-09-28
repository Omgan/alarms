// AlarmsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Alarms.h"
#include "AlarmsDlg.h"
#include "DAlarmNotify.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ALARMS_FILE                        _T("Alarms.txt")
#define ALARMS_FILE_FIELD_DELIMETER        _T(",")
#define ALARMS_FILE_CONDITION_DELIMETER    _T("=")

#define CONDITION_TYPE_TIMEOFDAY           _T("TimeOfDay")
#define CONDITION_TYPE_TIMEOFDAY_SEP       _T(':')

#define CONDITION_TYPE_DAYOFWEEK           _T("DayOfWeek")
#define CONDITION_TYPE_DAYOFWEEK_SUNDAY    _T("Sunday")
#define CONDITION_TYPE_DAYOFWEEK_MONDAY    _T("Monday")
#define CONDITION_TYPE_DAYOFWEEK_TUESDAY   _T("Tuesday")
#define CONDITION_TYPE_DAYOFWEEK_WEDNESDAY _T("Wednesday")
#define CONDITION_TYPE_DAYOFWEEK_THURSDAY  _T("Thursday")
#define CONDITION_TYPE_DAYOFWEEK_FRIDAY    _T("Friday")
#define CONDITION_TYPE_DAYOFWEEK_SATURDAY  _T("Saturday")

#define CONDITION_TYPE_DAYOFMONTH          _T("DayOfMonth")

#define CONDITION_TYPE_MONTHOFYEAR         _T("MonthOfYear")

#define ALARMS_SOUND_FILE                  _T("Alarms.wav")

#define TIMER_TRAYICONREFRESH_ID           101
#define TIMER_TRAYICONREFRESH_INTERVAL_MS  3000

#define TIMER_ALARMSFILERELOAD_ID          102
#define TIMER_ALARMSFILERELOAD_INTERVAL_MS 1000

#define TIMER_TRIGGERALARMS_ID             103
#define TIMER_TRIGGERALARMS_INTERVAL_MS    1000

#define TRAY_ICON_ID                       201
#define TRAY_MSG                           (WM_APP+1)

LPCTSTR defaultAlarmsFile[] =
{
    _T("################################################################################"),
    _T("#"),
    _T("# Alarms configuration file"),
    _T("#"),
    _T("################################################################################"),
    _T("#-------------------------------------------------------------------------------"),
    _T("# OVERVIEW:"),
    _T("#-------------------------------------------------------------------------------"),
    _T("#"),
    _T("# Saving changes to this file will cause the alarms to be re-loaded if the"),
    _T("# Alarms application is running.  It is not necessary to restart the app."),
    _T("#"),
    _T("# Each line configures an alarm.  An alarm consists of a text message and a set"),
    _T("# of conditions which, when satisfied, trigger the alarm."),
    _T("#"),
    _T("#    Text Message,condition0,condition1,...,conditionN-1"),
    _T("#"),
    _T("# Each condition consist of a condition type and the data specific to that type"),
    _T("# of condition.  For example, a condition representing 3:30pm would be:"),
    _T("#"),
    _T("#    Time=15:30"),
    _T("#"),
    _T("# Multiple conditions for an alarm are AND'ed together, so they must ALL be"),
    _T("# satisfied in order for the alarm to trigger."),
    _T("#"),
    _T("# For alarms which have no time specified, the alarm will only trigger once per"),
    _T("# day."),
    _T("#"),
    _T("#-------------------------------------------------------------------------------"),
    _T("# EXAMPLES:"),
    _T("#-------------------------------------------------------------------------------"),
    _T("#"),
    _T("#    # expense report reminder on the 5th of every month"),
    _T("#    Do Expense Report,DayOfMonth=5"),
    _T("#"),
    _T("#    # daily pickup reminder MTWF at 2:30pm"),
    _T("#    Pickup the Kids from School,DayOfWeek=Monday,TimeOfDay=14:30"),
    _T("#    Pickup the Kids from School,DayOfWeek=Tuesday,TimeOfDay=14:30"),
    _T("#    Pickup the Kids from School,DayOfWeek=Wednesday,TimeOfDay=14:30"),
    _T("#    Pickup the Kids from School,DayOfWeek=Friday,TimeOfDay=14:30"),
    _T("#"),
    _T("#    # reminder on Monday to check the weekly WinQual reports"),
    _T("#    Check WinQual Reports,DayOfWeek=Monday"),
    _T("#"),
    _T("#-------------------------------------------------------------------------------"),
	_T("# CONDITIONS:"),
    _T("#-------------------------------------------------------------------------------"),
    _T("#"),
    _T("# TimeOfDay=HH:MM"),
    _T("# DayOfWeek=Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday"),
    _T("# DayOfMonth=1...31"),
	_T("# MonthOfYear=1...12"),
    _T("#"),
    _T("################################################################################")
};

Alarm::Alarm()
: ValidFieldFlags(0)
, TimeOfDayHour(0)
, TimeOfDayMinute(0)
, DayOfWeek(0)
, DayOfMonth(0)
, MonthOfYear(0)
{
}

Alarm::Alarm(const Alarm& rhs)
{
	*this = rhs;
}

Alarm::~Alarm()
{
}

Alarm& Alarm::operator=(const Alarm& rhs)
{
	if (this == &rhs)
		return *this;

	TextMessage = rhs.TextMessage;
	ValidFieldFlags = rhs.ValidFieldFlags;
	TimeOfDayHour = rhs.TimeOfDayHour;
	TimeOfDayMinute = rhs.TimeOfDayMinute;
	DayOfWeek = rhs.DayOfWeek;
	DayOfMonth = rhs.DayOfMonth;
	MonthOfYear = rhs.MonthOfYear;

	return *this;
}

void Alarm::CheckAndTrigger()
{
	if (ValidFieldFlags == 0)
		return;

	BOOL Triggered = TRUE;

	CTime now = CTime::GetCurrentTime();

	if ((ValidFieldFlags & VF_TIMEOFDAY) != 0)
	{
		if ((now.GetHour() != TimeOfDayHour) ||
			(now.GetMinute() != TimeOfDayMinute))
		{
			Triggered = FALSE;
		}
	}

	if ((ValidFieldFlags & VF_DAYOFWEEK) != 0)
	{
		if (now.GetDayOfWeek() != DayOfWeek)
			Triggered = FALSE;
	}

	if ((ValidFieldFlags & VF_DAYOFMONTH) != 0)
	{
		if (now.GetDay() != DayOfMonth)
			Triggered = FALSE;
	}

	if ((ValidFieldFlags & VF_MONTHOFYEAR) != 0)
	{
		if (now.GetMonth() != MonthOfYear)
			Triggered = FALSE;
	}

	if (Triggered)
	{
		CTimeSpan timeSinceLastTrigger = now - TimeLastTriggered;

		if ((ValidFieldFlags & VF_TIMEOFDAY) != 0)
		{
			if (timeSinceLastTrigger.GetTotalMinutes() < 2)
				Triggered = FALSE;
		}
		else if (((ValidFieldFlags & VF_DAYOFWEEK) != 0) ||
			     ((ValidFieldFlags & VF_DAYOFMONTH) != 0))
		{
			if (timeSinceLastTrigger.GetTotalHours() < 26)
				Triggered = FALSE;
		}
		else
		{
			if (timeSinceLastTrigger.GetTotalHours() < (24*33))
				Triggered = FALSE;
		}
	}

	if (Triggered)
	{
		TimeLastTriggered = now;

		TRACE1("TRIGGER: %s\n", TextMessage);

		DAlarmNotify& dlg = *new DAlarmNotify;
		dlg.TextMessage = TextMessage;
		dlg.Create(IDD_ALARMNOTIFY);
		dlg.ShowWindow(SW_SHOW);

		sndPlaySound(((CAlarmsDlg*)theApp.m_pMainWnd)->alarmsSoundFile,SND_ASYNC);
	}
}


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


// CAlarmsDlg dialog




CAlarmsDlg::CAlarmsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAlarmsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	timerTrayIconRefresh = 0;
}

void CAlarmsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAlarmsDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_MESSAGE(TRAY_MSG,&OnTrayMsg)
	ON_COMMAND(ID_EDITALARMS, &CAlarmsDlg::OnEditalarms)
	ON_COMMAND(ID_EXIT, &CAlarmsDlg::OnExit)
END_MESSAGE_MAP()

// CAlarmsDlg message handlers

BOOL CAlarmsDlg::OnInitDialog()
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
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	alarmsFile.Format(_T("%s%s"), theApp.appDir, ALARMS_FILE);
	alarmsSoundFile.Format(_T("%s%s"), theApp.appDir, ALARMS_SOUND_FILE);

	sndPlaySound(NULL,NULL);

	SetWindowPos(NULL,0,0,0,0,SWP_NOZORDER);

	OnTimerTrayIconRefresh(NIM_ADD);
	timerTrayIconRefresh = SetTimer(TIMER_TRAYICONREFRESH_ID,TIMER_TRAYICONREFRESH_INTERVAL_MS,NULL);

	OnTimerAlarmsFileReload();
	timerAlarmsFileReload = SetTimer(TIMER_ALARMSFILERELOAD_ID,TIMER_ALARMSFILERELOAD_INTERVAL_MS,NULL);

	timerTriggerAlarms = SetTimer(TIMER_TRIGGERALARMS_ID,TIMER_TRIGGERALARMS_INTERVAL_MS,NULL);

	popupMenu.LoadMenu(IDR_TRAYMENU);
	popupSubMenu = popupMenu.GetSubMenu(0);

	ModifyStyleEx(WS_EX_APPWINDOW,0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAlarmsDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAlarmsDlg::OnPaint()
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
HCURSOR CAlarmsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAlarmsDlg::OnClose()
{
	if (timerTrayIconRefresh != 0)
	{
		KillTimer(timerTrayIconRefresh);
		timerTrayIconRefresh = 0;
	}
	OnTimerTrayIconRefresh(NIM_DELETE);

	if (timerAlarmsFileReload != 0)
	{
		KillTimer(timerAlarmsFileReload);
		timerAlarmsFileReload = 0;
	}

	if (timerTriggerAlarms != 0)
	{
		KillTimer(timerTriggerAlarms);
		timerTriggerAlarms = 0;
	}

	ClearAlarms();

	PostQuitMessage(0);

	CDialog::OnClose();
}

void CAlarmsDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_TRAYICONREFRESH_ID)
		OnTimerTrayIconRefresh(NIM_ADD);
	else if (nIDEvent == TIMER_ALARMSFILERELOAD_ID)
		OnTimerAlarmsFileReload();
	else if (nIDEvent == TIMER_TRIGGERALARMS_ID)
		OnTimerTriggerAlarms();

	CDialog::OnTimer(nIDEvent);
}

void CAlarmsDlg::OnTimerTrayIconRefresh(DWORD msg)
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = GetSafeHwnd();
	nid.uID = TRAY_ICON_ID;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.uCallbackMessage = TRAY_MSG;
	nid.hIcon = m_hIcon;
	_tcscpy(nid.szTip,_T("Alarms"));
	nid.uVersion = NOTIFYICON_VERSION;

	Shell_NotifyIcon(msg,&nid);
}

void CAlarmsDlg::Tokenize(LPCTSTR inStr, LPCTSTR delim, CStringArray& toks, BOOL trimEachToken /*= TRUE*/)
{
	ASSERT(inStr != NULL);
	if (inStr == NULL)
		return;
	ASSERT(delim != NULL);
	if (delim == NULL)
		return;

	CString _inStr(inStr);

	int curPos = 0;
	CString tok = _inStr.Tokenize(delim,curPos);
	while (tok != _T(""))
	{
		if (trimEachToken)
			tok.Trim();
		toks.Add(tok);
		tok = _inStr.Tokenize(delim,curPos);
	}
}

void CAlarmsDlg::OnTimerAlarmsFileReload()
{
	CFileStatus fs;
	if (!CFile::GetStatus(alarmsFile,fs))
		return;

	if (fs.m_mtime <= alarmsFileLastModifiedTime)
		return;

	CStdioFile f;
	if (!f.Open(alarmsFile,CFile::modeRead|CFile::typeText))
		return;

	ClearAlarms();

	CString line;
	while (f.ReadString(line))
	{
		line.Trim();
		if (line.GetLength() == 0)
			continue;
		if (line[0] == _T('#'))
			continue;

		CStringArray toks;
		Tokenize(line,ALARMS_FILE_FIELD_DELIMETER,toks);

		if (toks.GetSize() < 2)
			continue;

		Alarm a;
		a.TextMessage = toks[0];

		for (INT_PTR i = 1; i < toks.GetSize(); i++)
		{
			CString tok = toks[i];

			CStringArray conditionToks;
			Tokenize(tok,ALARMS_FILE_CONDITION_DELIMETER,conditionToks);

			if (conditionToks.GetSize() != 2)
				continue;

			CString conditionType = conditionToks[0];
			CString conditionValue = conditionToks[1];

			if (conditionType.CompareNoCase(CONDITION_TYPE_TIMEOFDAY) == 0)
			{
				int spos = conditionValue.Find(CONDITION_TYPE_TIMEOFDAY_SEP);
				if ((spos > 0) && (spos < (conditionValue.GetLength()-1)))
				{
					int TimeOfDayHour = _tstol(conditionValue.Left(spos));
					int TimeOfDayMinute = _tstol(conditionValue.Right(conditionValue.GetLength()-(spos+1)));

					if ((TimeOfDayHour >= 0) && (TimeOfDayHour < 24) &&
						(TimeOfDayMinute >= 0) && (TimeOfDayMinute < 60))
					{
						a.ValidFieldFlags |= Alarm::VF_TIMEOFDAY;
						a.TimeOfDayHour = TimeOfDayHour;
						a.TimeOfDayMinute = TimeOfDayMinute;
					}
				}
			}
			else if (conditionType.CompareNoCase(CONDITION_TYPE_DAYOFWEEK) == 0)
			{
				if (conditionValue.CompareNoCase(CONDITION_TYPE_DAYOFWEEK_SUNDAY) == 0)
				{
					a.ValidFieldFlags |= Alarm::VF_DAYOFWEEK;
					a.DayOfWeek = 1;
				}
				else if (conditionValue.CompareNoCase(CONDITION_TYPE_DAYOFWEEK_MONDAY) == 0)
				{
					a.ValidFieldFlags |= Alarm::VF_DAYOFWEEK;
					a.DayOfWeek = 2;
				}
				else if (conditionValue.CompareNoCase(CONDITION_TYPE_DAYOFWEEK_TUESDAY) == 0)
				{
					a.ValidFieldFlags |= Alarm::VF_DAYOFWEEK;
					a.DayOfWeek = 3;
				}
				else if (conditionValue.CompareNoCase(CONDITION_TYPE_DAYOFWEEK_WEDNESDAY) == 0)
				{
					a.ValidFieldFlags |= Alarm::VF_DAYOFWEEK;
					a.DayOfWeek = 4;
				}
				else if (conditionValue.CompareNoCase(CONDITION_TYPE_DAYOFWEEK_THURSDAY) == 0)
				{
					a.ValidFieldFlags |= Alarm::VF_DAYOFWEEK;
					a.DayOfWeek = 5;
				}
				else if (conditionValue.CompareNoCase(CONDITION_TYPE_DAYOFWEEK_FRIDAY) == 0)
				{
					a.ValidFieldFlags |= Alarm::VF_DAYOFWEEK;
					a.DayOfWeek = 6;
				}
				else if (conditionValue.CompareNoCase(CONDITION_TYPE_DAYOFWEEK_SATURDAY) == 0)
				{
					a.ValidFieldFlags |= Alarm::VF_DAYOFWEEK;
					a.DayOfWeek = 7;
				}
			}
			else if (conditionType.CompareNoCase(CONDITION_TYPE_DAYOFMONTH) == 0)
			{
				int DayOfMonth = _tstol(conditionValue);
				if ((DayOfMonth > 0) && (DayOfMonth <= 31))
				{
					a.ValidFieldFlags |= Alarm::VF_DAYOFMONTH;
					a.DayOfMonth = DayOfMonth;
				}
			}
			else if (conditionType.CompareNoCase(CONDITION_TYPE_MONTHOFYEAR) == 0)
			{
				int MonthOfYear = _tstol(conditionValue);
				if ((MonthOfYear > 0) && (MonthOfYear <= 12))
				{
					a.ValidFieldFlags |= Alarm::VF_MONTHOFYEAR;
					a.MonthOfYear = MonthOfYear;
				}
			}
		}

		if (a.ValidFieldFlags == 0)
			continue;

		Alarms.Add(new Alarm(a));
	}

	f.Close();

	alarmsFileLastModifiedTime = fs.m_mtime;
}

void CAlarmsDlg::OnTimerTriggerAlarms()
{
	for (INT_PTR i = 0; i < Alarms.GetSize(); i++)
		Alarms[i]->CheckAndTrigger();
}

LRESULT CAlarmsDlg::OnTrayMsg(WPARAM wParam, LPARAM lParam)
{
	if ((lParam == WM_RBUTTONDOWN) || (lParam == WM_CONTEXTMENU))
	{
		POINT p;
		GetCursorPos(&p);
		SetForegroundWindow();
		popupSubMenu->TrackPopupMenuEx(TPM_LEFTALIGN|TPM_TOPALIGN,p.x,p.y,this,NULL);
	}

	return 0;
}

void CAlarmsDlg::OnEditalarms()
{
	CFileStatus fs;
	if (!CFile::GetStatus(alarmsFile,fs))
	{
		CStdioFile f;
		if (!f.Open(alarmsFile,CFile::modeCreate|CFile::modeWrite|CFile::typeText))
		{
			CString s;
			s.Format(_T("ERROR: Unable to create default Alarms.txt file\n\n%s"), alarmsFile);
			AfxMessageBox(s);
			return;
		}

		DWORD_PTR numLines = sizeof(defaultAlarmsFile) / sizeof(LPCTSTR);
		for (DWORD_PTR i = 0; i < numLines; i++)
		{
			CString s;
			s.Format(_T("%s\n"),defaultAlarmsFile[i]);
			f.WriteString(s);
		}

		f.Flush();
		f.Close();
	}

	ShellExecute(GetSafeHwnd(),_T("open"),alarmsFile,NULL,NULL,SW_SHOW);
}

void CAlarmsDlg::OnExit()
{
	PostMessage(WM_CLOSE);
}

void CAlarmsDlg::ClearAlarms()
{
	for (INT_PTR i = 0; i < Alarms.GetSize(); i++)
		delete Alarms[i];
	Alarms.RemoveAll();
}
