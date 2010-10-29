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

#ifndef _TVN_SERVICE_H_
#define _TVN_SERVICE_H_

#include "TvnServer.h"
#include "TvnServerListener.h"

#include "win-system/Service.h"

#include "thread/Thread.h"

class TvnService : public Service,
                   public TvnServerListener
{
public:
  static const TCHAR SERVICE_COMMAND_LINE_KEY[];
  static const TCHAR SERVICE_PORTABLE_COMMAND_LINE_KEY[];

  static const TCHAR SERVICE_NAME[];
  static const TCHAR SERVICE_PORTABLE_NAME[];
  static const TCHAR SERVICE_NAME_TO_DISPLAY[];
  static const TCHAR SERVICE_PORTABLE_NAME_TO_DISPLAY[];

public:
  TvnService(const TCHAR* serviceName = SERVICE_NAME, const TCHAR *commandLine =0);
  virtual ~TvnService();

  virtual void onTvnServerShutdown();

  static void install() throw(SystemException);
  static void installPortable(const TCHAR *commandLine =0 ) throw(SystemException);

  static void remove() throw(SystemException);
  static void removePortable() throw(SystemException);

  static void reinstall() throw(SystemException);
  static void reinstallportable( const TCHAR *commandLine =0, boolean shouldReinstall = false)  throw(SystemException);

  
  static void start(bool waitCompletion = false) throw(SystemException);
  static void startPortable(bool waitCompletion = false) throw(SystemException);
  
  static void stop(bool waitCompletion = false) throw(SystemException);
  static void stopPortable(bool waitCompletion = false) throw(SystemException);

  static boolean isServiceRunning(const TCHAR* name) throw(SystemException);

protected:
  virtual void onStart() throw(SystemException);

  virtual void main();

  virtual void onStop();

  static bool getBinPath(StringStorage *binPath, const TCHAR* serviceName, const TCHAR *commandLine =0 );

private:
  void parse() throw(SystemException);

  static void installService( const TCHAR* serviceName, const TCHAR* serviceDisplayName, StringStorage binPath, DWORD startType = SERVICE_AUTO_START);
  static void removeService(const TCHAR* serviceName ) throw(SystemException);
  static void startService(const TCHAR* serviceName, bool waitCompletion = false) throw(SystemException);
  static void stopService(const TCHAR* serviceName, bool waitCompletion = false) throw(SystemException);

protected:
  WindowsEvent m_shutdownEvent;
  TvnServer *m_tvnServer;
  StringStorage m_commandLine;
  StringStorage m_vncIniDirectory;
};

#endif
