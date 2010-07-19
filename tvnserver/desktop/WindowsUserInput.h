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

#ifndef __WINDOWSUSERINPUT_H__
#define __WINDOWSUSERINPUT_H__

#include "UserInput.h"
#include "WindowsClipboard.h"
#include "KeyEvent.h"

class WindowsUserInput : public UserInput
{
public:
  WindowsUserInput(ClipboardListener *clipboardListener,
                   bool ctrlAltDelEnabled);
  virtual ~WindowsUserInput(void);

  virtual void setNewClipboard(const StringStorage *newClipboard);
  virtual void setMouseEvent(const Point *newPos, UINT8 keyFlag);
  virtual void setKeyboardEvent(UINT32 keySym, bool down);

  virtual void getCurrentUserInfo(StringStorage *desktopName,
                                  StringStorage *userName);

  virtual void initKeyFlag(UINT8 initValue) { m_prevKeyFlag = initValue; }

protected:
  WindowsClipboard *m_clipboard;
  KeyEvent m_keyEvent;

  UINT8 m_prevKeyFlag;
};

#endif 
