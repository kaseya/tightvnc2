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

#include "RfbServer.h"
#include "server-config-lib/Configurator.h"
#include "util/Log.h"

RfbServer::RfbServer(const TCHAR *bindHost, unsigned short bindPort,
                     RfbClientManager *clientManager,
                     bool lockAddr,
                     const Rect *viewPort)
: TcpServer(bindHost, bindPort, false, lockAddr),
  m_clientManager(clientManager),
  m_viewPortSpecified(false)
{
  if (viewPort != 0) {
    m_viewPort = *viewPort;
    m_viewPortSpecified = true;
  }

  TcpServer::start();

  if (viewPort == 0) {
    Log::message(_T("Rfb server started at %s:%d"), bindHost, (int)bindPort);
  } else {
    Log::message(_T("Rfb server started at %s:%d with [%d %d %d %d] view port specified"),
                 bindHost, (int)bindPort,
                 viewPort->left, viewPort->right, viewPort->top, viewPort->bottom);
  }
}

RfbServer::~RfbServer()
{
  Log::message(_T("Rfb server at %s:%d stopped"), getBindHost(), (int)getBindPort());
}

void RfbServer::onAcceptConnection(SocketIPv4 *socket)
{
  try {
    SocketAddressIPv4 peerAddr;
    socket->getPeerAddr(&peerAddr);
    StringStorage peerIpString;
    peerAddr.toString(&peerIpString);

    Log::message(_T("Incoming rfb connection from %s"), peerIpString.getString());

    struct sockaddr_in addr_in = peerAddr.getSockAddr();

    ServerConfig *config = Configurator::getInstance()->getServerConfig();
    IpAccessRule::ActionType action = config->getActionByAddress((unsigned long)addr_in.sin_addr.S_un.S_addr);

    if (action == IpAccessRule::ACTION_TYPE_DENY) {
      Log::message(_T("Connection rejected due to access control rules"));
      delete socket;
      return;
    }

    socket->enableNaggleAlgorithm(false);

    if (m_viewPortSpecified) {
      m_clientManager->addNewConnection(socket, &m_viewPort, false, false);
    } else {
      m_clientManager->addNewConnection(socket, 0, false, false);
    }
  } catch (Exception &ex) {
    Log::error(_T("Failed to process incoming rfb connection with following reason: \"%s\""), ex.getMessage());
  }
}
