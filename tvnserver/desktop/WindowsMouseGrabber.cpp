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

#include "WindowsMouseGrabber.h"
#include "WindowsScreenGrabber.h"

WindowsMouseGrabber::WindowsMouseGrabber(void)
: m_lastHCursor(0)
{
}

WindowsMouseGrabber::~WindowsMouseGrabber(void)
{
}

bool WindowsMouseGrabber::isCursorShapeChanged()
{
  HCURSOR hCursor = getHCursor();
  if (hCursor == m_lastHCursor) {
    return false;
  }
  m_lastHCursor = hCursor;

  return true;
}

bool WindowsMouseGrabber::grab(PixelFormat *pixelFormat)
{
  return grabPixels(pixelFormat);
}

bool WindowsMouseGrabber::grabPixels(PixelFormat *pixelFormat)
{
  HCURSOR hCursor = getHCursor();
  if (hCursor == 0) {
    return false;
  }
  m_lastHCursor = hCursor;

  ICONINFO iconInfo;
  if (!GetIconInfo(hCursor, &iconInfo)) {
    return false;
  }

  if (iconInfo.hbmMask == NULL) {
    return false;
  }

  bool isColorShape = (iconInfo.hbmColor != NULL);

  BITMAP bmMask;
  if (!GetObject(iconInfo.hbmMask, sizeof(BITMAP), (LPVOID)&bmMask)) {
    DeleteObject(iconInfo.hbmMask);
    return false;
  }

  if (bmMask.bmPlanes != 1 || bmMask.bmBitsPixel != 1) {
    DeleteObject(iconInfo.hbmMask);
    return false;
  }

  m_cursorShape.setHotSpot(iconInfo.xHotspot, iconInfo.yHotspot);

  int width = bmMask.bmWidth;
  int height = isColorShape ? bmMask.bmHeight : bmMask.bmHeight/2;
  int widthBytes = bmMask.bmWidthBytes;

  const FrameBuffer *pixels= m_cursorShape.getPixels();

  m_cursorShape.setProperties(&Dimension(width, height), pixelFormat);

  std::vector<char> maskBuff(widthBytes * bmMask.bmHeight);
  char *mask = &maskBuff.front();

  bool result = GetBitmapBits(iconInfo.hbmMask,
                              widthBytes * bmMask.bmHeight,
                              mask) != 0;

  DeleteObject(iconInfo.hbmMask);
  if (!result) {
    return false;
  }

  HDC screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }

  WindowsScreenGrabber::BMI bmi;
  if (!WindowsScreenGrabber::getBMI(&bmi, screenDC)) {
    return false;
  }

  bmi.bmiHeader.biBitCount = pixelFormat->bitsPerPixel;
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biCompression = BI_BITFIELDS;
  bmi.red   = pixelFormat->redMax   << pixelFormat->redShift;
  bmi.green = pixelFormat->greenMax << pixelFormat->greenShift;
  bmi.blue  = pixelFormat->blueMax  << pixelFormat->blueShift;

  HDC destDC = CreateCompatibleDC(NULL);
  if (destDC == NULL) {
    DeleteDC(screenDC);
    return false;
  }

  void *buffer;
  HBITMAP hbmDIB = CreateDIBSection(destDC, (BITMAPINFO *) &bmi, DIB_RGB_COLORS, &buffer, NULL, NULL);
  if (hbmDIB == 0) {
    DeleteDC(destDC);
    DeleteDC(screenDC);
    return false;
  }

  HBITMAP hbmOld = (HBITMAP)SelectObject(destDC, hbmDIB);

  result = DrawIconEx(destDC, 0, 0, hCursor, 0, 0, 0, NULL, DI_IMAGE) != 0;

  memcpy(pixels->getBuffer(), buffer, pixels->getBufferSize());

  if (!isColorShape) {
    fixCursorShape(pixels, mask, mask + widthBytes * height);
  } else {
    inverse(mask, widthBytes * height);
    fixAlphaChannel(pixels, mask);
  }

  m_cursorShape.assignMaskFromWindows(mask);

  SelectObject(destDC, hbmOld);
  DeleteObject(hbmDIB);
  DeleteDC(destDC);
  DeleteDC(screenDC);

  return result;
}

