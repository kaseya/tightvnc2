//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
//
//  This file is part of the TightVNC software.
//
//  TightVNC is free software; you can redistribute it and/or modify
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
// TightVNC homepage on the Web: http://www.tightvnc.com/

#include "VncViewerConfigDialog.h"

#include "res/resource.h"

#include "client-config-lib/VncViewerConfig.h"
#include "client-config-lib/VncViewerSettingsManager.h"

#include "util/StringParser.h"

#include "gui/SpinControl.h"

VncViewerConfigDialog g_vncViewerConfigDialog;

VncViewerConfigDialog::VncViewerConfigDialog()
: m_historyWasCleared(false)
{
  setResourceId(IDD_GENERAL_OPTION);
}

VncViewerConfigDialog::~VncViewerConfigDialog()
{
}

bool VncViewerConfigDialog::historyWasCleared()
{
  return m_historyWasCleared;
}

BOOL VncViewerConfigDialog::onInitDialog()
{
  initControls();

  updateControlValues();

  m_historyWasCleared = false;

  return TRUE;
}

// FIXME: Stub
BOOL VncViewerConfigDialog::onNotify(UINT controlID, LPARAM data)
{
  return TRUE;
}

BOOL VncViewerConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
  switch (controlID) {
  case IDC_OK:
    onOkButtonClick();
    break;
  case IDC_CANCEL:
    onCancelButtonClick();
    break;
  case IDCANCEL:
    kill(IDCANCEL);
    break;
  case IDC_BUTTON_CLEAR_LIST:
    onClearConnectionHistoryButtonClick();
    break;
  }
  return TRUE;
}

// FIXME: Stub
BOOL VncViewerConfigDialog::onDestroy()
{
  return TRUE;
}

void VncViewerConfigDialog::onOkButtonClick()
{
  if (!isInputValid()) {
    return ;
  }

  VncViewerConfig *config = VncViewerConfig::getInstance();

  StringStorage text;
  int intVal;

  m_listenPort.getText(&text);
  StringParser::parseInt(text.getString(), &intVal);
  config->setListenPort(intVal);

  m_logLevel.getText(&text);
  StringParser::parseInt(text.getString(), &intVal);
  config->setLogLevel(intVal);

  int oldLimit = config->getHistoryLimit();
  m_historyLimit.getText(&text);
  StringParser::parseInt(text.getString(), &intVal);
  config->setHistoryLimit(intVal);

  if (config->getHistoryLimit() < oldLimit) {
    config->getConnectionHistory()->truncate();
  }

  config->showToolbar(m_showToolbar.isChecked());
  config->promptOnFullscreen(m_promptOnFullscreen.isChecked());

  SettingsManager *sm = VncViewerSettingsManager::getInstance();
  config->saveToStorage(sm);

  kill(IDOK);
}

void VncViewerConfigDialog::onClearConnectionHistoryButtonClick()
{
  VncViewerConfig::getInstance()->getConnectionHistory()->clear();
  m_historyWasCleared = true;
}

void VncViewerConfigDialog::onCancelButtonClick()
{
  kill(IDCANCEL);
}

void VncViewerConfigDialog::initControls()
{
  HWND hwnd = m_ctrlThis.getWindow();

  m_listenPort.setWindow(GetDlgItem(hwnd, IDC_LISTEN_PORT));
  m_logLevel.setWindow(GetDlgItem(hwnd, IDC_EDIT_LOG_LEVEL));
  m_historyLimit.setWindow(GetDlgItem(hwnd, IDC_EDIT_AMOUNT_LIST));

  m_showToolbar.setWindow(GetDlgItem(hwnd, IDC_CHECK_TOOLBAR));
  m_promptOnFullscreen.setWindow(GetDlgItem(hwnd, IDC_CHECK_MESSAGE));

  m_pathToLogFile.setWindow(GetDlgItem(hwnd, IDC_EDIT_LOG_FILE));

  SpinControl spin1;
  SpinControl spin2;
  SpinControl spin3;

  spin1.setWindow(GetDlgItem(hwnd, IDC_SPIN1));
  spin1.setBuddy(&m_historyLimit);
  spin1.setRange(1, 1024);

  spin2.setWindow(GetDlgItem(hwnd, IDC_SPIN2));
  spin2.setBuddy(&m_listenPort);
  spin2.setRange32(1, 65535);

  spin3.setWindow(GetDlgItem(hwnd, IDC_SPIN3));
  spin3.setBuddy(&m_logLevel);
  spin3.setRange(0, 9);
}

bool VncViewerConfigDialog::isInputValid()
{
  if (!testNum(&m_listenPort, _T("Listen port"))) {
    return false;
  }
  if (!testNum(&m_logLevel, _T("Log level"))) {
    return false;
  }
  if (!testNum(&m_historyLimit, _T("History limit"))) {
    return false;
  }
  return true;
}

bool VncViewerConfigDialog::testNum(TextBox *tb, const TCHAR *tbName)
{
  StringStorage text;
  tb->getText(&text);

  if (StringParser::tryParseInt(text.getString())) {
    return true;
  }

  StringStorage message;
  message.format(_T("Value in field '%s' must be numeric"), tbName);

  MessageBox(m_ctrlThis.getWindow(), message.getString(),
             _T("VNCViewer general config"), MB_OK | MB_ICONWARNING);

  tb->setFocus();

  return false;
}

void VncViewerConfigDialog::updateControlValues()
{
  VncViewerConfig *config = VncViewerConfig::getInstance();

  StringStorage txt;

  txt.format(_T("%d"), config->getListenPort());
  m_listenPort.setText(txt.getString());

  txt.format(_T("%d"), config->getLogLevel());
  m_logLevel.setText(txt.getString());

  txt.format(_T("%d"), config->getHistoryLimit());
  m_historyLimit.setText(txt.getString());

  m_showToolbar.check(config->isToolbarShown());
  m_promptOnFullscreen.check(config->isPromptOnFullscreenEnabled());

  m_pathToLogFile.setText(config->getPathToLogFile());
}
