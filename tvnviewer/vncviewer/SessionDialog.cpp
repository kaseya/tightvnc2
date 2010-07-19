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


// SessionDialog.cpp: implementation of the SessionDialog class.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "SessionDialog.h"
#include "Exception.h"
#include "Htmlhelp.h"
#include "VncViewerConfigDialog.h"
#include "client-config-lib/ConnectionConfigSM.h"

SessionDialog::SessionDialog(VNCOptions *pOpt,ClientConnection *cc)
{
	m_pOpt = pOpt;
	m_cc = cc;
	m_config = VncViewerConfig::getInstance();
	m_conConf = &cc->m_conConf;
}

SessionDialog::~SessionDialog()
{
}

// It's exceedingly unlikely, but possible, that if two modal dialogs were
// closed at the same time, the static variables used for transfer between 
// window procedure and this method could overwrite each other.
int SessionDialog::DoDialog()
{
 	return DialogBoxParam(pApp->m_instance, DIALOG_MAKEINTRESOURCE(IDD_SESSION_DLG), 
							NULL, (DLGPROC) SessDlgProc, (LONG) this);
}

BOOL CALLBACK SessionDialog::SessDlgProc(  HWND hwnd,  UINT uMsg,  WPARAM wParam, LPARAM lParam ) 
{
	// This is a static method, so we don't know which instantiation we're 
	// dealing with. But we can get a pseudo-this from the parameter to 
	// WM_INITDIALOG, which we therafter store with the window and retrieve
	// as follows:
	SessionDialog *_this = (SessionDialog *) GetWindowLong(hwnd, GWL_USERDATA);
	TCHAR tmphost[256];
	TCHAR buffer[256];
	HWND hCustomRadio = GetDlgItem(hwnd, IDC_CUSTOM_RADIO);
	HWND hModemRadio = GetDlgItem(hwnd, IDC_MODEM_RADIO);
	HWND hLocNetRadio = GetDlgItem(hwnd, IDC_LOC_NET_RADIO);
	HWND hListMode = GetDlgItem(hwnd, IDC_LIST_MODE);
	HWND hcombo = GetDlgItem(hwnd, IDC_HOSTNAME_EDIT);

	switch (uMsg) {
	case WM_INITDIALOG: 
		{			
            SetWindowLong(hwnd, GWL_USERDATA, lParam);
            SessionDialog *_this = (SessionDialog *) lParam;
            CentreWindow(hwnd);
			_this->m_cc->m_hSess = hwnd;
            _this->m_ctrlThis.setWindow(hwnd);

			_this->updateConHistory(hcombo);

			if (_this->m_pOpt->m_display[0] == '\0') {
				SendMessage(hcombo, CB_SETCURSEL, 0, 0);
				LRESULT r = SendMessage(hcombo, CB_GETLBTEXTLEN, 0, 0);
				if (r > 1 && r <= 256) {
					r = SendMessage(hcombo, CB_GETLBTEXT, 0, (LPARAM)buffer);
					// Load connection config
					if (r > 1) {
						ConnectionConfigSM ccsm(&buffer[0]);
						_this->m_conConf->loadFromStorage(&ccsm);
					}
				}
			} else {
				SetDlgItemText(hwnd, IDC_HOSTNAME_EDIT, _this->m_pOpt->m_display);
			}
			_this->cmp(hwnd);

			UpdateConnectButton(hwnd);
			SetFocus(hcombo);
            return TRUE;
		}
	case WM_HELP: 
		help.Popup(lParam);
		return 0;		
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
		if ((pApp->m_options.m_listening) || 
			(FindWindow("VNCviewer Daemon", 0) != NULL)) {
			EnableWindow( hListMode, FALSE);
		}
		if((!pApp->m_options.m_listening) && 
			(FindWindow("VNCviewer Daemon", 0) == NULL)) {
			EnableWindow( hListMode, TRUE);
		}
		return 0;		
	case WM_COMMAND:		
		switch (LOWORD(wParam)) {
		case IDC_HOSTNAME_EDIT:
			switch (HIWORD(wParam)) {
			case CBN_SELENDOK:
				{
					int a = (int)SendMessage(hcombo, CB_GETCURSEL, 0, 0L);
					SendMessage(hcombo, CB_GETLBTEXT, a, (LPARAM)(int FAR*)buffer );

					// Load connection config
					ConnectionConfigSM ccsm(&buffer[0]);
					_this->m_conConf->loadFromStorage(&ccsm);

					_this->cmp(hwnd);

					EnableConnectButton(hwnd, TRUE);
					SetFocus(hcombo);
				}
				break;
			case CBN_EDITCHANGE:
				UpdateConnectButton(hwnd);
				break;
			}
			return TRUE;
		case IDC_LOAD:
			{
				TCHAR buf[80];
				buf[0]='\0';
				if (_this->m_cc->LoadConnection(buf, true) != -1) {
					FormatDisplay(_this->m_cc->m_port,
							_this->m_pOpt->m_display,
							_this->m_cc->m_host);
					SetDlgItemText(hwnd, IDC_HOSTNAME_EDIT,
									_this->m_pOpt->m_display);
					_this->cmp(hwnd);
				}
				UpdateConnectButton(hwnd);
				SetFocus(hcombo);
				return TRUE;
			}
		case IDC_LIST_MODE: 
			// Load connection config
			{
				ConnectionConfigSM ccsm(_T(".listen"));
				pApp->m_conConf.loadFromStorage(&ccsm);
			}

			pApp->m_options.m_listening=true;
			pApp->ListenMode();
			_this->m_conConfigDialog.kill(IDCANCEL);
			EndDialog(hwnd, FALSE);
			return TRUE; 				
		case IDC_OK:             
			TCHAR display[256];			
			GetDlgItemText(hwnd, IDC_HOSTNAME_EDIT, display, 256);			 
			if (_tcslen(display) == 0) {				
				return TRUE;
			}
			if (!ParseDisplay(display, tmphost, 255, &_this->m_cc->m_port)) {
				MessageBox(NULL, 
						_T("Invalid VNC server specified.\n\r")
						_T("Server should be of the form host:display."), 
						_T("Connection setup"), MB_OK | MB_ICONEXCLAMATION );
				return TRUE;
			} else {
				_tcscpy(_this->m_cc->m_host, tmphost);
				_tcscpy(_this->m_pOpt->m_display, display);
			}
			
			_this->m_conConfigDialog.kill(IDCANCEL);
			EndDialog(hwnd, TRUE);

			return TRUE;						
		case IDCANCEL:
			_this->m_conConfigDialog.kill(IDCANCEL);
			EndDialog(hwnd, FALSE);
			return TRUE;
		case IDC_VNCVIEWER_OPTIONS_BUTTON:
			{
			VncViewerConfigDialog *dialog = &g_vncViewerConfigDialog;
			dialog->setParent(&_this->m_ctrlThis);
			if (dialog->showModal() == IDOK) {
				if (dialog->historyWasCleared()) {
					_this->updateConHistory(GetDlgItem(hwnd, IDC_HOSTNAME_EDIT));
				}
			}
			}
			return TRUE;
		case IDC_LOC_NET_RADIO:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				// FIXME: Eliminate code duplication, see below
				// (handle connection profiles in one place, in VNCOptions?)
				_this->SetConnectionProfile(false, true);
				return TRUE;
			}
			return TRUE;
		case IDC_MODEM_RADIO:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				// FIXME: Eliminate code duplication, see above and below.
				_this->SetConnectionProfile(true, false);
				return TRUE;
			}
			return TRUE;
		case IDC_CUSTOM_RADIO:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				// FIXME: Eliminate code duplication, see above.
				_this->SetConnectionProfile(false, false);
				return TRUE;
			}
			return TRUE;
		case IDC_OPTIONBUTTON:
			{
				_this->m_conConfigDialog.setConnectionConfig(_this->m_conConf);
				_this->m_conConfigDialog.showModal();

				HWND hOptionButton = GetDlgItem(hwnd, IDC_OPTIONBUTTON);
				GetDlgItemText(hwnd, IDC_HOSTNAME_EDIT, 
								_this->m_pOpt->m_display, 256);

				_this->updateConHistory(hcombo);

				SetDlgItemText(hwnd, IDC_HOSTNAME_EDIT, _this->m_pOpt->m_display); 				
				_this->cmp(hwnd);									
				SetFocus(hOptionButton);
				return TRUE;
			}
		}			
	case WM_DESTROY:
		EndDialog(hwnd, FALSE);
		return TRUE;
	}	
	return 0;
}

