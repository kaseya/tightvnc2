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

#include "RfbKeySym.h"
#include "util/CommonHeader.h"
#include "util/Log.h"

#define XK_MISCELLANY
#include "rfb/keysymdef.h"

RfbKeySym::RfbKeySym(RfbKeySymListener *extKeySymListener)
: m_extKeySymListener(extKeySymListener),
  m_allowProcessCharEvent(false)
{
  clearKeyState();
}

RfbKeySym::~RfbKeySym()
{
}

void RfbKeySym::processKeyEvent(unsigned short virtKey,
                                unsigned int addKeyData)
{
  Log::debug(_T("processKeyEvent() function called: virtKey = %#4.4x, addKeyData")
             _T(" = %#x"), (unsigned int)virtKey, addKeyData);
  if (virtKey == VK_LWIN || virtKey == VK_RWIN) { 
    Log::debug(_T("Ignoring the Win key event"));
    return;
  }

  bool down = (addKeyData & 0x80000000) == 0;
  Log::debug(_T("down = %u"), (unsigned int)down);

  bool ctrlPressed = isPressed(VK_CONTROL) || isPressed(VK_RCONTROL);
  bool altPressed = isPressed(VK_MENU) || isPressed(VK_RMENU);
  bool shiftPressed = isPressed(VK_SHIFT) || isPressed(VK_RSHIFT);
  bool capsToggled = (GetKeyState(VK_CAPITAL) & 1) != 0;
  Log::debug(_T("ctrl = %u, alt = %u, shift = %u, caps toggled = %u"),
             (unsigned int)ctrlPressed,
             (unsigned int)altPressed,
             (unsigned int)shiftPressed,
             (unsigned int)capsToggled);

  m_viewerKeyState[virtKey & 255] = down ? 128 : 0;
  m_viewerKeyState[VK_CAPITAL & 255] = capsToggled ? 1 : 0;

  bool extended = (addKeyData & 0x1000000) != 0; 
  Log::debug(_T("extended = %u"), (unsigned int)extended);
  if (extended) {
    switch (virtKey) {
    case VK_CONTROL:
      virtKey = VK_RCONTROL;
      break;
    case VK_MENU:
      virtKey = VK_RMENU;
      break;
    }
  }

  m_serverKeyState[virtKey & 255] = down ? 128 : 0;

  UINT32 rfbSym;
  if (m_keyMap.virtualCodeToKeySym(&rfbSym, virtKey & 255)) {
    Log::debug(_T("The key has been mapped to the %#4.4x rfb symbol"),
               rfbSym);
    sendKeySymEvent(rfbSym, down);
  } else {
    WCHAR outBuff[20];
    HKL currentLayout = GetKeyboardLayout(0);

    int count = ToUnicodeEx(virtKey, 0, m_viewerKeyState, outBuff,
                            sizeof(outBuff) / sizeof(WCHAR),
                            0, currentLayout);
    if (count > 0) {
      count = ToUnicodeEx(virtKey, 0, m_viewerKeyState, outBuff,
                          sizeof(outBuff) / sizeof(WCHAR),
                          0, currentLayout);
    }

    Log::debug(_T("ToUnicodeEx() return %d"), count);

    if (count == 1 && !m_allowProcessCharEvent || count > 1) {
      bool ctrlAltPressed = ctrlPressed && altPressed;
      bool onlyCtrlPressed = ctrlPressed && !altPressed;
      if (ctrlAltPressed) {
        Log::debug(_T("Release the ctrl and alt")
                   _T(" modifiers before send the key event(s)"));
        releaseModifiers();
      }
      for (int i = 0; i < count; i++) {
        if (onlyCtrlPressed && outBuff[i] < 32) {
          if (onlyCtrlPressed && outBuff[i] >= 1 && outBuff[i] <= 26 &&
              !shiftPressed) {
            Log::debug(_T("The %u char is a control symbol then")
                       _T(" it will be increased by 96 to %u"),
                       (unsigned int)outBuff[i], (unsigned int)outBuff[i] + 96);
            outBuff[i] += 96;
          } else {
            Log::debug(_T("The %u char is a control symbol then")
                       _T(" it will be increased by 64 to %u"),
                       (unsigned int)outBuff[i], (unsigned int)outBuff[i] + 64);
            outBuff[i] += 64;
          }
        }
        if (m_keyMap.unicodeCharToKeySym(outBuff[i], &rfbSym)) {
          Log::debug(_T("Sending the %#4.4x rfb symbol"), rfbSym);
          sendKeySymEvent(rfbSym, down);
        } else {
          Log::error(_T("Can't translate the %#4.4x unicode character to an")
                     _T(" rfb symbol to send it"), (unsigned int)outBuff[i]);
        }
      }
      if (ctrlAltPressed) {
        Log::debug(_T("Restore the ctrl and alt")
                   _T(" modifiers after send the key event(s)"));
        restoreModifiers();
      }
    } else if (count == 0) {
      Log::debug(_T("Was get a not printable symbol then try get a printable")
                 _T(" with turned off the ctrl and alt modifiers"));
      unsigned char withoutCtrlAltKbdState[256];
      memcpy(withoutCtrlAltKbdState, m_serverKeyState, sizeof(withoutCtrlAltKbdState));
      withoutCtrlAltKbdState[VK_LCONTROL] = 0;
      withoutCtrlAltKbdState[VK_RCONTROL] = 0;
      withoutCtrlAltKbdState[VK_CONTROL] = 0;
      withoutCtrlAltKbdState[VK_LMENU] = 0;
      withoutCtrlAltKbdState[VK_RMENU] = 0;
      withoutCtrlAltKbdState[VK_MENU] = 0;
      count = ToUnicodeEx(virtKey, 0, withoutCtrlAltKbdState, outBuff,
                          sizeof(outBuff) / sizeof(WCHAR),
                          0, currentLayout);
      Log::debug(_T("ToUnicodeEx() without ctrl and alt return %d"), count);
      if (count == 1) { 
        if (m_keyMap.unicodeCharToKeySym(outBuff[0], &rfbSym)) {
          sendKeySymEvent(rfbSym, down);
        } else {
          Log::error(_T("Can't translate the %#4.4x unicode character to an")
                     _T(" rfb symbol to send it"), (unsigned int)outBuff[0]);
        }
      }
    } else if (count == -1 && down) {
      Log::debug(_T("Dead key pressed, wait for a char event"));
      m_allowProcessCharEvent = true;
    }
  }
}

