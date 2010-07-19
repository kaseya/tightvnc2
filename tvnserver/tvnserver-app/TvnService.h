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
  static const TCHAR SERVICE_NAME[];
  static const TCHAR SERVICE_NAME_TO_DISPLAY[];
public:
  TvnService();
  virtual ~TvnService();

  virtual void onTvnServerShutdown();

  static void install() throw(SystemException);
  static void remove() throw(SystemException);
  static void reinstall() throw(SystemException);
  static void start(bool waitCompletion = false) throw(SystemException);
  static void stop(bool waitCompletion = false) throw(SystemException);

protected:
  virtual void onStart() throw(SystemException);

  virtual void main();

  virtual void onStop();

  static bool getBinPath(StringStorage *binPath);

protected:
  WindowsEvent m_shutdownEvent;
  TvnServer *m_tvnServer;
};

#endif
