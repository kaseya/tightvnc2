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

#ifndef _INPUT_INJECTOR_H_
#define _INPUT_INJECTOR_H_

#include "util/CommonHeader.h"

#include "SystemException.h"

class InputInjector
{
public:
  InputInjector(bool ctrlAltDelEnabled);

  void resetModifiers();

  void injectKeyPress(BYTE vkCode);

  void injectKeyRelease(BYTE vkCode);

  void injectKeyEvent(BYTE vkCode, bool release) throw(SystemException);

  void injectCharEvent(WCHAR ch, bool release) throw(SystemException);

private:
  bool capsToggled();

  bool isDeadKey(SHORT scanResult, HKL keyboardLayout);

  bool isOneKeyEventChar(WCHAR ch, SHORT scanResult, HKL keyboardLayout);

  bool isInvariantToShift(BYTE virtKey, HKL keyboardLayout);

  bool isExtendedKey(BYTE vkCode);

  bool isAscii(WCHAR ch);

  HKL getCurrentKbdLayout();

  SHORT searchVirtKey(WCHAR ch, HKL hklCurrent);

  static const BYTE EXTENDED_KEYS[];

  bool m_controlIsPressed;
  bool m_menuIsPressed;
  bool m_deleteIsPressed;
  bool m_shiftIsPressed;
  bool m_winIsPressed;
  bool m_ctrlAltDelEnabled;
};

#endif
