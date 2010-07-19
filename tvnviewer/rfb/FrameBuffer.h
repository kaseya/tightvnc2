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

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include "region/Dimension.h"
#include "rfb/PixelFormat.h"

class FrameBuffer
{
public:
  FrameBuffer(void);
  virtual ~FrameBuffer(void);

  bool assignProperties(const FrameBuffer *srcFrameBuffer);
  bool clone(const FrameBuffer *srcFrameBuffer);
  void setColor(UINT8 reg, UINT8 green, UINT8 blue);

  bool isEqualTo(const FrameBuffer *frameBuffer);

  bool copyFrom(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                int srcX, int srcY);
  bool copyFrom(const FrameBuffer *srcFrameBuffer,
                int srcX, int srcY);
  void move(const Rect *dstRect, const int srcX, const int srcY);
  bool cmpFrom(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
               const int srcX, const int srcY);

  bool setDimension(const Dimension *newDim);
  bool setDimension(const Rect *rect)
  {
    Dimension dim(rect);
    return setDimension(&dim);
  }

  void setEmptyDimension(const Rect *dimByRect);

  void setEmptyPixelFmt(const PixelFormat *pf);

  inline Dimension getDimension() const { return m_dimension; }

  bool setPixelFormat(const PixelFormat *pixelFormat);
  inline PixelFormat getPixelFormat() const { return m_pixelFormat; }

  bool setProperties(const Dimension *newDim, const PixelFormat *pixelFormat);
  bool setProperties(const Rect *dimByRect, const PixelFormat *pixelFormat);

  size_t getBitsPerPixel() const;

  size_t getBytesPerPixel() const;

  void setBuffer(void *newBuffer) { m_buffer = newBuffer; }
  inline virtual void *getBuffer() const { return m_buffer; }

  virtual void *getBufferPtr(int x, int y) const;

  inline virtual int getBufferSize() const;
  inline int getBytesPerRow() const { return m_dimension.width *
                                             m_pixelFormat.bitsPerPixel / 8; }

protected:
  bool resizeBuffer();
  void clipRect(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                const int srcX, const int srcY,
                Rect *dstClippedRect, Rect *srcClippedRect);

  Dimension m_dimension;

  PixelFormat m_pixelFormat;
  void *m_buffer;
};

#endif 
