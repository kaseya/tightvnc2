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

#include "FrameBuffer.h"
#include <string.h>

FrameBuffer::FrameBuffer(void)
: m_buffer(0)
{
  memset(&m_pixelFormat, 0, sizeof(m_pixelFormat));
}

FrameBuffer::~FrameBuffer(void)
{
  if (m_buffer != 0) {
    delete []m_buffer;
  }
}

bool FrameBuffer::assignProperties(const FrameBuffer *srcFrameBuffer)
{
  setProperties(&srcFrameBuffer->getDimension(),
                &srcFrameBuffer->getPixelFormat());
  return resizeBuffer();
}

bool FrameBuffer::clone(const FrameBuffer *srcFrameBuffer)
{
  if (!assignProperties(srcFrameBuffer)) {
    return false;
  }

  Rect fbRect = &m_dimension.getRect();
  copyFrom(&fbRect, srcFrameBuffer, fbRect.left, fbRect.top);

  return true;
}

void FrameBuffer::setColor(UINT8 red, UINT8 green, UINT8 blue)
{
  size_t sizeInPixels = m_dimension.area();
  int pixelSize = m_pixelFormat.bitsPerPixel / 8;
  UINT32 redPix = (red * m_pixelFormat.redMax / 255) <<
                  m_pixelFormat.redShift;
  UINT32 greenPix = (green * m_pixelFormat.greenMax / 255) <<
                    m_pixelFormat.greenShift;
  UINT32 bluePix = (blue * m_pixelFormat.blueMax / 255) <<
                   m_pixelFormat.blueShift;
  UINT32 color = redPix | greenPix | bluePix;

  UINT8 *endPixPtr = (UINT8 *)m_buffer + getBufferSize();
  UINT8 *pixPtr = (UINT8 *)m_buffer;
  for (; pixPtr < endPixPtr; pixPtr += pixelSize) {
    memcpy(pixPtr, &color, pixelSize);
  }
}

bool FrameBuffer::isEqualTo(const FrameBuffer *frameBuffer)
{
  return m_dimension.cmpDim(&frameBuffer->getDimension()) &&
         m_pixelFormat.isEqualTo(&frameBuffer->getPixelFormat());
}

void FrameBuffer::clipRect(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                           const int srcX, const int srcY,
                           Rect *dstClippedRect, Rect *srcClippedRect)
{
  Rect dstBufferRect = m_dimension.getRect();
  Rect srcBufferRect = srcFrameBuffer->getDimension().getRect();

  Rect srcRect(srcX, srcY, srcX + dstRect->getWidth(), srcY + dstRect->getHeight());

  Rect dstRectFB = dstBufferRect.intersection(dstRect);
  Rect srcRectFB = srcBufferRect.intersection(&srcRect);

  Rect dstCommonArea(&dstRectFB);
  Rect srcCommonArea(&srcRectFB);
  dstCommonArea.move(-dstRect->left, -dstRect->top);
  srcCommonArea.move(-srcRect.left, -srcRect.top);

  Rect commonRect(&dstCommonArea.intersection(&srcCommonArea));

  dstClippedRect->setRect(&commonRect);
  dstClippedRect->move(dstRect->left, dstRect->top);

  srcClippedRect->setRect(&commonRect);
  srcClippedRect->move(srcRect.left, srcRect.top);
}

bool FrameBuffer::copyFrom(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                           int srcX, int srcY)
{
  if (!m_pixelFormat.isEqualTo(&srcFrameBuffer->getPixelFormat())) {
    return false;
  }

  Rect srcClippedRect, dstClippedRect;

  clipRect(dstRect, srcFrameBuffer, srcX, srcY, &dstClippedRect, &srcClippedRect);
  if (dstClippedRect.area() <= 0 || srcClippedRect.area() <= 0) {
    return true;
  }

  int pixelSize = m_pixelFormat.bitsPerPixel / 8;
  int dstStrideBytes = m_dimension.width * pixelSize;
  int srcStrideBytes = srcFrameBuffer->getDimension().width * pixelSize;

  int resultHeight = dstClippedRect.getHeight();
  int resultWidthBytes = dstClippedRect.getWidth() * pixelSize;

  UINT8 *pdst = (UINT8 *)m_buffer
                + dstClippedRect.top * dstStrideBytes
                + pixelSize * dstClippedRect.left;

  UINT8 *psrc = (UINT8 *)srcFrameBuffer->getBuffer()
                + srcClippedRect.top * srcStrideBytes
                + pixelSize * srcClippedRect.left;

  for (int i = 0; i < resultHeight; i++, pdst += dstStrideBytes, psrc += srcStrideBytes) {
    memcpy(pdst, psrc, resultWidthBytes);
  }

  return true;
}

bool FrameBuffer::copyFrom(const FrameBuffer *srcFrameBuffer,
                           int srcX, int srcY)
{
  return copyFrom(&m_dimension.getRect(), srcFrameBuffer, srcX, srcY);
}

