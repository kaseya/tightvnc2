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

#ifndef SOCKET_IPV4_H
#define SOCKET_IPV4_H

#include "sockdefs.h"

#include "SocketIPv4.h"
#include "SocketAddressIPv4.h"
#include "SocketException.h"

#include "io-lib/Channel.h"
#include "io-lib/IOException.h"

#include "thread/LocalMutex.h"

class SocketIPv4
{
public:
  SocketIPv4();
  virtual ~SocketIPv4();

  void connect(const TCHAR *host, unsigned short port) throw(SocketException);
  void connect(const SocketAddressIPv4 &addr) throw(SocketException);
  void close() throw(SocketException);
  void shutdown(int how) throw(SocketException);
  void bind(const TCHAR *bindHost, unsigned int bindPort);
  void bind(const SocketAddressIPv4 &addr) throw(SocketException);
  bool isBound();
  void listen(int backlog) throw(SocketException);

  SocketIPv4 *accept() throw(SocketException);

  int send(const char *data, int size, int flags = 0) throw(IOException);
  int recv(char *buffer, int size, int flags = 0) throw(IOException);

  bool getLocalAddr(SocketAddressIPv4 *addr);
  bool getPeerAddr(SocketAddressIPv4 *addr);

  void setSocketOptions(int level, int name, void *value, socklen_t len) throw(SocketException);
  void getSocketOptions(int level, int name, void *value, socklen_t *len) throw(SocketException);

  void enableNaggleAlgorithm(bool enabled) throw(SocketException);
  void setExclusiveAddrUse() throw(SocketException);

protected:
  LocalMutex m_mutex;

  SOCKET m_socket;

  SocketAddressIPv4 *m_localAddr;
  SocketAddressIPv4 *m_peerAddr;

  bool m_isBound;
};

#endif
