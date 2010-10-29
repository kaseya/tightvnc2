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


// VNCOptions.cpp: implementation of the VNCOptions class.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "VNCOptions.h"
#include "Exception.h"
#include "Htmlhelp.h"
#include "commctrl.h"
#include "AboutBox.h"

#include "config-lib/IniFileSettingsManager.h"
#include "config-lib/RegistrySettingsManager.h"

#include "client-config-lib/VncViewerSettingsManager.h"
#include "client-config-lib/ConnectionConfigSM.h"

VNCOptions::VNCOptions()
{
	m_config = VncViewerConfig::getInstance();

	m_display[0] = '\0';
	m_host[0] = '\0';
	m_port = -1;
	m_kbdname[0] = '\0';
	m_kbdSpecified = false;
	
	m_delay=0;
	m_connectionSpecified = false;
	m_configSpecified = false;
	m_configFilename[0] = '\0';
	m_listening = false;

#ifdef UNDER_CE
	m_palmpc = false;
	
	// Check for PalmPC aspect 
	HDC temp_hdc = GetDC(NULL);
	int screen_width = GetDeviceCaps(temp_hdc, HORZRES);
	if (screen_width < 320)
	{
		m_palmpc = true;
	}
	ReleaseDC(NULL,temp_hdc);

	m_slowgdi = false;
#endif
}

VNCOptions& VNCOptions::operator=(VNCOptions& s)
{
	strcpy(m_display, s.m_display);
	strcpy(m_host, s.m_host);
	m_port				= s.m_port;

	strcpy(m_kbdname, s.m_kbdname);
	m_kbdSpecified		= s.m_kbdSpecified;
	
	m_delay				= s.m_delay;
	m_connectionSpecified = s.m_connectionSpecified;
	m_configSpecified   = s.m_configSpecified;
	strcpy(m_configFilename, s.m_configFilename);

	m_listening			= s.m_listening;

#ifdef UNDER_CE
	m_palmpc			= s.m_palmpc;
	m_slowgdi			= s.m_slowgdi;
#endif
	return *this;
}

VNCOptions::~VNCOptions()
{
}

inline bool SwitchMatch(LPCTSTR arg, LPCTSTR swtch) {
	return (arg[0] == '-' || arg[0] == '/') &&
		(_tcsicmp(&arg[1], swtch) == 0);
}

static void ArgError(LPTSTR msg) {
    MessageBox(NULL,  msg, _T("Argument error"),MB_OK | MB_TOPMOST | MB_ICONSTOP);
}

