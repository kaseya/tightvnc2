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

#include "ControlApplication.h"
#include "ControlTrayIcon.h"
#include "ControlTrayIcon.h"
#include "ControlPipeName.h"
#include "ControlCommand.h"
#include "ReloadConfigCommand.h"
#include "DisconnectAllCommand.h"
#include "ControlAuth.h"
#include "ConnectCommand.h"
#include "ShutdownCommand.h"

#include "util/VncPassCrypt.h"
#include "util/winhdr.h"
#include "util/StringTable.h"

#include "tvncontrol-app/ControlCommandLine.h"
#include "tvnserver-app/TvnServerHelp.h"

#include "win-system/Environment.h"
#include "win-system/Shell.h"

#include "thread/ZombieKiller.h"
#include "thread/GlobalMutex.h"

#include "gui/CommonControlsEx.h"

#include "network/socket/WindowsSocket.h"

#include "tvnserver/resource.h"

#include "wsconfig-lib/ConfigDialog.h"

ControlApplication::ControlApplication(HINSTANCE hinst, const TCHAR *commandLine = 0)
 : LocalWindowsApplication(hinst), m_serverControl(0), m_gate(0), m_transport(0),
   m_trayIcon(0), m_slaveModeEnabled(false)
{
  m_commandLine.setString(commandLine);

  CommonControlsEx::init();

  WindowsSocket::startup(2, 1);
}

ControlApplication::~ControlApplication()
{
  try {
    WindowsSocket::cleanup();
  } catch (...) { }

  if (m_serverControl != 0) {
    delete m_serverControl;
  }
  if (m_gate != 0) {
    delete m_gate;
  }
  if (m_transport != 0) {
    delete m_transport;
  }
}

int ControlApplication::run()
{
  ControlCommandLine cmdLineParser;

  try {
    cmdLineParser.parse(m_commandLine.getString());
  } catch (CommandLineFormatException &) {
    TvnServerHelp::showUsage();
    return 1;
  }

  if (cmdLineParser.hasConfigAppFlag() || cmdLineParser.hasConfigServiceFlag()) {
    return runConfigurator( cmdLineParser );
  }

  if(cmdLineParser.hasSetVncPortAndPrimaryPasswdFlag() ) {
      bool hasServiceFlag = ! cmdLineParser.hasPortableFlag();
      ServerConfig *config = loadServerConfig( hasServiceFlag, cmdLineParser.hasPortableFlag(), cmdLineParser.getVncIniDirectoryPath() );
      setVncPort(cmdLineParser, config);
      setControlOrVncPrimaryPassword(cmdLineParser, config);
      Configurator::getInstance()->save();
      return 0;
  }
  if (cmdLineParser.hasSetControlPasswordFlag() || cmdLineParser.hasSetVncPasswordFlag()) {
      bool hasServiceFlag = ! cmdLineParser.hasPortableFlag();
      ServerConfig *config = loadServerConfig( hasServiceFlag, cmdLineParser.hasPortableFlag(), cmdLineParser.getVncIniDirectoryPath() );
      setControlOrVncPrimaryPassword(cmdLineParser, config);
      Configurator::getInstance()->save();
      return 0;
  }
  
  if ( cmdLineParser.hasSetVncPortFlag() ) {
      bool hasServiceFlag = ! cmdLineParser.hasPortableFlag();
      ServerConfig *config = loadServerConfig( hasServiceFlag, cmdLineParser.hasPortableFlag(), cmdLineParser.getVncIniDirectoryPath() );
      setVncPort(cmdLineParser, config);
      Configurator::getInstance()->save();
      return 0;
  }



  int retCode = 0;

  GlobalMutex *appGlobalMutex = 0;

  if (cmdLineParser.hasControlServiceFlag() && cmdLineParser.isSlave()) {
    try {
      appGlobalMutex = new GlobalMutex(_T("tvnserverServiceControlSlave"), false, true);
    } catch (...) {
      return 1;
    }
  }

  ZombieKiller zombieKiller;

  try {
    connect(cmdLineParser.hasControlServiceFlag(), cmdLineParser.isSlave());
  } catch (Exception &) {
    if (!cmdLineParser.isSlave()) {
      const TCHAR *msg = StringTable::getString(IDS_FAILED_TO_CONNECT_TO_CONTROL_SERVER);
      const TCHAR *caption = StringTable::getString(IDS_MBC_TVNCONTROL);
      MessageBox(0, msg, caption, MB_OK | MB_ICONERROR);
    }
    return 1;
  }

  if (cmdLineParser.isCommandSpecified()) {
    Command *command = 0;

    StringStorage passwordFile;
    cmdLineParser.getPasswordFile(&passwordFile);
    m_serverControl->setPasswordProperties(passwordFile.getString(), true,
                                           cmdLineParser.hasControlServiceFlag());

    if (cmdLineParser.hasKillAllFlag()) {
      command = new DisconnectAllCommand(m_serverControl);
    } else if (cmdLineParser.hasReloadFlag()) {
      command = new ReloadConfigCommand(m_serverControl);
    } else if (cmdLineParser.hasConnectFlag()) {
      StringStorage hostName;
      cmdLineParser.getConnectHostName(&hostName);
      command = new ConnectCommand(m_serverControl, hostName.getString());
    } else if (cmdLineParser.hasShutdownFlag()) {
      command = new ShutdownCommand(m_serverControl);
    }

    retCode = runControlCommand(command);

    if (command != 0) {
      delete command;
    }
  } else {
    bool showIcon = true;

    if (cmdLineParser.isSlave()) {
      m_slaveModeEnabled = true;
      try {
        try {
          showIcon = m_serverControl->getShowTrayIconFlag();
        } catch (RemoteException &remEx) {
          notifyServerSideException(remEx.getMessage());
        }
        try {
          m_serverControl->updateTvnControlProcessId(GetCurrentProcessId());
        } catch (RemoteException &remEx) {
          notifyServerSideException(remEx.getMessage());
        }
      } catch (IOException &) {
        notifyConnectionLost();
        return 1;
      } catch (Exception &) {
        _ASSERT(FALSE);
      }
    }

    retCode = runControlInterface(showIcon);
  }

  if (appGlobalMutex != 0) {
    delete appGlobalMutex;
  }

  return retCode;
}

