// Alarms.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Alarms.h"
#include "AlarmsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAlarmsApp

BEGIN_MESSAGE_MAP(CAlarmsApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAlarmsApp construction

CAlarmsApp::CAlarmsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CAlarmsApp object

CAlarmsApp theApp;


// CAlarmsApp initialization

BOOL CAlarmsApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Alarms"));

	TCHAR appPathBuf[MAX_PATH+1];
	GetModuleFileName(NULL,appPathBuf,MAX_PATH);
	_TCHAR* bs = _tcsrchr(appPathBuf,_T('\\'));
	if (bs != NULL)
		*bs = _T('\0');
	_tcscat(appPathBuf,_T("\\"));
	appDir = appPathBuf;

	CAlarmsDlg& dlg = *new CAlarmsDlg;
	m_pMainWnd = &dlg;
//	dlg.Create(IDD_ALARMS_DIALOG);
//	dlg.ShowWindow(SW_SHOW);
//	return TRUE;
	dlg.DoModal();
	delete &dlg;
	return FALSE;
}
