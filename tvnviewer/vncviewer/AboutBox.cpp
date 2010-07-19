//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.



#include "stdhdrs.h"
#include "vncviewer.h"

// Process the About dialog.

static LRESULT CALLBACK AboutDlgProc(HWND hwnd, UINT iMsg, 
									WPARAM wParam, LPARAM lParam) 
{
	switch (iMsg) {
	case WM_INITDIALOG: 
		SetDlgItemText(hwnd, IDC_BUILDTIME, g_buildTime);
		CentreWindow(hwnd);
		return TRUE;
		
	case WM_CLOSE:
		EndDialog(hwnd, TRUE);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_VISIT_SITE:
			{
				char psz[] = "http://www.tightvnc.com/";
				ShellExecute(hwnd, "open", psz, NULL, NULL, SW_SHOWNORMAL);
			}
			break;
		case IDC_ORDER_SUPPORT:
			{
				char psz[] = "http://www.tightvnc.com/support.php";
				ShellExecute(hwnd, "open", psz, NULL, NULL, SW_SHOWNORMAL);
			}
			break;
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd, TRUE);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

void ShowAboutBox()
{
	int res = DialogBox(pApp->m_instance, 
 						DIALOG_MAKEINTRESOURCE(IDD_APP_ABOUT),
						NULL, (DLGPROC) AboutDlgProc);
}

static LRESULT CALLBACK HelpDlgProc(HWND hwnd, UINT iMsg, 
									WPARAM wParam, LPARAM lParam) 
{
	switch (iMsg) {
	case WM_INITDIALOG: 
		{
			TCHAR buf [2048];
			LoadString(pApp->m_instance, IDS_HELP, buf, sizeof(buf));
			SetDlgItemText(hwnd, IDC_EDIT_HELP, buf);
			SetWindowText(hwnd, (LPTSTR)lParam);
			CentreWindow(hwnd);
			return TRUE;
		}
	case WM_CLOSE:
		EndDialog(hwnd, TRUE);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hwnd, TRUE);
		}
	}
	return FALSE;
}

void ShowHelpBox(LPTSTR title)
{
	int res = DialogBoxParam(pApp->m_instance, 
 							 DIALOG_MAKEINTRESOURCE(IDD_HELP),
							 NULL, (DLGPROC)HelpDlgProc,
							 (LPARAM)title);
}

