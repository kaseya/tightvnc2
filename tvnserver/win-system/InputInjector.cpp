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

#include "InputInjector.h"
#include "Keyboard.h"
#include "win-system/Environment.h"
#include <vector>

#include <crtdbg.h>

const BYTE InputInjector::EXTENDED_KEYS[] = { VK_MENU, VK_CONTROL, VK_INSERT, VK_DELETE, VK_HOME, VK_END,
                                              VK_PRIOR, VK_NEXT, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN, VK_NUMLOCK,
                                              VK_CANCEL, VK_RETURN, VK_DIVIDE, VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2,
                                              VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD7,
                                              VK_NUMPAD8, VK_NUMPAD9 };

InputInjector::InputInjector(bool ctrlAltDelEnabled)
: m_controlIsPressed(false),
  m_menuIsPressed(false),
  m_deleteIsPressed(false),
  m_shiftIsPressed(false),
  m_winIsPressed(false),
  m_ctrlAltDelEnabled(ctrlAltDelEnabled)
{
  resetModifiers();
}

void InputInjector::injectKeyPress(BYTE vkCode)
{
  injectKeyEvent(vkCode, false);
}

void InputInjector::injectKeyRelease(BYTE vkCode)
{
  injectKeyEvent(vkCode, true);
}

void InputInjector::injectKeyEvent(BYTE vkCode, bool release)
{
  if (vkCode == VK_CONTROL || vkCode == VK_RCONTROL || vkCode == VK_LCONTROL) {
    m_controlIsPressed = !release;
  }
  if (vkCode == VK_MENU || vkCode == VK_RMENU || vkCode == VK_LMENU) {
    m_menuIsPressed = !release;
  }
  if (vkCode == VK_DELETE) {
    m_deleteIsPressed = !release;
  }
  if (vkCode == VK_SHIFT || vkCode == VK_RSHIFT || vkCode == VK_LSHIFT) {
    m_shiftIsPressed = !release;
  }
  if (vkCode == VK_LWIN || vkCode == VK_RWIN) {
    m_winIsPressed = !release;
  }

  if (m_controlIsPressed && m_menuIsPressed && m_deleteIsPressed &&
      !m_winIsPressed && !m_shiftIsPressed) {
    if (m_ctrlAltDelEnabled) {
      Environment::simulateCtrlAltDel();
    } 
  } else {
    INPUT keyEvent = {0};

    keyEvent.type = INPUT_KEYBOARD;
    keyEvent.ki.wVk = vkCode;
    keyEvent.ki.wScan = MapVirtualKey(vkCode, 0);

    if (release) {
      keyEvent.ki.dwFlags = KEYEVENTF_KEYUP;
    }

    if (isExtendedKey(vkCode)) {
      keyEvent.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }

    if (SendInput(1, &keyEvent, sizeof(keyEvent)) == 0) {
      DWORD errCode = GetLastError();
      if (errCode != ERROR_SUCCESS) {
        throw SystemException(errCode);
      } else {
        throw Exception(_T("SendInput() function failed"));
      }
    }
  }
}

