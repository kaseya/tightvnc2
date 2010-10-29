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

#include "ControlCommandLine.h"

#include "util/CommonHeader.h"
#include "util/CommandLine.h"

#include "ConnectStringParser.h"

const TCHAR ControlCommandLine::SET_CONTROL_PASSWORD[] = _T("-setservicecontrolpass");
const TCHAR ControlCommandLine::SET_PRIMARY_VNC_PASSWORD[] = _T("-setservicevncpass");

const TCHAR ControlCommandLine::CONTROL_SERVICE[] = _T("-controlservice");
const TCHAR ControlCommandLine::CONTROL_APPLICATION[] = _T("-controlapp");
const TCHAR ControlCommandLine::PASSWORD_FILE[] = _T("-passfile");
const TCHAR ControlCommandLine::CONFIG_RELOAD[]  = _T("-reload");
const TCHAR ControlCommandLine::DISCONNECT_ALL[] = _T("-disconnectall");
const TCHAR ControlCommandLine::CONNECT[] = _T("-connect");
const TCHAR ControlCommandLine::SHUTDOWN[] = _T("-shutdown");

const TCHAR ControlCommandLine::CONFIG_APPLICATION[] = _T("-configapp");
const TCHAR ControlCommandLine::CONFIG_SERVICE[] = _T("-configservice");

const TCHAR ControlCommandLine::SLAVE_MODE[] = _T("-slave");

const TCHAR ControlCommandLine::DONT_ELEVATE[] = _T("-dontelevate");

const TCHAR ControlCommandLine::PORTABLE[] = _T("-portable");

const TCHAR ControlCommandLine::SET_VNC_PORT[] = _T("-setvncport");
const TCHAR ControlCommandLine::VNC_INI_DIRECTORY[] = _T("-inidirectory");



ControlCommandLine::ControlCommandLine()
{
}

ControlCommandLine::~ControlCommandLine()
{
}

void ControlCommandLine::parse(const TCHAR *commandLine)
{
  CommandLineFormat fmt[] = {
    { PASSWORD_FILE, NEEDS_ARG },
    { CONFIG_RELOAD, NO_ARG },
    { DISCONNECT_ALL, NO_ARG },
    { CONNECT, NEEDS_ARG },
    { SHUTDOWN, NO_ARG },
    { SET_PRIMARY_VNC_PASSWORD, NEEDS_ARG },
    { SET_CONTROL_PASSWORD, NEEDS_ARG },
    { CONTROL_SERVICE, NO_ARG },
    { CONTROL_APPLICATION, NO_ARG },
    { CONFIG_APPLICATION, NO_ARG },
    { CONFIG_SERVICE, NO_ARG },
    { SLAVE_MODE, NO_ARG },
    { DONT_ELEVATE, NO_ARG },
    { PORTABLE, NO_ARG },
    { SET_VNC_PORT, NEEDS_ARG },
    { VNC_INI_DIRECTORY, NEEDS_ARG },

  };

  if (!CommandLine::parse(fmt, sizeof(fmt) / sizeof(CommandLineFormat), commandLine)) {
    throw CommandLineFormatException();
  }

  if (hasConfigServiceFlag() && ((int)m_foundKeys.size() > (optionSpecified(DONT_ELEVATE) ? 2 : 1))) {
    throw CommandLineFormatException();
  }

  if ( ( hasConfigAppFlag()  && ! hasPortableFlag() && m_foundKeys.size() > 1  ) ||
       ( hasConfigAppFlag()  &&  hasPortableFlag()  && m_foundKeys.size() > 2 ) ) {
    throw CommandLineFormatException();
  }

  bool hasPassFile = hasPasswordFile();
  if (hasPassFile) {
    optionSpecified(PASSWORD_FILE, &m_passwordFile);
  }

  if (hasKillAllFlag() && hasReloadFlag()) {
    throw CommandLineFormatException();
  }

  if (hasConnectFlag()) {
    optionSpecified(CONNECT, &m_connectHostName);
  }

  if(hasSetVncPortAndPrimaryPasswdFlag()) {
      setVncPortAndPrimaryPassword();
  }

  if ((hasControlServiceFlag() || hasControlAppFlag()) && (isSlave()) && (m_foundKeys.size() > 2)) {
    throw CommandLineFormatException();
  }

  bool hasNotSlaveControl = (hasControlServiceFlag() || hasControlAppFlag()) && !isSlave();
  if ((hasNotSlaveControl && !hasPassFile && m_foundKeys.size() > 2) ||
      (hasNotSlaveControl && hasPassFile && m_foundKeys.size() != 3)) {
    throw CommandLineFormatException();
  }

  if(hasSetVncPortFlag() && ! hasSetVncPasswordFlag()) {
      setVncPort();
  }

  if(! hasSetVncPortFlag() && ( hasSetVncPasswordFlag() || hasSetControlPasswordFlag()) ) {
      setVncOrControlPassword();
  }

  if (m_foundKeys.size() == 0) {
    throw CommandLineFormatException();
  }
}

void ControlCommandLine::getPasswordFile(StringStorage *passwordFile) const
{
  *passwordFile = m_passwordFile;
}

bool ControlCommandLine::hasPasswordFile()
{
  return optionSpecified(PASSWORD_FILE);
}

bool ControlCommandLine::hasReloadFlag()
{
  return optionSpecified(CONFIG_RELOAD);
}

bool ControlCommandLine::hasKillAllFlag()
{
  return optionSpecified(DISCONNECT_ALL);
}

