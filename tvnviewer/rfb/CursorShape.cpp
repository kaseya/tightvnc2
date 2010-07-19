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

#include "CursorShape.h"

CursorShape::~CursorShape()
{
  if (m_mask) {
    delete[] m_mask;
  }
}

bool CursorShape::clone(const CursorShape *srcCursorShape)
{
  m_hotSpot = srcCursorShape->getHotSpot();
  if (m_pixels.clone(srcCursorShape->getPixels())) {
    if (resizeBuffer()) {
      memcpy(m_mask, srcCursorShape->getMask(), srcCursorShape->getMaskSize());
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void CursorShape::assignMaskFromWindows(const char *srcMask)
{
  if (m_mask) {
    int height = m_pixels.getDimension().height;
    int winWidthInBytes = ((m_pixels.getDimension().width + 15) / 16) * 2;
    int rfbWidthInBytes = getMaskWidthInBytes();
    for (int i = 0; i < height; i++) {
      memcpy(&m_mask[i * rfbWidthInBytes],
             &srcMask[i * winWidthInBytes],
             rfbWidthInBytes);
    }
  }
}

bool CursorShape::setDimension(const Dimension *newDim)
{
  bool result = m_pixels.setDimension(newDim);
  return result && resizeBuffer();
}

bool CursorShape::setPixelFormat(const PixelFormat *pixFormat)
{
  bool result = m_pixels.setPixelFormat(pixFormat);
  return result && resizeBuffer();
}

bool CursorShape::setProperties(const Dimension *newDim,
                                const PixelFormat *pixelFormat)
{
  bool result = m_pixels.setDimension(newDim) &&
                m_pixels.setPixelFormat(pixelFormat);
  return result && resizeBuffer();
}

bool CursorShape::resizeBuffer()
{
  if (m_mask != 0) {
    delete[] m_mask;
  }
  m_mask = new char[getMaskSize()];

  return m_mask != 0;
}

int CursorShape::getMaskSize() const
{
  return getMaskWidthInBytes() * m_pixels.getDimension().height;
}

int CursorShape::getMaskWidthInBytes() const
{
  return (m_pixels.getDimension().width + 7) / 8;
}
