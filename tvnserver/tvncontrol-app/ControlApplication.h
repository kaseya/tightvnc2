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

#ifndef _CONTROL_APPLICATION_H_
#define _CONTROL_APPLICATION_H_

#include "Transport.h"
#include "TransportFactory.h"

#include "util/CommonHeader.h"
#include "util/ResourceLoader.h"
#include "util/Command.h"

#include "thread/Thread.h"
#include "thread/LocalMutex.h"

#include "tvncontrol-app/ControlGate.h"
#include "tvncontrol-app/ControlAuthException.h"

#include "ControlProxy.h"
#include "Notificator.h"

#include "win-system/LocalWindowsApplication.h"

class ControlTrayIcon;

class ControlApplication : public LocalWindowsApplication,  
                           public Notificator,  
                           private Thread  
{
public:
  ControlApplication(HINSTANCE hinst, const TCHAR *commandLine) throw(Exception);
  virtual ~ControlApplication();

  virtual int run();

  virtual void notifyServerSideException(const TCHAR *reason);
  virtual void notifyConnectionLost();

protected:
  void connect(bool controlService, bool slave) throw(Exception);

  void execute();

  int runControlInterface(bool showIcon);
  int runControlCommand(Command *command);
  int runConfigurator(bool configService, bool isRunAsRequested);

private:
  static void getCryptedPassword(UINT8 cryptedPass[8], const TCHAR *plainTextPass);

protected:
  Transport *m_transport;
  ControlGate *m_gate;
  ControlProxy *m_serverControl;
  ControlTrayIcon *m_trayIcon;
  StringStorage m_commandLine;
  bool m_slaveModeEnabled;
  friend class ControlTrayIcon;
};

#endif
