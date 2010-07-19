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
#include "VNCviewerApp.h"
#include "Exception.h"

#include "client-config-lib/VncViewerSettingsManager.h"
#include "util/FileLog.h"

// For WinCE Palm, you might want to use this for debugging, since it
// seems impossible to give the command some arguments.
// #define PALM_LOG 1

VNCviewerApp *pApp;

VNCviewerApp::VNCviewerApp(HINSTANCE hInstance, LPTSTR szCmdLine) {
	// Load vncviewer application configuration
	m_config = VncViewerConfig::getInstance();
	m_config->loadFromStorage(VncViewerSettingsManager::getInstance());

	pApp = this;
	m_instance = hInstance;

	// Logging info
	m_log = new FileLog(m_config->getPathToLogFile());
	m_log->changeLevel(m_config->getLogLevel());

	// Read the command line
	m_options.SetFromCommandLine(szCmdLine, &m_conConf);
	
	// Clear connection list
	for (int i = 0; i < MAX_CONNECTIONS; i++)
		m_clilist[i] = NULL;

	// Initialise winsock
	WORD wVersionRequested = MAKEWORD(2, 0);
	WSADATA wsaData;
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		MessageBox(NULL, _T("Error initialising sockets library"), _T("VNC info"), MB_OK | MB_ICONSTOP);
		PostQuitMessage(1);
	}
    Log::warning(_T("Started and Winsock (v %d) initialised\n"), wsaData.wVersion);
}


// The list of clients should fill up from the start and have NULLs
// afterwards.  If the first entry is a NULL, the list is empty.

void VNCviewerApp::RegisterConnection(ClientConnection *pConn) {
	omni_mutex_lock l(m_clilistMutex);
	int i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (m_clilist[i] == NULL) {
			m_clilist[i] = pConn;
			Log::warning(_T("Registered connection with app\n"));
			return;
		}
	}
	// If we've got here, something is wrong.
	Log::error(_T("Client list overflow!\n"));
	MessageBox(NULL, _T("Client list overflow!"), _T("VNC error"),
		MB_OK | MB_ICONSTOP);
	PostQuitMessage(1);

}

void VNCviewerApp::DeregisterConnection(ClientConnection *pConn) {
	omni_mutex_lock l(m_clilistMutex);
	int i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (m_clilist[i] == pConn) {
			// shuffle everything above downwards
			for (int j = i; m_clilist[j] &&	j < MAX_CONNECTIONS-1 ; j++)
				m_clilist[j] = m_clilist[j+1];
			m_clilist[MAX_CONNECTIONS-1] = NULL;
			Log::warning(_T("Deregistered connection from app\n"));

			// No clients left? then we should finish, unless we're in
			// listening mode.
			if ((m_clilist[0] == NULL) && (!pApp->m_options.m_listening)){
				PostQuitMessage(0);}

			return;
		}
	}
	// If we've got here, something is wrong.
	Log::interror(_T("Client not found for deregistering!\n"));
	PostQuitMessage(1);
}

// ----------------------------------------------


VNCviewerApp::~VNCviewerApp() {
	// Clean up winsock
	WSACleanup();
	
	Log::warning(_T("VNC viewer closing down\n"));

}
