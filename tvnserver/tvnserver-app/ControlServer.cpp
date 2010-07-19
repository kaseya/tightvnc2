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

#include "ControlServer.h"
#include "ControlClient.h"
#include "util/Log.h"

ControlServer::ControlServer(Transport *serverTransport,
                             RfbClientManager *rfbClientManager)
: m_authenticator(30000, 3),
  m_serverTransport(serverTransport), m_rfbClientManager(rfbClientManager)
{
  Log::message(_T("%s"), _T("Control server started"));

  resume();
}

ControlServer::~ControlServer()
{
  Log::message(_T("Destroying control server transport"));

  terminate();
  wait();

  try {
    m_serverTransport->close();
  } catch (Exception &ex) {
    Log::error(_T("Failed to destroy control server transport with '%s' reason"), ex.getMessage());
  }

  delete m_serverTransport;

  m_authenticator.breakAndDisableAuthentications();

  Log::message(_T("%s"), _T("Control server stopped"));
}

void ControlServer::execute()
{
  try {
    while (!isTerminating()) {
      Transport *transport = m_serverTransport->accept();

      ControlClient *clientThread = new ControlClient(transport,
                                                      m_rfbClientManager,
                                                      &m_authenticator);

      clientThread->resume();

      m_threadCollector.addThread(clientThread);
    }
  } catch (Exception &ex) {
    Log::error(_T("Exception on control server thread: %s"), ex.getMessage());
  }
}

void ControlServer::onTerminate()
{
  try { m_serverTransport->close(); } catch (...) { }
}