void RfbKeySym::processCharEvent(WCHAR charCode,
                                 unsigned int addKeyData)
{
  if (m_allowProcessCharEvent) {
    Log::debug(_T("processCharEvent() function called with alowed processing:")
               _T(" charCode = %#4.4x, addKeyData = %#x"),
               (unsigned int)charCode, addKeyData);
    m_allowProcessCharEvent = false;
    UINT32 rfbSym;
    if (m_keyMap.unicodeCharToKeySym(charCode, &rfbSym)) {
      sendKeySymEvent(rfbSym, true);
      sendKeySymEvent(rfbSym, false);
    } else {
      Log::error(_T("Can't translate the %#4.4x unicode character to an")
                 _T(" rfb symbol to send it"), (unsigned int)charCode);
    }
  }
}

void RfbKeySym::processFocusRestoration()
{
  Log::info(_T("Process focus restoration in the RfbKeySym class"));
  unsigned char kbdState[256];
  if (GetKeyboardState(kbdState) != 0) {
    checkAndSendDiff(VK_CONTROL, kbdState[VK_CONTROL]);
    checkAndSendDiff(VK_RCONTROL, kbdState[VK_RCONTROL]);
    checkAndSendDiff(VK_LCONTROL, kbdState[VK_LCONTROL]);
    checkAndSendDiff(VK_MENU, kbdState[VK_MENU]);
    checkAndSendDiff(VK_LMENU, kbdState[VK_LMENU]);
    checkAndSendDiff(VK_RMENU, kbdState[VK_RMENU]);
    checkAndSendDiff(VK_SHIFT, kbdState[VK_SHIFT]);
    checkAndSendDiff(VK_RSHIFT, kbdState[VK_RSHIFT]);
    checkAndSendDiff(VK_LSHIFT, kbdState[VK_LSHIFT]);
  }
}

