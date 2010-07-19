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

#ifndef __VIEWPORT_H__
#define __VIEWPORT_H__

#include "region/Rect.h"
#include "rfb/FrameBuffer.h"
#include "thread/AutoLock.h"
#include "thread/LocalMutex.h"

// 
class ViewPort
{
public:
  ViewPort();
  ~ViewPort();

  void update(const Dimension *fbDimension);

  Rect getViewPortRect();

  void setFullDesktop();
  void setArbitraryRect(const Rect *rect);

private:
  static const int FULL_DESKTOP = 0;
  static const int ARBITRARY_RECT = 2;
  int m_mode;
  Rect m_rect;
  Rect m_arbitraryRect;
  LocalMutex m_rectLocMut;
};

#endif 
