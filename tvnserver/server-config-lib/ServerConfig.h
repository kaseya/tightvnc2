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

#ifndef _SERVER_CONFIG_H_
#define _SERVER_CONFIG_H_

#include "util/StringVector.h"
#include "util/Exception.h"
#include "PortMappingContainer.h"
#include "IpAccessControl.h"
#include "thread/AutoLock.h"
#include "thread/LocalMutex.h"
#include "IpAccessRule.h"
#include "io-lib/DataInputStream.h"
#include "io-lib/DataOutputStream.h"
#include "io-lib/IOException.h"

#include <shlobj.h>

#define VNC_PASSWORD_SIZE 8

class ServerConfig : public Lockable
{
public:
  static const unsigned int MINIMAL_POLLING_INTERVAL = 30;
  static const unsigned int MINIMAL_LOCAL_INPUT_PRIORITY_TIMEOUT = 1;
  static const unsigned int MINIMAL_QUERY_TIMEOUT = 1;

  enum DisconnectAction {
    DA_DO_NOTHING = 0,
    DA_LOCK_WORKSTATION = 1,
    DA_LOGOUT_WORKSTATION = 2,
    DA_STOP_AND_REMOVE_SERVICE = 10
  };

public:
  ServerConfig();
  virtual ~ServerConfig();

  void serialize(DataOutputStream *output) throw(Exception);

  void deserialize(DataInputStream *input) throw(Exception);

  virtual void lock() {
    m_objectCS.lock();
  }

  virtual void unlock() {
    m_objectCS.unlock();
  }

  bool isControlAuthEnabled();
  void useControlAuth(bool useAuth);

  void setRfbPort(int port);
  int getRfbPort();

  void setHttpPort(int port);
  int getHttpPort();

  bool isBlankScreenEnabled();
  void enableBlankScreen(bool enabled);

  void enableFileTransfers(bool enabled);
  bool isFileTransfersEnabled();

  void enableRemovingDesktopWallpaper(bool enabled);
  bool isRemovingDesktopWallpaperEnabled();

  void setDisconnectAction(DisconnectAction action);
  DisconnectAction getDisconnectAction();

  bool isAcceptingRfbConnections();
  void acceptRfbConnections(bool accept);

  void getPrimaryPassword(unsigned char *password);
  void setPrimaryPassword(const unsigned char *value);

  void getReadOnlyPassword(unsigned char *password);
  void setReadOnlyPassword(const unsigned char *value);

  void getControlPassword(unsigned char *password);
  void setControlPassword(const unsigned char *password);

  bool hasPrimaryPassword();
  bool hasReadOnlyPassword();
  bool hasControlPassword();

  void deletePrimaryPassword();
  void deleteReadOnlyPassword();
  void deleteControlPassword();

  unsigned int getIdleTimeout();

  void setIdleTimeout(unsigned int idleTimeout);

  bool isUsingAuthentication();

  void useAuthentication(bool enabled);

  bool isOnlyLoopbackConnectionsAllowed();

  void acceptOnlyLoopbackConnections(bool enabled);

  bool isAcceptingHttpConnections();

  void acceptHttpConnections(bool accept);

  bool isAppletParamInUrlEnabled();

  void enableAppletParamInUrl(bool enabled);

  int getLogLevel();

  void setLogLevel(int logLevel);

  bool isAlwaysShared();

  bool isNeverShared();

  bool isDisconnectingExistingClients();

  void setAlwaysShared(bool enabled);

  void setNeverShared(bool enabled);

  void disconnectExistingClients(bool disconnectExisting);

  void setPollingInterval(unsigned int interval);

  unsigned int getPollingInterval();

  void blockRemoteInput(bool blockEnabled);

  bool isBlockingRemoteInput();

  void setLocalInputPriority(bool localPriority);

  bool isLocalInputPriorityEnabled();

  unsigned int getLocalInputPriorityTimeout();

  void setLocalInputPriorityTimeout(unsigned int value);

  void blockLocalInput(bool enabled);

  bool isBlockingLocalInput();

  unsigned int getQueryTimeout();

  void setQueryTimeout(unsigned int timeout);

  bool isDefaultActionAccept();
  void setDefaultActionToAccept(bool accept);

  PortMappingContainer *getPortMappingContainer();

  IpAccessControl *getAccessControl();

  IpAccessRule::ActionType getActionByAddress(unsigned long ip);

  void allowLoopbackConnections(bool allow);

  bool isLoopbackConnectionsAllowed();

  void disableTrayIcon(bool disabled);

  bool isTrayIconDisabled();

  StringVector *getVideoClassNames();

  unsigned int getVideoRecognitionInterval();

  void setVideoRecognitionInterval(unsigned int interval);

  void saveLogToAllUsersPath(bool enabled);
  bool isSaveLogToAllUsersPathFlagEnabled();

  void setGrabTransparentWindowsFlag(bool grab);
  bool getGrabTransparentWindowsFlag();

  bool getShowTrayIconFlag();
  void setShowTrayIconFlag(bool val);

  void getLogFilePath(StringStorage *logFilePath);
  void getLogFileDirectory(StringStorage *logFileDirectory);
  void setLogFilePath(const TCHAR *logFilePath);
  void reloadConfigOnClientAuth(bool allow);

  bool shouldReloadConfigOnClientAuth();

protected:

  int m_rfbPort;
  int m_httpPort;

  bool m_blankScreen;

  bool m_enableFileTransfers;
  bool m_removeWallpaper;

  DisconnectAction m_disconnectAction;

  bool m_acceptRfbConnections;
  bool m_acceptHttpConnections;

  unsigned char m_primaryPassword[VNC_PASSWORD_SIZE];
  unsigned char m_readonlyPassword[VNC_PASSWORD_SIZE];
  unsigned char m_controlPassword[VNC_PASSWORD_SIZE];

  unsigned int m_idleTimeout;

  bool m_useAuthentication;
  bool m_onlyLoopbackConnections;
  bool m_enableAppletParamInUrl;
  int m_logLevel;
  bool m_useControlAuth;

  bool m_alwaysShared;
  bool m_neverShared;
  bool m_disconnectClients;

  unsigned int m_pollingInterval;

  bool m_blockRemoteInput;

  bool m_blockLocalInput;

  bool m_localInputPriority;

  unsigned int m_localInputPriorityTimeout;

  bool m_defaultActionAccept;

  unsigned int m_queryTimeout;

  PortMappingContainer m_portMappings;

  IpAccessControl m_accessControlContainer;
  bool m_allowLoopbackConnections;

  bool m_isTrayIconDisabled;

  StringVector m_videoClassNames;
  

  unsigned int m_videoRecognitionInterval;
  bool m_grabTransparentWindows;

  bool m_saveLogToAllUsersPath;
  bool m_showTrayIcon;

  StringStorage m_logFilePath;
private:

  bool m_hasPrimaryPassword;
  bool m_hasReadOnlyPassword;
  bool m_hasControlPassword;
  bool m_reloadConfigOnClientAuth;

  LocalMutex m_objectCS;
};

#endif
