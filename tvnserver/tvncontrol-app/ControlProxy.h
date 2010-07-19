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

#ifndef _CONTROL_PROXY_H_
#define _CONTROL_PROXY_H_

#include "tvncontrol-app/ControlGate.h"
#include "tvncontrol-app/RfbClientInfo.h"
#include "tvncontrol-app/TvnServerInfo.h"

#include "server-config-lib/ServerConfig.h"

#include "ControlMessage.h"
#include "RemoteException.h"

#include <list>

using namespace std;

class ControlProxy
{
public:
  ControlProxy(ControlGate *gate);
  virtual ~ControlProxy();

  void setPasswordProperties(const TCHAR *passwordFile,
                             bool getPassFromConfigEnabled,
                             bool forService);

  TvnServerInfo getServerInfo() throw(IOException, RemoteException);

  void getClientsList(list<RfbClientInfo *> *clients) throw(IOException, RemoteException);

  void reloadServerConfig() throw(IOException, RemoteException);
  
  void disconnectAllClients() throw(IOException, RemoteException);

  void shutdownTightVnc() throw(IOException, RemoteException);

  void makeOutgoingConnection(const TCHAR *connectString, bool viewOnly) throw(IOException, RemoteException);

  void setServerConfig(ServerConfig *config) throw(IOException, RemoteException);

  void getServerConfig(ServerConfig *config) throw(IOException, RemoteException);

  bool getShowTrayIconFlag() throw(IOException, RemoteException);

  void updateTvnControlProcessId(DWORD processId) throw(IOException, RemoteException);

protected:
  ControlMessage *createMessage(DWORD messageId);

protected:
  ControlGate *m_gate;
  ControlMessage *m_message;
private:
  void releaseMessage();

  StringStorage m_passwordFile;
  bool m_getPassFromConfigEnabled;
  bool m_forService;
};

#endif
