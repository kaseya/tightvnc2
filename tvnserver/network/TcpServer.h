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

#ifndef _MULTI_THREAD_TCP_SERVER_H_
#define _MULTI_THREAD_TCP_SERVER_H_

#include "thread/Thread.h"
#include "util/Exception.h"
#include "network/socket/SocketIPv4.h"

class TcpServer : private Thread
{
public:
  TcpServer(const TCHAR *bindHost,
            unsigned short bindPort,
            bool autoStart = false,
            bool lockAddr = false) throw(Exception);
  virtual ~TcpServer();

  const TCHAR *getBindHost() const;

  unsigned short getBindPort() const;

protected:
  virtual void start();

  virtual void onAcceptConnection(SocketIPv4 *socket) = 0;

  virtual void execute();

private:
  SocketIPv4 m_listenSocket;
  StringStorage m_bindHost;
  unsigned short m_bindPort;
};

#endif