HCURSOR WindowsMouseGrabber::getHCursor()
{
  CURSORINFO cursorInfo;
  cursorInfo.cbSize = sizeof(CURSORINFO);

  if (GetCursorInfo(&cursorInfo) == 0) {
    return false;
  }

  return cursorInfo.hCursor;
}

void WindowsMouseGrabber::inverse(char *bits, int count)
{
  for (int i = 0; i < count; i++, bits++) {
    *bits = ~*bits;
  }
}

void WindowsMouseGrabber::fixCursorShape(const FrameBuffer *pixels,
                                         char *maskAND, char *maskXOR)
{
  char *pixelsBuffer = (char *)pixels->getBuffer();
  char *pixel;
  int pixelSize = pixels->getBytesPerPixel();
  int pixelCount = pixels->getBufferSize() / pixelSize;

  int fbWidth = pixels->getDimension().width;
  int fbHeight = pixels->getDimension().height;
  int widthInBytes = ((fbWidth + 15) / 16) * 2;

  for (int iRow = 0; iRow < fbHeight; iRow++) {
    for (int iCol = 0; iCol < fbWidth; iCol++) {
      pixel = pixelsBuffer + (iRow * fbWidth + iCol) * pixelSize;

      char byteAnd = maskAND[iRow * widthInBytes + iCol / 8];
      char byteXor = maskXOR[iRow * widthInBytes + iCol / 8];

      bool maskANDBit = testBit(byteAnd, iCol % 8);
      bool maskXORBit = testBit(byteXor, iCol % 8);

      if (!maskANDBit && !maskXORBit) { 
        memset(pixel, 0, pixelSize);
      } else if (!maskANDBit && maskXORBit) { 
        memset(pixel, 0xff, pixelSize);
      } else if (maskANDBit && maskXORBit) { 
        memset(pixel, 0, pixelSize);
      }
    }
  }

  inverse(maskAND, widthInBytes * fbHeight);
  for (int i = 0; i < widthInBytes * fbHeight; i++) {
    *(maskAND +i) |= *(maskXOR + i);
  }
}

bool WindowsMouseGrabber::testBit(char byte, int index)
{
  char dummy = 128 >> index;
  return (dummy & byte) != 0;
}

void WindowsMouseGrabber::fixAlphaChannel(const FrameBuffer *pixels,
                                          char *maskAND)
{
  PixelFormat pf = pixels->getPixelFormat();
  if (pf.bitsPerPixel != 32) {
    return;
  }
  UINT32 alphaMax = pf.redMax << pf.redShift |
                    pf.greenMax << pf.greenShift |
                    pf.blueMax << pf.blueShift;
  alphaMax = ~alphaMax;
  UINT16 alphaShift = 0;
  for (alphaShift = 0; alphaShift < 32 && (alphaMax % 2) == 0 ; alphaShift++) {
    alphaMax = alphaMax >> 1;
  }
  if (alphaShift > 24) { 
    return;
  }

  UINT32 *pixelBuffer = (UINT32 *)pixels->getBuffer();
  int pixelSize = pixels->getBytesPerPixel();

  int fbWidth = pixels->getDimension().width;
  int fbHeight = pixels->getDimension().height;
  int widthInBytes = ((fbWidth + 15) / 16) * 2;

  for (int iRow = 0; iRow < fbHeight; iRow++) {
    for (int iCol = 0; iCol < fbWidth; iCol++) {
      UINT32 *pixel = &pixelBuffer[iRow * fbWidth + iCol];
      bool transparent = ((*pixel >> alphaShift) & alphaMax) < 128;
      if (transparent) {
        char *byteAnd = &maskAND[iRow * widthInBytes + iCol / 8];
        unsigned char curBit = 128 >> iCol % 8;
        curBit = ~curBit;
        *byteAnd = *byteAnd & curBit;
      }
    }
  }
}
