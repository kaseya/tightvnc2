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

#ifndef _TVN_SERVER_APPLICATION_H_
#define _TVN_SERVER_APPLICATION_H_

#include "util/CommonHeader.h"
#include "util/winhdr.h"

#include "win-system/WindowsApplication.h"

#include "TvnServer.h"
#include "TvnServerListener.h"
#include "WsConfigRunner.h"

class TvnServerApplication : public WindowsApplication,
                             public TvnServerListener
{
public:
  TvnServerApplication(HINSTANCE hInstance, const TCHAR *commandLine);
  virtual ~TvnServerApplication();

  virtual int run();

  virtual void onTvnServerShutdown();

private:
  StringStorage m_commandLine;
  TvnServer *m_tvnServer;
  WsConfigRunner *m_tvnControlRunner;
};

#endif