void InputInjector::injectCharEvent(WCHAR ch, bool release)
{
  bool ctrlOrAltPressed = m_controlIsPressed || m_menuIsPressed;
  SHORT vkKeyScanResult = 0;
  HKL hklCurrent = (HKL)0x04090409;
  try {
    hklCurrent = getCurrentKbdLayout();
    vkKeyScanResult = searchVirtKey(ch, hklCurrent);
  } catch (...) {
    if (ctrlOrAltPressed) {
      throw;
    }
    INPUT keyEvent = {0};

    keyEvent.type = INPUT_KEYBOARD;
    keyEvent.ki.wVk = 0;
    keyEvent.ki.wScan = ch;
    keyEvent.ki.dwFlags = KEYEVENTF_UNICODE;

    if (release) {
      keyEvent.ki.dwFlags |= KEYEVENTF_KEYUP;
    }

    if (SendInput(1, &keyEvent, sizeof(keyEvent)) == 0) {
      throw SystemException();
    }
    return;
  }
  bool controlSym;
  if (ch >= L'A' && ch <= L'Z' || ch >= L'a' && ch <= L'z') {
    controlSym = m_controlIsPressed && !m_menuIsPressed && !m_shiftIsPressed;
  } else {
    controlSym = false;
  }

  bool invariantToShift = isInvariantToShift((BYTE)vkKeyScanResult, hklCurrent);

  bool shiftedKey = (vkKeyScanResult >> 8 & 1) != 0 && !capsToggled();
  bool shiftPressNeeded = shiftedKey && !m_shiftIsPressed && !release &&
                          !controlSym;
  bool shiftUpNeeded = !shiftedKey && m_shiftIsPressed && !release &&
                       !controlSym && !invariantToShift;
  bool ctrlPressNeeded = (vkKeyScanResult >> 9 & 1) && !m_controlIsPressed &&
                         !release;
  bool altPressNeeded = (vkKeyScanResult >> 10 & 1) && !m_menuIsPressed &&
                        !release;
  if ((ctrlPressNeeded || altPressNeeded) &&
      (m_controlIsPressed || m_menuIsPressed)) {
    return;
  }

  if (ctrlPressNeeded) {
    injectKeyEvent(VK_CONTROL, false);
  }
  if (altPressNeeded) {
    injectKeyEvent(VK_MENU, false);
  }
  if (shiftPressNeeded) {
    injectKeyEvent(VK_SHIFT, false);
  } else if (shiftUpNeeded) {
    injectKeyEvent(VK_SHIFT, true);
  }
  injectKeyEvent(vkKeyScanResult & 255, release);
  if (shiftPressNeeded) {
    injectKeyEvent(VK_SHIFT, true);
  } else if (shiftUpNeeded) {
    injectKeyEvent(VK_SHIFT, false);
  }
  if (altPressNeeded) {
    injectKeyEvent(VK_MENU, true);
  }
  if (ctrlPressNeeded) {
    injectKeyEvent(VK_CONTROL, true);
  }
}

bool InputInjector::isExtendedKey(BYTE vkCode) {
  int i = sizeof(EXTENDED_KEYS);
  while (i-- > 0) {
    if (EXTENDED_KEYS[i] == vkCode) {
      return true;
    }
  }
  return false;
}

bool InputInjector::isAscii(WCHAR ch)
{
  if (ch >= 0 && ch < 128) {
    return true;
  }
  return false;
}

SHORT InputInjector::searchVirtKey(WCHAR ch, HKL hklCurrent)
{
  bool modifiersPressed = m_controlIsPressed || m_menuIsPressed ||
                          m_shiftIsPressed;
  bool onlyCtrlPressed = m_controlIsPressed && !m_menuIsPressed &&
                         !m_shiftIsPressed;
  SHORT vkKeyScanResult = VkKeyScanExW(ch, hklCurrent);
  if (vkKeyScanResult == -1) {
    if (onlyCtrlPressed) {
      if (ch >= L'A' && ch <= L'Z') {
        return (SHORT)ch;
      } else if (ch >= L'a' && ch <= L'z') {
        return (SHORT)(ch - L'a' + L'A');
      }
    }
    StringStorage errMess;
    errMess.format(_T("Can't translate the %d character to the scan code"),
                   (unsigned int)ch);
    throw Exception(errMess.getString());
  }
  if (isDeadKey(vkKeyScanResult, hklCurrent)) {
    throw Exception(_T("Special dead symbol must be inserted")
                    _T(" only as unicode character"));
  }
  if (!isOneKeyEventChar(ch, vkKeyScanResult, hklCurrent)) {
    StringStorage errMess;
    errMess.format(_T("Can't get the %d character by one keyboard event"),
                   (unsigned int)ch);
    throw Exception(errMess.getString());
  }
  return vkKeyScanResult;
}

bool InputInjector::capsToggled()
{
  return (GetKeyState(VK_CAPITAL) & 1) != 0;
}