void VNCOptions::SetFromCommandLine(LPTSTR szCmdLine, ConnectionConfig *conConf) {
	// We assume no quoting here.
	// Copy the command line - we don't know what might happen to the original
	int cmdlinelen = _tcslen(szCmdLine);
	
	if (cmdlinelen == 0) return;
		
	TCHAR CommLine[256] ;
	int f = 0;
	_tcscpy(CommLine, szCmdLine);

	if (_tcsstr( CommLine, "/listen") != NULL ||
        _tcsstr( CommLine, "-listen") != NULL) {
		// Load connection config
		ConnectionConfigSM ccsm(_T(".listen"));
		conConf->loadFromStorage(&ccsm);

		f = 1;
	}
	TCHAR *cmd = new TCHAR[cmdlinelen + 1];
	_tcscpy(cmd, szCmdLine);
	
	// Count the number of spaces
	// This may be more than the number of arguments, but that doesn't matter.
	int nspaces = 0;
	TCHAR *p = cmd;
	TCHAR *pos = cmd;
	while ( ( pos = _tcschr(p, ' ') ) != NULL ) {
		nspaces ++;
		p = pos + 1;
	}
	
	// Create the array to hold pointers to each bit of string
	TCHAR **args = new LPTSTR[nspaces + 1];
	
	// replace spaces with nulls and
	// create an array of TCHAR*'s which points to start of each bit.
	pos = cmd;
	int i = 0;
	args[i] = cmd;
	bool inquote=false;
	for (pos = cmd; *pos != 0; pos++) {
		// Arguments are normally separated by spaces, unless there's quoting
		if ((*pos == ' ') && !inquote) {
			*pos = '\0';
			p = pos + 1;
			args[++i] = p;
		}
		if (*pos == '"') {  
			if (!inquote) {      // Are we starting a quoted argument?
				args[i] = ++pos; // It starts just after the quote
			} else {
				*pos = '\0';     // Finish a quoted argument?
			}
			inquote = !inquote;
		}
	}
	i++;
	int j;
	for (j = 0; j < i; j++) {
		TCHAR phost[256];
		if (ParseDisplay(args[j], phost, 255, &m_port) && (f == 0) &&
			_tcsstr( args[j], "/") == NULL && args[j][0] != '-') {
			StringStorage keyName;
			keyName.format(_T("%s\\%s"), _T("Software\\ORL\\VNCviewer\\History"), args[j]);
			RegistryKey regKey(HKEY_CURRENT_USER, keyName.getString(), false);
			if (regKey.isOpened()) {
				ConnectionConfigSM ccsm(args[j]);
				conConf->loadFromStorage(&ccsm);
			}
		}
	}
	bool hostGiven = false, portGiven = false;
	// take in order.
	for (j = 0; j < i; j++) {
		if ( SwitchMatch(args[j], _T("help")) ||
			SwitchMatch(args[j], _T("?")) ||
			SwitchMatch(args[j], _T("h"))) {
			ShowHelpBox(_T("TightVNC Usage Help"));
			exit(1);
		} else if ( SwitchMatch(args[j], _T("listen"))) {
			m_listening = true;
			if (j+1 < i && args[j+1][0] >= '0' && args[j+1][0] <= '9') {
				int listenPort;
				if (_stscanf(args[j+1], _T("%d"), &listenPort) != 1) {
					ArgError(_T("Invalid listen port specified"));
					continue;
				} else {
					m_config->setListenPort(listenPort);
				}
				j++;
			}
		} else if ( SwitchMatch(args[j], _T("config") )) {
			if (++j == i) {
				ArgError(_T("No config file specified"));
				continue;
			}
			// The GetPrivateProfile* stuff seems not to like some relative paths
			_fullpath(m_configFilename, args[j], _MAX_PATH);
			if (_access(m_configFilename, 04)) {
				ArgError(_T("Can't open specified config file for reading."));
				continue;
			} else {
				IniFileSettingsManager ifsm(&m_configFilename[0], _T("options"));
				conConf->loadFromStorage(&ifsm);
				m_configSpecified = true;
			}
		} else if ( SwitchMatch(args[j], _T("register") )) {
			Register();
			exit(1);
		} else if (args[j][0] == '-' || args[j][0] == '/') {
			ArgError(_T("Invalid command-line option.\n"
						"Please run \"vncviewer -help\" for usage help."));
			exit(1);
		} else {
			TCHAR phost[256];
			if (!ParseDisplay(args[j], phost, 255, &m_port)) {
				ArgError(_T("Invalid VNC server specified."));
				continue;
			} else {
				_tcscpy(m_host, phost);
				_tcscpy(m_display, args[j]);
				m_connectionSpecified = true;
			}
		}
	}

	// tidy up
	delete [] cmd;
	delete [] args;
}

// Record the path to the VNC viewer and the type
// of the .vnc files in the registry
void VNCOptions::Register()
{
	char keybuf[_MAX_PATH * 2 + 20];
	HKEY hKey, hKey2;
	if ( RegCreateKey(HKEY_CLASSES_ROOT, ".vnc", &hKey)  == ERROR_SUCCESS ) {
		RegSetValue(hKey, NULL, REG_SZ, "VncViewer.Config", 0);
		RegCloseKey(hKey);
	} else {
		Log::interror("Failed to register .vnc extension\n");
	}

	char filename[_MAX_PATH];
	if (GetModuleFileName(NULL, filename, _MAX_PATH) == 0) {
		Log::interror("Error getting vncviewer filename\n");
		return;
	}
	Log::warning("Viewer is %s\n", filename);

	if ( RegCreateKey(HKEY_CLASSES_ROOT, "VncViewer.Config", &hKey)  == ERROR_SUCCESS ) {
		RegSetValue(hKey, NULL, REG_SZ, "VNCviewer Config File", 0);
		
		if ( RegCreateKey(hKey, "DefaultIcon", &hKey2)  == ERROR_SUCCESS ) {
			sprintf(keybuf, "%s,0", filename);
			RegSetValue(hKey2, NULL, REG_SZ, keybuf, 0);
			RegCloseKey(hKey2);
		}
		if ( RegCreateKey(hKey, "Shell\\open\\command", &hKey2)  == ERROR_SUCCESS ) {
			sprintf(keybuf, "\"%s\" -config \"%%1\"", filename);
			RegSetValue(hKey2, NULL, REG_SZ, keybuf, 0);
			RegCloseKey(hKey2);
		}

		RegCloseKey(hKey);
	}

	if ( RegCreateKey(HKEY_LOCAL_MACHINE, 
			"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\vncviewer.exe", 
			&hKey)  == ERROR_SUCCESS ) {
		RegSetValue(hKey, NULL, REG_SZ, filename, 0);
		RegCloseKey(hKey);
	}
}