void ControlApplication::connect(bool controlService, bool slave)
{
  StringStorage pipeName;
  ControlPipeName::createPipeName(controlService, &pipeName);

  int numTriesRemaining = slave ? 10 : 1;
  int msDelayBetweenTries = 2000;

  while (numTriesRemaining-- > 0) {
    try {
      m_transport = TransportFactory::createPipeClientTransport(pipeName.getString());
      break;
    } catch (Exception &) {
      if (numTriesRemaining <= 0) {
        throw;
      }
    }
    Sleep(msDelayBetweenTries);
  }

  m_gate = new ControlGate(m_transport->getIOStream());
  m_serverControl = new ControlProxy(m_gate);
}

void ControlApplication::notifyServerSideException(const TCHAR *reason)
{
  StringStorage message;

  message.format(StringTable::getString(IDS_CONTROL_SERVER_RAISE_EXCEPTION), reason);

  MessageBox(0, message.getString(), StringTable::getString(IDS_MBC_TVNSERVER), MB_OK | MB_ICONERROR);
}

void ControlApplication::notifyConnectionLost()
{
  MessageBox(0,
             StringTable::getString(IDS_CONTROL_CONNECTION_LOST),
             StringTable::getString(IDS_MBC_TVNCONTROL),
             MB_OK | MB_ICONEXCLAMATION);
}

void ControlApplication::execute()
{
  try {
    while (!isTerminating()) {
      Thread::sleep(500);
      bool showIcon = m_serverControl->getShowTrayIconFlag() || !m_slaveModeEnabled;

      if (showIcon && !m_trayIcon->isVisible()) {
        m_trayIcon->show();
      }
      if (m_trayIcon != 0 && !showIcon) {
        m_trayIcon->hide();
      }
      if (m_trayIcon->isVisible()) {
        m_trayIcon->syncStatusWithServer();
      }
    }
  } catch (...) {
    m_trayIcon->terminate();
    m_trayIcon->waitForTermination();
    shutdown();
  }
}

