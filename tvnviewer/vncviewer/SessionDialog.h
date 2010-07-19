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


#ifndef SESSIONDIALOG_H__
#define SESSIONDIALOG_H__

#pragma once

#include "VNCOptions.h"
#include "VNCviewerApp.h"
#include "VNCviewerApp32.h"

#include "client-config-lib/VncViewerConfig.h"
#include "client-config-lib/ConnectionConfig.h"

#include "gui/Control.h"

#include "VncViewerConfigDialog.h"
#include "ConnectionConfigDialog.h"

class SessionDialog  
{
public:
    
	// Create a connection dialog, with the options to be
	// displayed if the options.. button is clicked.
	SessionDialog(VNCOptions *pOpt,ClientConnection *cc);
	int DoDialog();
	int m_port;
	TCHAR m_host[256];
   	virtual ~SessionDialog();
	
private:
	void updateConHistory(HWND hcombo);

	void SetConnectionProfile(bool LowBandwidth, bool HighSpeed);
	int cmp(HWND hwnd);
	ClientConnection *m_cc;
	VncViewerConfig *m_config;
	VNCOptions *m_pOpt;
	TCHAR keyname[40];
	ConnectionConfigDialog m_conConfigDialog;
	Control m_ctrlThis;

	ConnectionConfig *m_conConf;

	static BOOL CALLBACK SessDlgProc(  HWND hwndDlg,  UINT uMsg, 
		WPARAM wParam, LPARAM lParam );

	static void EnableConnectButton(HWND hDialog, BOOL bEnable);
	static void UpdateConnectButton(HWND hDialog);
};

#endif // SESSIONDIALOG_H__

