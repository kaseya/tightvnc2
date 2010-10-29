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

#include "TvnService.h"

#include "ServerCommandLine.h"
#include "util/CommandLine.h"



#include "win-system/SCMClient.h"
#include "win-system/Environment.h"

const TCHAR TvnService::SERVICE_COMMAND_LINE_KEY[] = _T("-service");
const TCHAR TvnService::SERVICE_PORTABLE_COMMAND_LINE_KEY[] = _T("-portableservice");


const TCHAR TvnService::SERVICE_NAME[]             = _T("tvnserver");
const TCHAR TvnService::SERVICE_NAME_TO_DISPLAY[]  = _T("TightVNC Server");

const TCHAR TvnService::SERVICE_PORTABLE_NAME[]             = _T("tvnserverportable");
const TCHAR TvnService::SERVICE_PORTABLE_NAME_TO_DISPLAY[]  = _T("Live Connect Server");

TvnService::TvnService(const TCHAR* serviceName /*SERVICE_NAME*/,const TCHAR *commandLine /*=0*/)
: Service(serviceName), m_tvnServer(0), m_vncIniDirectory(_T(""))
{
    m_commandLine.setString(commandLine);
    parse();

}

TvnService::~TvnService()
{
}


void TvnService::onStart()
{
  
  bool isRunPortable = m_name.isEqualTo(SERVICE_PORTABLE_NAME);
  
  m_tvnServer = new TvnServer(true, m_vncIniDirectory,isRunPortable);

  m_tvnServer->addListener(this);
}

void TvnService::main()
{
  m_shutdownEvent.waitForEvent();

  m_tvnServer->removeListener(this);

  delete m_tvnServer;

  m_tvnServer = 0;
}

void TvnService::onStop()
{
  m_shutdownEvent.notify();
}

void TvnService::onTvnServerShutdown()
{
  TvnService::stop();
}

void TvnService::install()
{
    StringStorage binPath;

    TvnService::getBinPath(&binPath, SERVICE_NAME);

    installService(SERVICE_NAME, SERVICE_NAME_TO_DISPLAY, binPath);
}

void TvnService::installPortable(const TCHAR *commandLine /*=0 */)
{
    StringStorage binPath;

    TvnService::getBinPath(&binPath, SERVICE_PORTABLE_NAME, commandLine);

    installService(SERVICE_PORTABLE_NAME, SERVICE_PORTABLE_NAME_TO_DISPLAY, binPath, SERVICE_DEMAND_START);
}

void TvnService::installService(const TCHAR* serviceName, const TCHAR* serviceDisplayName, StringStorage binPath, DWORD startType /*SERVICE_AUTO_START*/)
{
    SCMClient scManager;

    scManager.installService(serviceName, serviceDisplayName,binPath.getString(), _T(""), startType);
}


void TvnService::remove()
{
  removeService(SERVICE_NAME);
}

void TvnService::removePortable()
{
    removeService(SERVICE_PORTABLE_NAME);
}

void TvnService::removeService(const TCHAR* serviceName)
{
    SCMClient scManager;

    scManager.removeService(serviceName);
}


void TvnService::reinstall()
{
  try {
    remove();
  } catch (...) { }

  TvnService::install();
}

void TvnService::reinstallportable(const TCHAR *commandLine /*=0 */, boolean shouldReinstall /*false*/)
{
	if ( shouldReinstall ) 
	{
	    try {
            removePortable();
	    }  catch (SCMClientException &scmEx) {
		    StringStorage errorMessage;
            errorMessage.setString(scmEx.getMessage());
	    } catch (SystemException &servEx) {
		    StringStorage errorMessage;
		    errorMessage.setString(servEx.getMessage());
	    } catch (Exception &ex) {
		    StringStorage errorMessage;
		    errorMessage.setString(ex.getMessage());
	    }
	}

    TvnService::installPortable(commandLine);
}

void TvnService::start(bool waitCompletion)
{
    startService(SERVICE_NAME, waitCompletion);
}

void TvnService::startPortable(bool waitCompletion)
{
    startService(SERVICE_PORTABLE_NAME, waitCompletion);
}


void TvnService::startService(const TCHAR* serviceName, bool waitCompletion)
{
    SCMClient scManager;

    scManager.startService(serviceName, waitCompletion);
}

void TvnService::stop(bool waitCompletion)
{
    stopService(SERVICE_NAME, waitCompletion);
}

void TvnService::stopPortable(bool waitCompletion)
{
    stopService(SERVICE_PORTABLE_NAME, waitCompletion);
}

void TvnService::stopService(const TCHAR* serviceName, bool waitCompletion)
{
    SCMClient scManager;

    scManager.stopService(serviceName, waitCompletion);
}

bool TvnService::getBinPath(StringStorage *binPath, const TCHAR* serviceName, const TCHAR *commandLine /*=0*/)
{
  StringStorage pathToServiceBinary;

  if (!Environment::getCurrentModuleFolderPath(&pathToServiceBinary)) {
    return false;
  }

  pathToServiceBinary.appendString(_T("\\tvnserver.exe"));
  boolean isRunPortable = _tcscmp(SERVICE_PORTABLE_NAME, serviceName) == 0;
  
  binPath->format(_T("\"%s\" %s %s"),
                  pathToServiceBinary.getString(),
                  isRunPortable == TRUE ? SERVICE_PORTABLE_COMMAND_LINE_KEY: SERVICE_COMMAND_LINE_KEY,
                  commandLine == 0 ? _T(""): commandLine);

  return true;
}

void TvnService::parse()
{
    CommandLineFormat fmt[] = {
        { ServerCommandLine::VNC_INI_DIRECTORY_PATH, NEEDS_ARG },
        { SERVICE_PORTABLE_COMMAND_LINE_KEY, NO_ARG }
    };
    
    CommandLine parser;

    if (!parser.parse(fmt, sizeof(fmt) / sizeof(CommandLineFormat), m_commandLine.getString())) {
        throw SystemException();
    }
    
    parser.optionSpecified(ServerCommandLine::VNC_INI_DIRECTORY_PATH, &m_vncIniDirectory);
}

boolean TvnService::isServiceRunning(const TCHAR* serviceName)
{
    SCMClient scManager;

    return scManager.isServiceRunning(serviceName);
}

