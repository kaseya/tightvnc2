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

#ifndef _TCP_CLIENT_THREAD_H_
#define _TCP_CLIENT_THREAD_H_

#include "thread/Thread.h"

#include "socket/SocketIPv4.h"

class TcpClientThread : public Thread
{
public:
  TcpClientThread(SocketIPv4 *socket);
  virtual ~TcpClientThread();

protected:
  virtual void onTerminate();

protected:
  SocketIPv4 *m_socket;
};

#endif
