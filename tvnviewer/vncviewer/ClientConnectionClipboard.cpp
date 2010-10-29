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
// whence you received this file, check http://www.uk.research.att.com/vnc or 
// contact the authors on vnc@uk.research.att.com for information on obtaining it.
//

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"
#include "Exception.h"

// This file contains the code for getting text from, and putting text into
// the Windows clipboard.

//
// ProcessClipboardChange
// Called by ClientConnection::WndProc.
// We've been informed that the local clipboard has been updated.
// If it's text we want to send it to the server.
//

void ClientConnection::ProcessLocalClipboardChange()
{
	Log::warning(_T("Clipboard changed\n"));
	
	HWND hOwner = GetClipboardOwner();
	if (hOwner == m_hwnd) {
		Log::warning(_T("We changed it - ignore!\n"));
	} else if (!m_initialClipboardSeen) {
		Log::warning(_T("Don't send initial clipboard!\n"));
		m_initialClipboardSeen = true;
	} else if (m_conConf.isClipboardEnabled()) {
		
		// The clipboard should not be modified by more than one thread at once
		omni_mutex_lock l(m_clipMutex);
		
		if (OpenClipboard(m_hwnd)) { 
			HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
			if (hglb == NULL) {
				CloseClipboard();
			} else {
				WCHAR *str = (WCHAR *)GlobalLock(hglb);
				StringStorage text;
				text.fromUnicodeString(str);
				GlobalUnlock(hglb);
				CloseClipboard();

				size_t bufSize = text.getLength() + 1;
				char *contents = new char[bufSize];
				text.toAnsiString(contents, bufSize);

				// Translate to Unix-format lines before sending
				char *unixcontents = new char[bufSize];
				int j = 0;
				for (int i = 0; contents[i] != '\0'; i++) {
					if (contents[i] != '\x0d') {
						unixcontents[j++] = contents[i];
					}
				}
				unixcontents[j] = '\0';
				try {
					SendClientCutText(unixcontents, strlen(unixcontents));
				} catch (WarningException &e) {
					Log::interror(_T("VncViewerException while sending clipboard text : %s\n"), e.m_info);
					DestroyWindow(m_hwnd1);
				}
				delete [] contents; 
				delete [] unixcontents;
			}
		}
	}
	// Pass the message to the next window in clipboard viewer chain
	::SendMessage(m_hwndNextViewer, WM_DRAWCLIPBOARD , 0,0); 
}

// We've read some text from the remote server, and
// we need to copy it into the local clipboard.
// Called by ClientConnection::ReadServerCutText()

void ClientConnection::UpdateLocalClipboard(char *buf, size_t len) {
	
	if (!m_conConf.isClipboardEnabled())
		return;

	// Copy to wincontents replacing LF with CR-LF
	char *wincontents = new char[len * 2 + 1];
	int j = 0;
	for (int i = 0; m_netbuf[i] != 0; i++, j++) {
        if (buf[i] == '\x0a') {
			wincontents[j++] = '\x0d';
            len++;
        }
		wincontents[j] = buf[i];
	}
	wincontents[j] = '\0';

    // The clipboard should not be modified by more than one thread at once
    {
        omni_mutex_lock l(m_clipMutex);

        if (!OpenClipboard(m_hwnd)) {
	        throw WarningException("Failed to open clipboard\n");
        }
        if (! ::EmptyClipboard()) {
	        throw WarningException("Failed to empty clipboard\n");
        }

#ifndef _UNICODE
        // Store locale information in the clipboard.
        HGLOBAL hmemLocale = GlobalAlloc(GMEM_MOVEABLE, sizeof(LCID));
        if (hmemLocale != NULL) {
            LCID *pLocale = (LCID *)GlobalLock(hmemLocale);
            *pLocale = GetUserDefaultLCID();
            GlobalUnlock(hmemLocale);
            if (SetClipboardData(CF_LOCALE, hmemLocale) == NULL) {
                GlobalFree(hmemLocale);
            }
        }
#endif

        // Allocate a global memory object for the text. 
        HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len +1) * sizeof(TCHAR));
        if (hglbCopy != NULL) { 
	        // Lock the handle and copy the text to the buffer.  
	        LPTSTR lptstrCopy = (LPTSTR) GlobalLock(hglbCopy); 
	        memcpy(lptstrCopy, wincontents, len * sizeof(TCHAR)); 
	        lptstrCopy[len] = (TCHAR) 0;    // null character 
	        GlobalUnlock(hglbCopy);          // Place the handle on the clipboard.  
#ifndef _UNICODE
            SetClipboardData(CF_TEXT, hglbCopy);
#else
            SetClipboardData(CF_UNICODETEXT, hglbCopy);
#endif
        }

        delete [] wincontents;

        if (! ::CloseClipboard()) {
	        throw WarningException("Failed to close clipboard\n");
        }
    }
}
