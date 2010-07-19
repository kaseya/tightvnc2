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

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "util/CommonHeader.h"

#include "DeviceContext.h"
#include "Bitmap.h"
#include "Brush.h"
#include "Pen.h"

class Graphics
{
public:
  Graphics(DeviceContext *dc);
  virtual ~Graphics();

  void setBkMode(bool transparent);
  void setBkColor(COLORREF color);

  void setTextColor(COLORREF color);
  void setBrush(const Brush *brush);
  void setPen(const Pen *pen);

  void moveTo(int x, int y);
  void lineTo(int x, int y);

  void fillRect(int l, int t, int r, int b, const Brush *brush);
  void ellipse(int l, int t, int r, int b);
  void rectangle(int l, int t, int r, int b);

  void drawBitmap(const Bitmap *bitmap, int x, int y, int w, int h);
  void drawText(const TCHAR *text, int cchText, RECT *rect, UINT format);

protected:
  DeviceContext *m_dc;
};

#endif
