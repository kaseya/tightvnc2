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

#ifndef _CONTROL_CLIENT_H_
#define _CONTROL_CLIENT_H_

#include "RfbClientManager.h"

#include "tvncontrol-app/ControlGate.h"
#include "tvncontrol-app/ControlAuthException.h"
#include "tvncontrol-app/Transport.h"
#include "ControlAppAuthenticator.h"

class ControlException : public Exception {
public:
  ControlException(const Exception *parent) : Exception(parent->getMessage()) { }
  ControlException(const TCHAR *message) : Exception(message) { }
  virtual ~ControlException() { };
};

class ControlClient : public Thread
{
public:
  ControlClient(Transport *transport,
                RfbClientManager *rfbClientManager,
                ControlAppAuthenticator *authenticator);
  virtual ~ControlClient();

protected:
  virtual void execute();

  virtual void onTerminate();

private:
  void sendError(const TCHAR *message) throw(IOException);

  void authMsgRcdv() throw(IOException);

  void getClientsListMsgRcvd() throw(IOException);
  void getServerInfoMsgRcvd() throw(IOException);
  void reloadConfigMsgRcvd() throw(IOException);
  void disconnectAllMsgRcvd() throw(IOException);
  void shutdownMsgRcvd() throw(IOException);
  void addClientMsgRcvd() throw(IOException);
  void setServerConfigMsgRcvd() throw(IOException);
  void getServerConfigMsgRcvd() throw(IOException);
  void getShowTrayIconFlagMsgRcvd() throw(IOException);
  void updateTvnControlProcessIdMsgRcvd() throw(IOException);

private:
  Transport *m_transport;
  Channel *m_stream;

  ControlGate *m_gate;

  RfbClientManager *m_rfbClientManager;

  bool m_authPassed;

  ControlAppAuthenticator *m_authenticator;

  static const UINT32 REQUIRES_AUTH[];
};

#endif