bool InputInjector::isDeadKey(SHORT scanResult, HKL keyboardLayout)
{
  unsigned char kbdState[256];
  memset(kbdState, 0, sizeof(kbdState));
  WCHAR outBuff[20];
  bool withShift = (scanResult >> 8 & 1) != 0;
  bool withCtrl  = (scanResult >> 9 & 1) != 0;
  bool withAlt   = (scanResult >> 10 & 1) != 0;

  kbdState[VK_SHIFT]   = withShift ? 128 : 0;
  kbdState[VK_CONTROL] = withCtrl  ? 128 : 0;
  kbdState[VK_MENU]    = withAlt   ? 128 : 0;

  unsigned char virtKey = scanResult & 255;

  int count = ToUnicodeEx(virtKey, 0, kbdState, outBuff,
                          sizeof(outBuff) / sizeof(WCHAR),
                          0, keyboardLayout);
  bool result = count == -1;
  count = ToUnicodeEx(virtKey, 0, kbdState, outBuff,
                      sizeof(outBuff) / sizeof(WCHAR),
                      0, keyboardLayout);
  result = result || count == -1;
  return result;
}

bool InputInjector::isOneKeyEventChar(WCHAR ch, SHORT scanResult,
                                      HKL keyboardLayout)
{
  unsigned char kbdState[256];
  memset(kbdState, 0, sizeof(kbdState));
  WCHAR outBuff[20];
  bool withShift = (scanResult >> 8 & 1) != 0;
  bool withCtrl  = (scanResult >> 9 & 1) != 0;
  bool withAlt   = (scanResult >> 10 & 1) != 0;

  kbdState[VK_SHIFT]   = withShift ? 128 : 0;
  kbdState[VK_CONTROL] = withCtrl  ? 128 : 0;
  kbdState[VK_MENU]    = withAlt   ? 128 : 0;

  unsigned char virtKey = scanResult & 255;

  int count = ToUnicodeEx(virtKey, 0, kbdState, outBuff,
                          sizeof(outBuff) / sizeof(WCHAR),
                          0, keyboardLayout);
  if (count == 1) {
    return outBuff[0] == ch;
  } else {
    return false;
  }
}

HKL InputInjector::getCurrentKbdLayout()
{
  HWND hwnd = GetForegroundWindow();
  if (hwnd == 0) {
    throw Exception(_T("Can't insert key event because")
                    _T(" a window is losing activation"));
  }
  DWORD threadId = GetWindowThreadProcessId(hwnd, 0);
  return GetKeyboardLayout(threadId);
}

bool InputInjector::isInvariantToShift(BYTE virtKey, HKL keyboardLayout)
{
  unsigned char kbdState[256];
  memset(kbdState, 0, sizeof(kbdState));
  WCHAR outBuff1[20], outBuff2[20];

  int count1 = ToUnicodeEx(virtKey, 0, kbdState, outBuff1,
                           sizeof(outBuff1) / sizeof(WCHAR),
                           0, keyboardLayout);

  kbdState[VK_SHIFT] = 128;
  int count2 = ToUnicodeEx(virtKey, 0, kbdState, outBuff2,
                          sizeof(outBuff2) / sizeof(WCHAR),
                          0, keyboardLayout);
  if (count1 != count2) return false; 
  if (memcmp(outBuff1, outBuff2, count1 * sizeof(WCHAR)) != 0) {
    return false;
  } else {
    return true;
  }
}

void InputInjector::resetModifiers()
{
  injectKeyEvent(VK_MENU, true);
  injectKeyEvent(VK_LMENU, true);
  injectKeyEvent(VK_RMENU, true);
  injectKeyEvent(VK_SHIFT, true);
  injectKeyEvent(VK_LSHIFT, true);
  injectKeyEvent(VK_RSHIFT, true);
  injectKeyEvent(VK_CONTROL, true);
  injectKeyEvent(VK_LCONTROL, true);
  injectKeyEvent(VK_RCONTROL, true);
  injectKeyEvent(VK_LWIN, true);
  injectKeyEvent(VK_RWIN, true);
  injectKeyEvent(VK_DELETE, true);
}
