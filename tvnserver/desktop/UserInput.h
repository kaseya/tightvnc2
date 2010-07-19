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

#ifndef __USERINPUT_H__
#define __USERINPUT_H__

#include "util/CommonHeader.h"
#include "region/Point.h"
#include "desktop-ipc/BlockingGate.h"

class UserInput
{
public:
  UserInput();
  virtual ~UserInput();

  virtual void sendInit(BlockingGate *gate) {}

  virtual void setNewClipboard(const StringStorage *newClipboard) = 0;
  virtual void setMouseEvent(const Point *newPos, UINT8 keyFlag) = 0;
  virtual void setKeyboardEvent(UINT32 keySym, bool down) = 0;
  virtual void getCurrentUserInfo(StringStorage *desktopName,
                                  StringStorage *userName) = 0;
};

#endif 
