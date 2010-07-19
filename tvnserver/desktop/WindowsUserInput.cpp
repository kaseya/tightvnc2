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

#include "WindowsUserInput.h"
#include "thread/DesktopSelector.h"
#include "win-system/Environment.h"
#include "win-system/Keyboard.h"
#include "util/Log.h"

WindowsUserInput::WindowsUserInput(ClipboardListener *clipboardListener,
                                   bool ctrlAltDelEnabled)
: m_prevKeyFlag(0),
  m_keyEvent(ctrlAltDelEnabled)
{
  m_clipboard = new WindowsClipboard(clipboardListener);
}

WindowsUserInput::~WindowsUserInput(void)
{
  delete m_clipboard;
}

void WindowsUserInput::setMouseEvent(const Point *newPos, UINT8 keyFlag)
{
  DWORD dwFlags = 0;
  dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

  bool prevState;
  bool currState;
  prevState = (m_prevKeyFlag & 1) != 0;
  currState = (keyFlag       & 1) != 0;
  if (currState != prevState) {
    if (currState) {
      dwFlags |= MOUSEEVENTF_LEFTDOWN;
    } else {
      dwFlags |= MOUSEEVENTF_LEFTUP;
    }
  }
  prevState = (m_prevKeyFlag & 2) != 0;
  currState = (keyFlag       & 2) != 0;
  if (currState != prevState) {
    if (currState) {
      dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
    } else {
      dwFlags |= MOUSEEVENTF_MIDDLEUP;
    }
  }
  prevState = (m_prevKeyFlag & 4) != 0;
  currState = (keyFlag       & 4) != 0;
  if (currState != prevState) {
    if (currState) {
      dwFlags |= MOUSEEVENTF_RIGHTDOWN;
    } else {
      dwFlags |= MOUSEEVENTF_RIGHTUP;
    }
  }

  DWORD mouseWheelValue = 0;
  bool prevWheelUp = (m_prevKeyFlag & 8) != 0;
  bool currWheelUp = (keyFlag       & 8) != 0;
  bool prevWheelDown = (m_prevKeyFlag & 16) != 0;
  bool currWheelDown = (keyFlag       & 16) != 0;

  if (currWheelUp && !prevWheelUp) {
    dwFlags |= MOUSEEVENTF_WHEEL;
    mouseWheelValue = 120;
  } else if (currWheelDown && !prevWheelDown) {
    dwFlags |= MOUSEEVENTF_WHEEL;
    mouseWheelValue = -120;
  }

  m_prevKeyFlag = keyFlag;

  UINT16 desktopWidth = GetSystemMetrics(SM_CXSCREEN);
  UINT16 desktopHeight = GetSystemMetrics(SM_CYSCREEN);
  int fbOffsetX = GetSystemMetrics(SM_XVIRTUALSCREEN);
  int fbOffsetY = GetSystemMetrics(SM_YVIRTUALSCREEN);
  INT32 x = (INT32)((newPos->x + fbOffsetX) * 65535 / (desktopWidth - 1));
  INT32 y = (INT32)((newPos->y + fbOffsetY)* 65535 / (desktopHeight - 1));

  INPUT input;
  memset(&input, 0, sizeof(INPUT));
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = dwFlags;
  input.mi.dx = x;
  input.mi.dy = y;
  input.mi.mouseData = mouseWheelValue;
  SendInput(1, &input, sizeof(INPUT));
  DWORD error = GetLastError();
}

void WindowsUserInput::setNewClipboard(const StringStorage *newClipboard)
{
  m_clipboard->writeToClipBoard(newClipboard->getString());
}

void WindowsUserInput::setKeyboardEvent(UINT32 keySym, bool down)
{
  try {
    Log::info(_T("Received the %#4.4x keysym"), keySym);
    m_keyEvent.generate(keySym, !down);
  } catch (Exception &someEx) {
    Log::error(_T("Exception while processing key event: %s"), someEx.getMessage());
  }
}

void WindowsUserInput::getCurrentUserInfo(StringStorage *desktopName,
                                          StringStorage *userName)
{
  DesktopSelector::getCurrentDesktopName(desktopName);
  Environment::getCurrentUserName(userName);
}
