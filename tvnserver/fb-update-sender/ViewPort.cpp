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

#include "ViewPort.h"

ViewPort::ViewPort()
: m_mode(FULL_DESKTOP)
{
}

ViewPort::~ViewPort()
{
}

void ViewPort::update(const Dimension *fbDimension)
{
  AutoLock al(&m_rectLocMut);

  Rect rect;
  switch(m_mode) {
  case FULL_DESKTOP:
    rect.setRect(&fbDimension->getRect());
    break;
  case ARBITRARY_RECT:
    rect = m_arbitraryRect;
    break;
  }
  m_rect = rect.intersection(&fbDimension->getRect());
}

Rect ViewPort::getViewPortRect()
{
  AutoLock al(&m_rectLocMut);
  return m_rect;
}

void ViewPort::setFullDesktop()
{
  AutoLock al(&m_rectLocMut);
  m_mode = FULL_DESKTOP;
}

void ViewPort::setArbitraryRect(const Rect *rect)
{
  AutoLock al(&m_rectLocMut);
  m_mode = ARBITRARY_RECT;
  m_arbitraryRect = *rect;
}
