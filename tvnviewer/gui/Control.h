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

#ifndef __CONTROL_H_
#define __CONTROL_H_

#include "util/CommonHeader.h"
#include "util/StringStorage.h"

enum VerticalAlignment
{
  Left = 0x0,
  Center = 0x1,
  Right = 0x2
};

class Control
{
public:
  Control();
  Control(HWND hwnd);
  virtual ~Control();

  void setWindow(HWND hwnd);

  virtual void setEnabled(bool enabled);

  virtual void setText(const TCHAR *text);
  virtual void setSignedInt(int value);
  virtual void setUnsignedInt(unsigned int value);

  void setTextVerticalAlignment(VerticalAlignment align);

  void setFocus();

  bool hasFocus();

  bool setForeground();

  void setVisible(bool visible);

  virtual bool isEnabled();

  void invalidate();

  virtual void getText(StringStorage *storage);

  VerticalAlignment getTextVerticalAlignment();

  HWND getWindow();

protected:

  void setStyle(DWORD styleFlags);

  DWORD getStyle();

  void addStyle(DWORD styleFlag);

  void removeStyle(DWORD styleFlag);

  bool isStyleEnabled(DWORD styleFlags);

  void setExStyle(DWORD style);
  void addExStyle(DWORD styleFlag);
  void removeExStyle(DWORD styleFlag);

  DWORD getExStyle();
  bool isExStyleEnabled(DWORD styleFlag);

  void replaceWindowProc(WNDPROC wndProc);

protected:

  HWND m_hwnd;

  WNDPROC m_defWindowProc;
};

#endif
