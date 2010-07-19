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

#ifndef _VNC_VIEWER_CONFIG_H_
#define _VNC_VIEWER_CONFIG_H_

#include "util/StringStorage.h"
#include "util/Singleton.h"
#include "config-lib/ConnectionHistory.h"

#include "thread/LocalMutex.h"
#include "thread/AutoLock.h"

#include "ConnectionConfig.h"

class VncViewerConfig : public Singleton<VncViewerConfig>
{
public:
  VncViewerConfig();
  ~VncViewerConfig();

  bool loadFromStorage(SettingsManager *storage);

  bool saveToStorage(SettingsManager *storage);

  void setListenPort(int listenPort);
  int getListenPort();

  void setLogLevel(int logLevel);
  int getLogLevel();

  void setHistoryLimit(int historyLimit);
  int getHistoryLimit();

  void showToolbar(bool show);
  bool isToolbarShown();

  void promptOnFullscreen(bool promt);
  bool isPromptOnFullscreenEnabled();

  const TCHAR *getPathToLogFile() const;

  ConnectionHistory *getConnectionHistory();

protected:
  void createPathToLogFile();

protected:
  int m_listenPort;
  int m_logLevel;
  int m_historyLimit;
  bool m_showToolbar;
  bool m_promptOnFullscreen;
  StringStorage m_pathToLogFile;
  ConnectionHistory m_conHistory;
private:
  LocalMutex m_cs;
};

#endif