void SessionDialog::EnableConnectButton(HWND hDialog, BOOL bEnable)
{
	HWND hConnectButton = GetDlgItem(hDialog, IDC_OK);
	EnableWindow(hConnectButton, bEnable);
}

void SessionDialog::UpdateConnectButton(HWND hDialog)
{
	HWND hHostComboBox = GetDlgItem(hDialog, IDC_HOSTNAME_EDIT);
	BOOL hostNotEmpty = (GetWindowTextLength(hHostComboBox) != 0);
	EnableConnectButton(hDialog, hostNotEmpty);
}

int SessionDialog::cmp(HWND hwnd)
{
	int a=1;
	HWND hCustomRadio = GetDlgItem(hwnd, IDC_CUSTOM_RADIO);
	HWND hModemRadio = GetDlgItem(hwnd, IDC_MODEM_RADIO);
	HWND hLocNetRadio = GetDlgItem(hwnd, IDC_LOC_NET_RADIO);
	int i;
	for (i = rfbEncodingRaw; i <= LASTENCODING; i++)
		if ((m_conConf->isEncodingAllowed(i) != true) && (i != 3)) a = 0;
	if (m_conConf->isEncodingAllowed(i) != false) a = 0;
	if (m_conConf->getPreferredEncoding() != rfbEncodingTight) a = 0;
	if (m_conConf->isUsing8BitColor()!= true) a = 0;
	if (m_conConf->isCustomCompressionEnabled() != true) {
		a = 0;
	} else {
		if (m_conConf->getCustomCompressionLevel() != 6) a = 0;
	}
	if (a == 1) {
		SendMessage(hModemRadio, BM_CLICK, 0, 0);
		return a;
	}

	a = 2;
	for (i = rfbEncodingRaw; i <= LASTENCODING; i++)
		if ((m_conConf->isEncodingAllowed(i) != true) && (i != 3)) a = 0;
	if (m_conConf->isEncodingAllowed(i) != false) a = 0;
	if (m_conConf->isUsing8BitColor()!= false) a = 0;
	if (m_conConf->getPreferredEncoding() != rfbEncodingHextile) a = 0;
		
	if (a == 2) {
		SendMessage(hLocNetRadio, BM_CLICK, 0, 0);
		return a;
	}

	a = 3;
	for (i = rfbEncodingRaw; i <= LASTENCODING; i++)
		if ((m_conConf->isEncodingAllowed(i) != true) && (i != 3)) a = 0;
	if (m_conConf->isEncodingAllowed(i) != false) a = 0;
	if (m_conConf->getPreferredEncoding() != rfbEncodingTight) a = 0;
	if (m_conConf->isCustomCompressionEnabled() != false) a = 0;
	if (m_conConf->isUsing8BitColor()!= false) a = 0;
	if (m_conConf->isJpegCompressionEnabled() != true) {
		a = 0;
	} else {
		if (m_conConf->getJpegCompressionLevel() != 6) a = 0;
	}
	if (a == 3) {
		SendMessage(hCustomRadio, BM_CLICK, 0, 0);
		return a;
	}
	if (a == 0) {
		SendMessage(hLocNetRadio, BM_SETCHECK, 0, 0L);
		SendMessage(hModemRadio, BM_SETCHECK, 0, 0L);
		SendMessage(hCustomRadio, BM_SETCHECK, 0, 0L);
		return a;
	}
	return a;
}