bool FrameBuffer::cmpFrom(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                          const int srcX, const int srcY)
{
  if (!m_pixelFormat.isEqualTo(&srcFrameBuffer->getPixelFormat())) {
    return false;
  }

  Rect srcClippedRect, dstClippedRect;

  clipRect(dstRect, srcFrameBuffer, srcX, srcY, &dstClippedRect, &srcClippedRect);
  if (dstClippedRect.area() <= 0 || srcClippedRect.area() <= 0) {
    return true;
  }

  int pixelSize = m_pixelFormat.bitsPerPixel / 8;
  int dstStrideBytes = m_dimension.width * pixelSize;
  int srcStrideBytes = srcFrameBuffer->getDimension().width * pixelSize;

  int resultHeight = dstClippedRect.getHeight();
  int resultWidthBytes = dstClippedRect.getWidth() * pixelSize;

  UINT8 *pdst = (UINT8 *)m_buffer
                + dstClippedRect.top * dstStrideBytes
                + pixelSize * dstClippedRect.left;

  UINT8 *psrc = (UINT8 *)srcFrameBuffer->getBuffer()
                + srcClippedRect.top * srcStrideBytes
                + pixelSize * srcClippedRect.left;

  for (int i = 0; i < resultHeight; i++, pdst += dstStrideBytes, psrc += srcStrideBytes) {
    if (memcmp(pdst, psrc, resultWidthBytes) != 0) {
      return false;
    }
  }

  return true;
}

void FrameBuffer::move(const Rect *dstRect, const int srcX, const int srcY)
{
  Rect srcClippedRect, dstClippedRect;

  clipRect(dstRect, this, srcX, srcY, &dstClippedRect, &srcClippedRect);
  if (dstClippedRect.area() <= 0 || srcClippedRect.area() <= 0) {
    return;
  }

  int pixelSize = m_pixelFormat.bitsPerPixel / 8;
  int strideBytes = m_dimension.width * pixelSize;

  int resultHeight = dstClippedRect.getHeight();
  int resultWidthBytes = dstClippedRect.getWidth() * pixelSize;

  UINT8 *pdst, *psrc;

  if (srcY > dstRect->top) {
    pdst = (UINT8 *)m_buffer + dstClippedRect.top * strideBytes
           + pixelSize * dstClippedRect.left;
    psrc = (UINT8 *)m_buffer + srcClippedRect.top * strideBytes
           + pixelSize * srcClippedRect.left;

    for (int i = 0; i < resultHeight; i++, pdst += strideBytes, psrc += strideBytes) {
      memcpy(pdst, psrc, resultWidthBytes);
    }

  } else {
    pdst = (UINT8 *)m_buffer + (dstClippedRect.bottom - 1) * strideBytes
           + pixelSize * dstClippedRect.left;
    psrc = (UINT8 *)m_buffer + (srcClippedRect.bottom - 1) * strideBytes
           + pixelSize * srcClippedRect.left;

    for (int i = resultHeight - 1; i >= 0; i--, pdst -= strideBytes, psrc -= strideBytes) {
      memmove(pdst, psrc, resultWidthBytes);
    }
  }
}

bool FrameBuffer::setPixelFormat(const PixelFormat *pixelFormat)
{
  m_pixelFormat = *pixelFormat;
  return resizeBuffer();
}

bool FrameBuffer::setDimension(const Dimension *newDim)
{
  m_dimension = *newDim;
  return resizeBuffer();
}

void FrameBuffer::setEmptyDimension(const Rect *dimByRect)
{
  m_dimension.setDim(dimByRect);
}

void FrameBuffer::setEmptyPixelFmt(const PixelFormat *pf)
{
  m_pixelFormat = *pf;
}

bool FrameBuffer::setProperties(const Dimension *newDim,
                                const PixelFormat *pixelFormat)
{
  m_pixelFormat = *pixelFormat;
  m_dimension = *newDim;
  return resizeBuffer();
}

bool FrameBuffer::setProperties(const Rect *dimByRect,
                                const PixelFormat *pixelFormat)
{
  m_pixelFormat = *pixelFormat;
  m_dimension.setDim(dimByRect);
  return resizeBuffer();
}

size_t FrameBuffer::getBitsPerPixel() const
{
  return (size_t)m_pixelFormat.bitsPerPixel;
}

size_t FrameBuffer::getBytesPerPixel() const
{
  return (size_t)(m_pixelFormat.bitsPerPixel / 8);
}

void *FrameBuffer::getBufferPtr(int x, int y) const
{
  char *ptr = (char *)m_buffer;
  ptr += (y * m_dimension.width + x) * getBytesPerPixel();

  return (void *)ptr;
}

int FrameBuffer::getBufferSize() const
{ 
  return (m_dimension.area() * m_pixelFormat.bitsPerPixel) / 8;
}

bool FrameBuffer::resizeBuffer()
{
  if (m_buffer != 0) {
    delete []m_buffer;
  }
  if ((m_buffer = new UINT8[getBufferSize()]) == 0) {
    return false;
  }
  return true;
}
