// Copyright (C) 2008, 2009, 2010 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the TightVNC software.  Please visit our Web site:
//
//                       http://www.tightvnc.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#include "VncViewerConfig.h"

#include "win-system/Environment.h"
#include "win-system/RegistryKey.h"
#include "win-system/Registry.h"

#include "file-lib/File.h"

#define TEST_FAIL(C,R) if (!C) { R = false; }

VncViewerConfig::VncViewerConfig()
: m_logLevel(0), m_listenPort(5500), m_historyLimit(32),
  m_showToolbar(true), m_promptOnFullscreen(true),
  m_conHistory(new RegistryKey(Registry::getCurrentUserKey(),
                              _T("Software\\ORL\\VNCviewer\\History"),
                              true), m_historyLimit)
{
  createPathToLogFile();
}

VncViewerConfig::~VncViewerConfig()
{
}

bool VncViewerConfig::loadFromStorage(SettingsManager *storage)
{
  bool loadAllOk = true;

  TEST_FAIL(storage->getInt(_T("LogLevel"), &m_logLevel), loadAllOk);
  TEST_FAIL(storage->getInt(_T("ListenPort"), &m_listenPort), loadAllOk);
  TEST_FAIL(storage->getInt(_T("HistoryLimit"), &m_historyLimit), loadAllOk);

  TEST_FAIL(storage->getBoolean(_T("NoToolbar"), &m_showToolbar), loadAllOk);

  if (storage->getBoolean(_T("SkipFullScreenPrompt"), &m_promptOnFullscreen)) {
    m_promptOnFullscreen = !m_promptOnFullscreen;
  } else {
    loadAllOk = false;
  }

  return loadAllOk;
}

bool VncViewerConfig::saveToStorage(SettingsManager *storage)
{
  bool saveAllOk = true;

  TEST_FAIL(storage->setInt(_T("LogLevel"),  m_logLevel), saveAllOk);
  TEST_FAIL(storage->setInt(_T("ListenPort"), m_listenPort), saveAllOk);
  TEST_FAIL(storage->setInt(_T("HistoryLimit"), m_historyLimit), saveAllOk);
  TEST_FAIL(storage->setBoolean(_T("NoToolbar"), m_showToolbar), saveAllOk);
  TEST_FAIL(storage->setBoolean(_T("SkipFullScreenPrompt"), !m_promptOnFullscreen), saveAllOk);

  return saveAllOk;
}

void VncViewerConfig::setListenPort(int listenPort)
{
  AutoLock l(&m_cs);

  if (listenPort < 0) {
    listenPort = 0;
  } else if (listenPort > 65535) {
    listenPort = 65535;
  }

  m_listenPort = listenPort;
}

int VncViewerConfig::getListenPort()
{
  AutoLock l(&m_cs);
  return m_listenPort;
}

void VncViewerConfig::setLogLevel(int logLevel)
{
  AutoLock l(&m_cs);

  if (logLevel < 0) {
    logLevel = 0;
  } else if (logLevel > 9){
    logLevel = 9;
  }

  m_logLevel = logLevel;
}

int VncViewerConfig::getLogLevel()
{
  AutoLock l(&m_cs);
  return m_logLevel;
}

void VncViewerConfig::setHistoryLimit(int historyLimit)
{
  AutoLock l(&m_cs);

  if (historyLimit < 1) {
    historyLimit = 1;
  } else if (historyLimit > 1024) {
    historyLimit = 1024;
  }

  m_historyLimit = historyLimit;

  m_conHistory.setLimit(m_historyLimit);
}

int VncViewerConfig::getHistoryLimit()
{
  AutoLock l(&m_cs);
  return m_historyLimit;
}

void VncViewerConfig::showToolbar(bool show)
{
  AutoLock l(&m_cs);
  m_showToolbar = show;
}

bool VncViewerConfig::isToolbarShown()
{
  AutoLock l(&m_cs);
  return m_showToolbar;
}

void VncViewerConfig::promptOnFullscreen(bool prompt)
{
  AutoLock l(&m_cs);
  m_promptOnFullscreen = prompt;
}

bool VncViewerConfig::isPromptOnFullscreenEnabled()
{
  AutoLock l(&m_cs);
  return m_promptOnFullscreen;
}

const TCHAR *VncViewerConfig::getPathToLogFile() const
{
  return m_pathToLogFile.getString();
}

ConnectionHistory *VncViewerConfig::getConnectionHistory()
{
  AutoLock l(&m_cs);
  return &m_conHistory;
}

void VncViewerConfig::createPathToLogFile()
{
  StringStorage logFileFolderPath;
  StringStorage appDataPath;

  if (Environment::getSpecialFolderPath(Environment::APPLICATION_DATA_SPECIAL_FOLDER, &appDataPath)) {
    logFileFolderPath.format(_T("%s\\TightVNC"), appDataPath.getString());
  } else {
    logFileFolderPath.setString(_T("TightVNC"));
  }

  {
    File folder(logFileFolderPath.getString());
    folder.mkdir();
  }

  StringStorage logFilePath;

  unsigned int i = 0;
  bool useNumber = false;
  int maxTries = 100;

  while (true) {
    if (maxTries == 0) {
      break;
    }

    if (useNumber) {
      logFilePath.format(_T("%s\\wviewer.%d.log"), logFileFolderPath.getString(), i++);
    } else {
      logFilePath.format(_T("%s\\wviewer.log"), logFileFolderPath.getString());
    } 
    
    File logFile(logFilePath.getString());

    if (!logFile.exists()) {
      logFile.createNewFile();
    } 

    if (!logFile.canWrite()) {
      if (!useNumber) {
        useNumber = true;
      } 
      maxTries--;
      continue ;
    } 

    break;
  } 

  if (maxTries > 0) {
    AutoLock l(&m_cs);
    m_pathToLogFile = logFilePath;
  } 
}
