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

#include "util/CommonHeader.h"
#include "HooksUpdateDetector.h"
#include "region/Rect.h"
#include "util/Log.h"
#include "win-system/UipiControl.h"

const TCHAR HooksUpdateDetector::LIBRARY_NAME[] = _T("screenhooks.dll");
const char HooksUpdateDetector::SET_HOOK_FUNCTION_NAME[] = "setHook";
const char HooksUpdateDetector::UNSET_HOOK_FUNCTION_NAME[] = "unsetHook";

typedef bool (*SetHookFunction)(HWND targedWinHwnd);
typedef bool (*UnsetHookFunction)();
const UINT specIpcCode = RegisterWindowMessage(_T("HOOK.MESSAGE.CODE"));

HooksUpdateDetector::HooksUpdateDetector(UpdateKeeper *updateKeeper,
                                         UpdateListener *updateListener)
: UpdateDetector(updateKeeper, updateListener),
  m_updateTimer(updateListener),
  m_pSetHook(0),
  m_pUnSetHook(0),
  m_targetWin(0),
  m_lib(0)
{
  try {
    m_lib = new DynamicLibrary(LIBRARY_NAME);
  } catch (Exception &) {
    Thread::terminate();
    Log::error(_T("Failed to load library %s"), LIBRARY_NAME);
  }
  HINSTANCE hinst = GetModuleHandle(0);
  m_targetWin = new Window(hinst, _T("HookTargetWinClassName"));
}

HooksUpdateDetector::~HooksUpdateDetector()
{
  terminate();
  wait();

  if (m_targetWin != 0) {
    delete m_targetWin;
  }
  if (m_lib != 0) {
    delete m_lib;
  }
}

void HooksUpdateDetector::onTerminate()
{
  if (m_targetWin != 0) {
    PostMessage(m_targetWin->getHWND(), WM_QUIT, 0, 0);
  }
  m_initWaiter.notify();
}

bool HooksUpdateDetector::initHook()
{
  Log::info(_T("Try to initialize the hooks..."));
  HINSTANCE hinst = GetModuleHandle(0);

  if (m_lib != 0) {
    m_pSetHook = m_lib->getProcAddress(SET_HOOK_FUNCTION_NAME);
    m_pUnSetHook = m_lib->getProcAddress(UNSET_HOOK_FUNCTION_NAME);
  }
  if (!m_pSetHook || !m_pUnSetHook) {
    return false;
  }

  SetHookFunction setHookFunction = (SetHookFunction)m_pSetHook;
  return setHookFunction(m_targetWin->getHWND());
}

bool HooksUpdateDetector::unInitHook()
{
  bool result = true;

  if (m_pUnSetHook) {
    UnsetHookFunction unsetHookFunction = (UnsetHookFunction)m_pUnSetHook;
    result = unsetHookFunction();
    m_pUnSetHook = 0;
    m_pSetHook = 0;
  }

  return result;
}

void HooksUpdateDetector::execute()
{
  Log::info(_T("Hooks update detector thread id = %d"), getThreadId());

  if (!isTerminating() && m_targetWin != 0) {
    m_targetWin->createWindow();
    Log::info(_T("Hooks target window has been created (hwnd = %d)"),
              m_targetWin->getHWND());
  }

  try {
    UipiControl uipiControl;
    uipiControl.allowMessage(specIpcCode, m_targetWin->getHWND());
  } catch (Exception &e) {
    terminate();
    Log::error(e.getMessage());
  }

  while (!isTerminating() && !initHook()) {
    Log::error(_T("Hooks initialization failed, wait for the next trying"));
    m_initWaiter.waitForEvent(5000);
    unInitHook();
  }

  if (!isTerminating()) {
    Log::info(_T("Hooks update detector has been successfully initialized"));
  }

  MSG msg;
  while (!isTerminating()) {
    if (PeekMessage(&msg, m_targetWin->getHWND(), 0, 0, PM_REMOVE) != 0) {
      if (msg.message == specIpcCode) {
        Rect rect((INT16)(msg.wParam >> 16), (INT16)(msg.wParam & 0xffff),
                  (INT16)(msg.lParam >> 16), (INT16)(msg.lParam & 0xffff));
        if (!rect.isEmpty()) {
          m_updateKeeper->addChangedRect(&rect);
          m_updateTimer.sear();
        }
      } else {
        DispatchMessage(&msg);
      }
    } else {
      if (WaitMessage() == 0) {
        Log::error(_T("Hooks update detector has failed"));
        Thread::terminate();
      }
    }
  }

  unInitHook();
  Log::info(_T("Hooks update detector has been terminated."));
}
