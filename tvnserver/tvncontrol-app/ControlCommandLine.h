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

#ifndef _CONTROL_COMMAND_LINE_H_
#define _CONTROL_COMMAND_LINE_H_

#include "util/CommandLine.h"

#include "CommandLineFormatException.h"

class ControlCommandLine : private CommandLine
{
public:
  static const TCHAR CONFIG_APPLICATION[];
  static const TCHAR CONFIG_SERVICE[];

  static const TCHAR CONTROL_SERVICE[];
  static const TCHAR CONTROL_APPLICATION[];
  static const TCHAR PASSWORD_FILE[];
  static const TCHAR CONFIG_RELOAD[];
  static const TCHAR DISCONNECT_ALL[];
  static const TCHAR CONNECT[];
  static const TCHAR SHUTDOWN[];

  static const TCHAR SET_CONTROL_PASSWORD[];
  static const TCHAR SET_PRIMARY_VNC_PASSWORD[];

  static const TCHAR SLAVE_MODE[];
  static const TCHAR DONT_ELEVATE[];

public:
  ControlCommandLine();
  virtual ~ControlCommandLine();

  void parse(const TCHAR *commandLine) throw(CommandLineFormatException);

  void getPasswordFile(StringStorage *passwordFile) const;
  bool hasPasswordFile();
  bool hasReloadFlag();
  bool hasKillAllFlag();
  bool hasConnectFlag();
  void getConnectHostName(StringStorage *hostName) const;
  bool hasShutdownFlag();
  bool hasSetVncPasswordFlag();
  bool hasSetControlPasswordFlag();
  bool hasConfigAppFlag();
  bool hasConfigServiceFlag();
  bool hasControlServiceFlag();
  bool hasControlAppFlag();
  bool hasDontElevateFlag();
  bool isSlave();

  const TCHAR *getPrimaryVncPassword() const;
  const TCHAR *getControlPassword() const;

  bool isCommandSpecified();

protected:
  StringStorage m_vncPassword;
  StringStorage m_controlPassword;

  StringStorage m_connectHostName;
  StringStorage m_passwordFile;
};

#endif