bool ControlCommandLine::hasConnectFlag()
{
  return optionSpecified(CONNECT);
}

void ControlCommandLine::getConnectHostName(StringStorage *hostName) const
{
  *hostName = m_connectHostName;
}

bool ControlCommandLine::hasShutdownFlag()
{
  return optionSpecified(SHUTDOWN);
}

bool ControlCommandLine::hasSetVncPasswordFlag()
{
  return optionSpecified(SET_PRIMARY_VNC_PASSWORD);
}

bool ControlCommandLine::hasSetControlPasswordFlag()
{
  return optionSpecified(SET_CONTROL_PASSWORD);
}

bool ControlCommandLine::hasConfigAppFlag()
{
  return optionSpecified(CONFIG_APPLICATION);
}

bool ControlCommandLine::hasConfigServiceFlag()
{
  return optionSpecified(CONFIG_SERVICE);
}

bool ControlCommandLine::hasDontElevateFlag()
{
  return optionSpecified(DONT_ELEVATE);
}

bool ControlCommandLine::hasControlServiceFlag()
{
  return optionSpecified(CONTROL_SERVICE);
}

bool ControlCommandLine::hasControlAppFlag()
{
  return optionSpecified(CONTROL_APPLICATION);
}

bool ControlCommandLine::isSlave()
{
  return optionSpecified(SLAVE_MODE);
}

const TCHAR *ControlCommandLine::getPrimaryVncPassword() const
{
  return m_vncPassword.getString();
}

const TCHAR *ControlCommandLine::getControlPassword() const
{
  return m_controlPassword.getString();
}

bool ControlCommandLine::isCommandSpecified()
{
  return hasKillAllFlag() || hasReloadFlag() || hasSetControlPasswordFlag() ||
         hasSetVncPasswordFlag() || hasConnectFlag() || hasShutdownFlag();
}

bool ControlCommandLine::hasPortableFlag()
{
    return optionSpecified(PORTABLE);
}

bool ControlCommandLine::hasSetVncPortFlag()
{
    return optionSpecified(SET_VNC_PORT);
}

const int ControlCommandLine::getVncPort() const
{
    return _ttoi( m_vncPort.getString() );
}

bool ControlCommandLine::hasVncIniDirectoryFlag()
{
    return optionSpecified(VNC_INI_DIRECTORY);
}

const TCHAR* ControlCommandLine::getVncIniDirectoryPath() const
{
    return m_vncIniDirectoryPath.getString();
}

bool ControlCommandLine::hasSetVncPortAndPrimaryPasswdFlag()
{
    return optionSpecified(SET_VNC_PORT) && optionSpecified(SET_PRIMARY_VNC_PASSWORD);
}

void ControlCommandLine::setVncOrControlPassword()
{
    bool hasSetVncOrControlPasswordFlag =  hasSetVncPasswordFlag() || hasSetControlPasswordFlag();
    if ( ( hasSetVncOrControlPasswordFlag && ! hasPortableFlag() && m_foundKeys.size() > 1 ) ||
        ( hasSetVncOrControlPasswordFlag &&  hasPortableFlag()  && ! hasVncIniDirectoryFlag() && m_foundKeys.size() > 2 ) || 
        ( hasSetVncOrControlPasswordFlag &&  hasPortableFlag()  && hasVncIniDirectoryFlag() && m_foundKeys.size() > 3 ) ) {
            throw CommandLineFormatException();
    } else {
        optionSpecified(SET_CONTROL_PASSWORD, &m_controlPassword);
        optionSpecified(SET_PRIMARY_VNC_PASSWORD, &m_vncPassword);
        optionSpecified(VNC_INI_DIRECTORY, &m_vncIniDirectoryPath);

    }
}

void ControlCommandLine::setVncPort()
{
    if ( ( hasSetVncPortFlag() && ! hasPortableFlag() && m_foundKeys.size() > 1 ) ||
        ( hasSetVncPortFlag() &&  hasPortableFlag()  && ! hasVncIniDirectoryFlag() && m_foundKeys.size() > 2 )  ||
        ( hasSetVncPortFlag() &&  hasPortableFlag() && hasVncIniDirectoryFlag() && m_foundKeys.size() > 3 ) ) {
            throw CommandLineFormatException();
    } else {
        optionSpecified(SET_VNC_PORT, &m_vncPort);
        optionSpecified(VNC_INI_DIRECTORY, &m_vncIniDirectoryPath);

    }
}

void ControlCommandLine::setVncPortAndPrimaryPassword()
{
    if ( ( hasSetVncPortFlag() && hasSetVncPasswordFlag() && ! hasPortableFlag() && m_foundKeys.size() > 2 ) ||
        ( hasSetVncPortFlag() && hasSetVncPasswordFlag() &&  hasPortableFlag()  && ! hasVncIniDirectoryFlag() && m_foundKeys.size() > 3 )  ||
        ( hasSetVncPortFlag() && hasSetVncPasswordFlag() &&  hasPortableFlag() && hasVncIniDirectoryFlag() && m_foundKeys.size() > 4 ) ) {
            throw CommandLineFormatException();
    } else {
        optionSpecified(SET_PRIMARY_VNC_PASSWORD, &m_vncPassword);
        optionSpecified(SET_CONTROL_PASSWORD, &m_controlPassword);
        optionSpecified(SET_VNC_PORT, &m_vncPort);
        optionSpecified(VNC_INI_DIRECTORY, &m_vncIniDirectoryPath);
    }
}