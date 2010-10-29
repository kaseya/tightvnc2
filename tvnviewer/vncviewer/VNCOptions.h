//  Copyright (C) 2000 Tridia Corporation All Rights Reserved.
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


#ifndef VNCOPTIONS_H__
#define VNCOPTIONS_H__

#pragma once

// The idea here was to use a slider control.  However, that seems
// to require MFC, which has not been used up to this point.  I 
// wonder why not?  WinCE compatibility maybe?
// #include <afxcmn.h>

#include "client-config-lib/VncViewerConfig.h"
#include "util/Log.h"

#define LASTENCODING rfbEncodingZlibHex

#define NOCURSOR 0
#define DOTCURSOR 1
#define NORMALCURSOR 2
#define SMALLCURSOR 3
#define MAX_LEN_COMBO 7

#define KEY_VNCVIEWER_HISTORI _T("Software\\ORL\\VNCviewer\\History")

class VNCOptions  
{
public:
	VNCOptions();
	VNCOptions& operator=(VNCOptions& s);
	virtual ~VNCOptions();

	// process options
	bool	m_listening;
	TCHAR	m_display[256];
	bool	m_connectionSpecified;
	bool	m_configSpecified;
	TCHAR   m_configFilename[_MAX_PATH];

	// Keyboard can be specified on command line as 8-digit hex
	TCHAR	m_kbdname[9];
	bool	m_kbdSpecified;

	TCHAR   m_host[256];
	int     m_port;

	// for debugging purposes
	int m_delay;

	void Register();
#ifdef UNDER_CE
	// WinCE screen format for dialogs (Palm vs HPC)
	int	m_palmpc;
	// Use slow GDI rendering, but more accurate colours.
	int m_slowgdi;
#endif

	void SetFromCommandLine(LPTSTR szCmdLine, ConnectionConfig *conConf);

private:
	// Just for temporary use
	bool m_running;

private:
	VncViewerConfig *m_config;
};

#endif // VNCOPTIONS_H__

