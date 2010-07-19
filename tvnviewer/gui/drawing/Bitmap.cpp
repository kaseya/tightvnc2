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

#include "Bitmap.h"

Bitmap::Bitmap(size_t width, size_t height)
: m_bitmap(NULL)
{
  int bpp = 32;
  size_t size = width * height * (bpp / 8);
  unsigned char *bits = new unsigned char[size];
  memset(bits, 0, size);
  m_bitmap = CreateBitmap(width, height, 1, bpp, bits);
  delete []bits;
}

Bitmap::Bitmap(HDC dc, size_t width, size_t height)
{
  m_bitmap = CreateCompatibleBitmap(dc, width, height);
}

Bitmap::Bitmap(HBITMAP bitmap)
: m_bitmap(bitmap)
{
}

Bitmap::~Bitmap()
{
  if (m_bitmap != NULL) {
    DeleteObject(m_bitmap);
  }
}

size_t Bitmap::getWidth() const
{
  BITMAP bitmap;
  GetObject(m_bitmap, sizeof(BITMAP), &bitmap);
  return bitmap.bmWidth;
}

size_t Bitmap::getHeight() const
{
  BITMAP bitmap;
  GetObject(m_bitmap, sizeof(BITMAP), &bitmap);
  return bitmap.bmHeight;
}