int ControlApplication::runControlInterface(bool showIcon)
{
  ControlTrayIcon *icon = 0;

  m_trayIcon = new ControlTrayIcon(m_serverControl, this, this, showIcon);

  resume();

  int ret = WindowsApplication::run();

  terminate();
  wait();

  delete m_trayIcon;

  return ret;
}

int ControlApplication::runControlCommand(Command *command)
{
  ControlCommand ctrlCmd(command);

  ctrlCmd.execute();

  int errorCode = ctrlCmd.executionResultOk() ? 0 : 1;
  return errorCode;
}

int ControlApplication::runConfigurator( ControlCommandLine cmdLineParser )
{
   bool configService = cmdLineParser.hasConfigServiceFlag();
   bool isRunAsRequested = cmdLineParser.hasDontElevateFlag();
   bool runAsPortable = cmdLineParser.hasPortableFlag();

    if (configService && (IsUserAnAdmin() == FALSE)) 
    {
        if (isRunAsRequested) 
        {
            MessageBox(0,
                StringTable::getString(IDS_ADMIN_RIGHTS_NEEDED),
                StringTable::getString(IDS_MBC_TVNCONTROL),
                MB_OK | MB_ICONERROR);
            return 0;
        }
    StringStorage pathToBinary;
    StringStorage childCommandLine;

    Environment::getCurrentModulePath(&pathToBinary);
    childCommandLine.format(_T("%s -dontelevate"), m_commandLine.getString());

    try {
      Shell::runAsAdmin(pathToBinary.getString(), childCommandLine.getString());
    } catch (SystemException &sysEx) {
      if (sysEx.getErrorCode() != ERROR_CANCELLED) {
        MessageBox(0,
          sysEx.getMessage(),
          StringTable::getString(IDS_MBC_TVNCONTROL),
          MB_OK | MB_ICONERROR);
      }
      return 1;
    } 
    return 0;
  }

  loadServerConfig(configService, runAsPortable, cmdLineParser.getVncIniDirectoryPath() );

  ConfigDialog confDialog(configService, 0);

  return confDialog.showModal();
}

void ControlApplication::getCryptedPassword(UINT8 cryptedPass[8], const TCHAR *plainTextPassString)
{
  UINT8 textPlainPass[8];
  memset(textPlainPass, 0, 8);
  char plainTextPassANSI[9];
  StringStorage plainTextPassTCHAR(plainTextPassString);
  plainTextPassTCHAR.toAnsiString(plainTextPassANSI, 9);
  memcpy(textPlainPass, plainTextPassANSI, strlen(plainTextPassANSI));
  VncPassCrypt::getEncryptedPass(cryptedPass, textPlainPass);
}

ServerConfig* ControlApplication::loadServerConfig( bool hasServiceFlag, bool hasPortableFlag, const TCHAR* vncIniDirectoryPath )
{
    Configurator *configurator = Configurator::getInstance();

    configurator->setServiceFlag( hasServiceFlag );
    configurator->setPortableRunFlag( hasPortableFlag );
    configurator->setVncIniDirectoryPath( vncIniDirectoryPath );

    configurator->load();
    return configurator->getServerConfig();
}

void ControlApplication::setControlOrVncPrimaryPassword(ControlCommandLine& cmdLineParser, ServerConfig *config)
{
    UINT8 cryptedPass[8];
    if (cmdLineParser.hasSetControlPasswordFlag()) {
        getCryptedPassword(cryptedPass, cmdLineParser.getControlPassword());
        config->setControlPassword((const unsigned char *)cryptedPass);
        config->useControlAuth(true);
    } else {
        getCryptedPassword(cryptedPass, cmdLineParser.getPrimaryVncPassword());
        config->setPrimaryPassword((const unsigned char *)cryptedPass);
        config->useAuthentication(true);
    }
   
}

void ControlApplication::setVncPort(ControlCommandLine& cmdLineParser, ServerConfig *config)
{
    config->setRfbPort( cmdLineParser.getVncPort() );
}
