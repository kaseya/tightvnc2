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

#include "WindowsApplication.h"

#include "util/CommonHeader.h"
#include "thread/AutoLock.h"

LocalMutex WindowsApplication::m_MDLMutex;

list<HWND> WindowsApplication::m_modelessDialogList;

WindowsApplication::WindowsApplication(HINSTANCE appInstance)
: m_appInstance(appInstance), m_mainWindow(0)
{
}

WindowsApplication::~WindowsApplication()
{
}

int WindowsApplication::run()
{
  WNDCLASS wndClass;
  memset(&wndClass, 0, sizeof(wndClass));

  wndClass.lpfnWndProc = wndProc;
  wndClass.hInstance = m_appInstance;
  wndClass.lpszClassName = _T("TvnWindowsApplicationClass");

  RegisterClass(&wndClass);

  m_mainWindow = CreateWindow(wndClass.lpszClassName,
                              0, 0,
                              0, 0, 0, 0,
                              HWND_MESSAGE, 0,
                              m_appInstance,
                              0);

  MSG msg;
  BOOL ret;
  while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
    if (ret < 0) {
      return 1;
    }
    if (!processDialogMessage(&msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return msg.wParam;
}

void WindowsApplication::shutdown()
{
  PostMessage(m_mainWindow, WM_CLOSE, 0, 0);
}

void WindowsApplication::addModelessDialog(HWND dialogWindow)
{
  AutoLock l(&m_MDLMutex);

  m_modelessDialogList.push_back(dialogWindow);
}

void WindowsApplication::removeModelessDialog(HWND dialogWindow)
{
  AutoLock l(&m_MDLMutex);

  m_modelessDialogList.remove(dialogWindow);
}

bool WindowsApplication::processDialogMessage(MSG *msg)
{
  AutoLock l(&m_MDLMutex);
  for (list<HWND>::iterator iter = m_modelessDialogList.begin();
       iter != m_modelessDialogList.end(); iter++) {
    if (IsDialogMessage(*iter, msg)) {
      return true;
    }
  }
  return false;
}

LRESULT CALLBACK WindowsApplication::wndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg) {
  case WM_CLOSE:
    DestroyWindow(hWnd);
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hWnd, msg, wparam, lparam);
}