void RfbKeySym::sendCtrlAltDel()
{
  releaseModifier(VK_RWIN);
  releaseModifier(VK_LWIN);
  releaseModifier(VK_SHIFT);
  releaseModifier(VK_RSHIFT);

  sendKeySymEvent(XK_Control_L, true);
  sendKeySymEvent(XK_Alt_L, true);
  sendKeySymEvent(XK_Delete, true);
  sendKeySymEvent(XK_Delete, false);
  sendKeySymEvent(XK_Alt_L, false);
  sendKeySymEvent(XK_Control_L, false);

  restoreModifier(VK_RSHIFT);
  restoreModifier(VK_SHIFT);
  restoreModifier(VK_LWIN);
  restoreModifier(VK_RWIN);
}

void RfbKeySym::releaseModifiers()
{
  releaseModifier(VK_CONTROL);
  releaseModifier(VK_RCONTROL);
  releaseModifier(VK_MENU);
  releaseModifier(VK_RMENU);
}

void RfbKeySym::restoreModifiers()
{
  restoreModifier(VK_CONTROL);
  restoreModifier(VK_RCONTROL);
  restoreModifier(VK_MENU);
  restoreModifier(VK_RMENU);
}

void RfbKeySym::releaseModifier(unsigned char modifier)
{
  UINT32 rfbSym;
  if (isPressed(modifier)) {
    bool success = m_keyMap.virtualCodeToKeySym(&rfbSym, modifier);
    _ASSERT(success);
    sendKeySymEvent(rfbSym, false);
  }
}

void RfbKeySym::restoreModifier(unsigned char modifier)
{
  UINT32 rfbSym;
  if (isPressed(modifier)) {
    bool success = m_keyMap.virtualCodeToKeySym(&rfbSym, modifier);
    _ASSERT(success);
    sendKeySymEvent(rfbSym, true);
  }
}

void RfbKeySym::checkAndSendDiff(unsigned char virtKey, unsigned char state)
{
  bool testedState = (state & 128) != 0;
  bool srvState = (m_serverKeyState[virtKey] & 128) != 0;
  m_serverKeyState[virtKey] = testedState ? 128 : 0;
  m_viewerKeyState[virtKey] = testedState ? 128 : 0;
  if (testedState != srvState) {
    UINT32 rfbSym;
    bool success = m_keyMap.virtualCodeToKeySym(&rfbSym, virtKey);
    _ASSERT(success);
    sendKeySymEvent(rfbSym, testedState);
  }
}

void RfbKeySym::clearKeyState()
{
  memset(m_viewerKeyState, 0, sizeof(m_viewerKeyState));
  memset(m_serverKeyState, 0, sizeof(m_serverKeyState));
}

bool RfbKeySym::isPressed(unsigned char virtKey)
{
  return m_serverKeyState[virtKey] != 0;
}

void RfbKeySym::sendKeySymEvent(unsigned short rfbKeySym, bool down)
{
  bool scrollLocked = (GetKeyState(VK_SCROLL) & 1) != 0;

  if (scrollLocked) {
    if (rfbKeySym == XK_Alt_L) {
      rfbKeySym = XK_Meta_L;
    } else if (rfbKeySym == XK_Alt_R) {
      rfbKeySym = XK_Meta_R;
    }
  }
  m_extKeySymListener->onRfbKeySymEvent(rfbKeySym, down);
}
