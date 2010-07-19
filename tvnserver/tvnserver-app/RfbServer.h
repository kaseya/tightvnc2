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

#ifndef __RFB_SERVER_H__
#define __RFB_SERVER_H__

#include "RfbClientManager.h"
#include "thread/Thread.h"
#include "network/TcpServer.h"
#include "region/Rect.h"

class RfbServer : public TcpServer
{
public:
  RfbServer(const TCHAR *bindHost, unsigned short bindPort,
            RfbClientManager *clientManager,
            bool lockAddr,
            const Rect *viewPort = 0)
            throw(Exception);
  virtual ~RfbServer();

protected:
  virtual void onAcceptConnection(SocketIPv4 *socket);

protected:
  RfbClientManager *m_clientManager;
  bool m_viewPortSpecified;
  Rect m_viewPort;
};

#endif 
