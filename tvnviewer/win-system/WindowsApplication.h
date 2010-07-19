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

#ifndef __WINDOWSAPPLICATION_H__
#define __WINDOWSAPPLICATION_H__

#include "util/winhdr.h"
#include "thread/LocalMutex.h"

#include <list>

using namespace std;

class WindowsApplication
{
public:
  WindowsApplication(HINSTANCE appInstance);

  virtual ~WindowsApplication();

  virtual int run();

  virtual void shutdown();

  static void addModelessDialog(HWND dialogWindow);

  static void removeModelessDialog(HWND dialogWindow);

protected:
  HINSTANCE m_appInstance;
  HWND m_mainWindow;
private:
  static bool processDialogMessage(MSG *msg);

  static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
  static LocalMutex m_MDLMutex; 
  static list<HWND> m_modelessDialogList;
};

#endif 
