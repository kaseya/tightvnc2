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

#ifndef __RFBKEYSYM_H__
#define __RFBKEYSYM_H__

#include "RfbKeySymListener.h"
#include "util/Keymap.h"

class RfbKeySym
{
public:
  RfbKeySym(RfbKeySymListener *extKeySymListener);
  virtual ~RfbKeySym();

  void processKeyEvent(unsigned short virtKey, unsigned int addKeyData);
  void processCharEvent(WCHAR charCode, unsigned int addKeyData);
  void processFocusRestoration();

  void sendCtrlAltDel();

private:
  void clearKeyState();

  void releaseModifier(unsigned char modifier);
  void restoreModifier(unsigned char modifier);
  void releaseModifiers();
  void restoreModifiers();

  bool isPressed(unsigned char virtKey);

  void checkAndSendDiff(unsigned char virtKey, unsigned char state);

  virtual void sendKeySymEvent(unsigned short rfbKeySym, bool down);

  RfbKeySymListener *m_extKeySymListener;

  unsigned char m_viewerKeyState[256];

  unsigned char m_serverKeyState[256];

  Keymap m_keyMap;
  bool m_allowProcessCharEvent;
};

#endif 
