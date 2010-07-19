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

#include "HttpClient.h"

#include "HttpRequestHandler.h"

#include "network/socket/SocketStream.h"

#include "util/Log.h"

HttpClient::HttpClient(SocketIPv4 *socket)
: TcpClientThread(socket)
{
  m_stream = new SocketStream(socket);

  m_dIS = new DataInputStream(m_stream);
  m_dOS = new DataOutputStream(m_stream);
}

HttpClient::~HttpClient()
{
  if (Thread::isActive()) {
    Thread::wait();
  }

  delete m_stream;
  delete m_dOS;
  delete m_dIS;
}

void HttpClient::execute()
{
  try {

    SocketAddressIPv4 peerAddress;

    m_socket->getPeerAddr(&peerAddress);

    StringStorage peerHost;

    peerAddress.toString(&peerHost);

    HttpRequestHandler httpRequestHandler(m_dIS, m_dOS, peerHost.getString());

    httpRequestHandler.processRequest();
  } catch (IOException &ioEx) { } 

  try {
    m_socket->shutdown(SD_BOTH);
  } catch (...) { } 

  try {
    m_socket->close();
  } catch (...) { } 
}