void SessionDialog::updateConHistory(HWND hcombo)
{
  ComboBox cb;
  cb.setWindow(hcombo);

  StringStorage text;
  cb.getText(&text);

  cb.removeAllItems();

  // Load connection history from registry
  // and inserts hosts to combobox.
  ConnectionHistory *conHistory = m_config->getConnectionHistory();
  conHistory->load();
  for (size_t i = 0; i < conHistory->getHostCount(); i++) {
    cb.insertItem(i, conHistory->getHost(i));
  }
  cb.setText(text.getString());
}

void SessionDialog::SetConnectionProfile(bool LowBandwidth, bool HighSpeed) 
{
	for (int i = rfbEncodingRaw; i <= LASTENCODING; i++)
		m_conConf->allowEncoding(i, true);

	m_conConf->allowEncoding(3, false);
	if (!LowBandwidth && !HighSpeed) {
		m_conConf->setPreferredEncoding(rfbEncodingTight);
		m_conConf->disableCustomCompression();
		m_conConf->setJpegCompressionLevel(6);
		m_conConf->use8BitColor(false);
	}
	if (LowBandwidth && !HighSpeed) {
		m_conConf->setPreferredEncoding(rfbEncodingTight);
		m_conConf->setCustomCompressionLevel(6);
		m_conConf->use8BitColor(true);
	}
	if (!LowBandwidth && HighSpeed) {
		m_conConf->setPreferredEncoding(rfbEncodingHextile);
		m_conConf->use8BitColor(false);
	}
}
	
	
	


