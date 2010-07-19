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

#include "WindowsScreenGrabber.h"

WindowsScreenGrabber::WindowsScreenGrabber(void)
: m_destDC(NULL), m_screenDC(NULL), m_hbmDIB(NULL), m_hbmOld(NULL)
{
  m_serverConfig = Configurator::getInstance()->getServerConfig();
  setWorkRectDefault();
  resume();
  m_hasStartedSignal.waitForEvent();
}

WindowsScreenGrabber::~WindowsScreenGrabber(void)
{
  closeDIBSection();
}

bool WindowsScreenGrabber::applyNewProperties()
{
  if (!applyNewPixelFormat() || !applyNewFullScreenRect() || !openDIBSection()) 
  {
    return false;
  }

  return true;
}

bool WindowsScreenGrabber::openDIBSection()
{
  closeDIBSection();

  m_workFrameBuffer.setBuffer(0);

  m_screenDC = GetDC(0);
  if (m_screenDC == NULL) {
    return false;
  }

  if (getPropertiesChanged()) {
    return false;
  }

  BMI bmi;
  if (!getBMI(&bmi, m_screenDC)) {
    return false;
  }

  PixelFormat pixelFormat = m_workFrameBuffer.getPixelFormat();
  Dimension workDim = m_workFrameBuffer.getDimension();

  bmi.bmiHeader.biBitCount = pixelFormat.bitsPerPixel;
  bmi.bmiHeader.biWidth = workDim.width;
  bmi.bmiHeader.biHeight = -workDim.height;
  bmi.bmiHeader.biCompression = BI_BITFIELDS;
  bmi.red   = pixelFormat.redMax   << pixelFormat.redShift;
  bmi.green = pixelFormat.greenMax << pixelFormat.greenShift;
  bmi.blue  = pixelFormat.blueMax  << pixelFormat.blueShift;

  m_destDC = CreateCompatibleDC(NULL);
  if (m_destDC == NULL) {
    DeleteDC(m_screenDC);
    return false;
  }

  void *buffer;
  m_hbmDIB = CreateDIBSection(m_destDC, (BITMAPINFO *) &bmi, DIB_RGB_COLORS, &buffer, NULL, NULL);
  if (m_hbmDIB == 0) {
    DeleteDC(m_destDC);
    DeleteDC(m_screenDC);
    return false;
  }
  m_workFrameBuffer.setBuffer(buffer);
  m_dibSectionDim = workDim;

  m_hbmOld = (HBITMAP) SelectObject(m_destDC, m_hbmDIB);

  return true;
}

bool WindowsScreenGrabber::closeDIBSection()
{
  SelectObject(m_destDC, m_hbmOld);

  DeleteObject(m_hbmDIB);
  m_hbmDIB = NULL;

  DeleteDC(m_destDC);
  m_destDC = NULL;

  DeleteDC(m_screenDC);
  m_screenDC = NULL;

  m_workFrameBuffer.setBuffer(NULL);;
  return true;
}

bool WindowsScreenGrabber::getPropertiesChanged()
{
  if (getScreenSizeChanged() || getPixelFormatChanged()) {
    return true;
  }

  return false;
}

bool WindowsScreenGrabber::getPixelFormatChanged()
{
  BMI bmi;
  if (!getBMI(&bmi)) {
    return false;
  }

  PixelFormat currentPF;
  PixelFormat frameBufferPF = m_workFrameBuffer.getPixelFormat();
  fillPixelFormat(&currentPF, &bmi);

  if (!frameBufferPF.isEqualTo(&currentPF)) {
    return true;
  }

  return false;
}

bool WindowsScreenGrabber::getScreenSizeChanged()
{
  BMI bmi;
  if (!getBMI(&bmi)) {
    return false;
  }

  int width = bmi.bmiHeader.biWidth;
  int height = bmi.bmiHeader.biHeight;

  if (width != m_fullScreenRect.getWidth() ||
      height != m_fullScreenRect.getHeight()) {
    return true;
  }

  int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
  int top = GetSystemMetrics(SM_YVIRTUALSCREEN);

  if (left != m_fullScreenRect.left||
      top != m_fullScreenRect.top) {
    applyNewProperties();
  }

  return false;
}

bool WindowsScreenGrabber::getBMI(BMI *bmi, HDC dc)
{
  HDC bitmapDC;
  if (dc == 0) {
    bitmapDC = GetDC(0);
    if (bitmapDC == NULL) {
      return false;
    }
  } else {
    bitmapDC = dc;
  }

  memset(bmi, 0, sizeof(BMI));
  bmi->bmiHeader.biBitCount = 0;
  bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

  HBITMAP hbm;
  hbm = (HBITMAP) GetCurrentObject(bitmapDC, OBJ_BITMAP);
  if (GetDIBits(bitmapDC, hbm, 0, 0, NULL, (LPBITMAPINFO) bmi, DIB_RGB_COLORS) == 0) {
    DeleteObject(hbm);
    DeleteDC(bitmapDC);
    return false;
  }

  if (bmi->bmiHeader.biCompression == BI_BITFIELDS) {
    if (GetDIBits(bitmapDC, hbm, 0, 0, NULL, (LPBITMAPINFO) bmi, DIB_RGB_COLORS) == 0) {
      DeleteObject(hbm);
      DeleteDC(bitmapDC);
      return false;
    }
  }

  DeleteObject(hbm);
  if (dc == 0) {
    DeleteDC(bitmapDC);
  }
  return true;
}

bool WindowsScreenGrabber::applyNewPixelFormat()
{
  BMI bmi;
  if (!getBMI(&bmi)) {
    return false;
  }

  PixelFormat pixelFormat;
  bool result = fillPixelFormat(&pixelFormat, &bmi);
  m_workFrameBuffer.setEmptyPixelFmt(&pixelFormat);

  return result;
}

bool WindowsScreenGrabber::applyNewFullScreenRect()
{
  m_fullScreenRect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
  m_fullScreenRect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
  m_fullScreenRect.setWidth(GetSystemMetrics(SM_CXVIRTUALSCREEN));
  m_fullScreenRect.setHeight(GetSystemMetrics(SM_CYVIRTUALSCREEN));

  setWorkRect(&m_fullScreenRect);

  return true;
}

bool WindowsScreenGrabber::grab(const Rect *rect)
{
  if (rect != NULL) {
    return grabByDIBSection(rect);
  }

  Rect grabRect;
  Dimension workDim = m_workFrameBuffer.getDimension();
  grabRect.left = 0;
  grabRect.top = 0;
  grabRect.setWidth(workDim.width);
  grabRect.setHeight(workDim.height);

  return grabByDIBSection(&grabRect);
}

bool WindowsScreenGrabber::grabByDIBSection(const Rect *rect)
{
  Dimension workDim = m_workFrameBuffer.getDimension();
  if (workDim.width != m_dibSectionDim.width ||
      workDim.height != m_dibSectionDim.height) {
    if (!openDIBSection()) {
      return false;
    }
  }

  DWORD bitBltFlag;
  if (m_serverConfig->getGrabTransparentWindowsFlag()) {
    bitBltFlag = SRCCOPY | CAPTUREBLT;
  } else {
    bitBltFlag = SRCCOPY;
  }

  if (BitBlt(m_destDC, rect->left, rect->top, rect->getWidth(), rect->getHeight(), 
             m_screenDC, rect->left + m_offsetFrameBuffer.x,
             rect->top + m_offsetFrameBuffer.y, bitBltFlag) == 0) {
    return false;
  }

  return !getPropertiesChanged();
}

bool WindowsScreenGrabber::fillPixelFormat(PixelFormat *pixelFormat, const BMI *bmi)
{
  memset(pixelFormat, 0, sizeof(PixelFormat));

  pixelFormat->bitsPerPixel = bmi->bmiHeader.biBitCount;

  if (bmi->bmiHeader.biCompression == BI_BITFIELDS) {
    pixelFormat->redShift   = findFirstBit(bmi->red);
    pixelFormat->greenShift = findFirstBit(bmi->green);
    pixelFormat->blueShift  = findFirstBit(bmi->blue);

    pixelFormat->redMax   = bmi->red    >> pixelFormat->redShift;
    pixelFormat->greenMax = bmi->green  >> pixelFormat->greenShift;
    pixelFormat->blueMax  = bmi->blue   >> pixelFormat->blueShift;

  } else {
    pixelFormat->bitsPerPixel = 32;
    pixelFormat->colorDepth = 24;
    pixelFormat->redMax = pixelFormat->greenMax = pixelFormat->blueMax = 0xff;
    pixelFormat->redShift   = 16;
    pixelFormat->greenShift = 8;
    pixelFormat->blueShift  = 0;
  }

  if (pixelFormat->bitsPerPixel == 32) {
    pixelFormat->colorDepth = 24;
  } else {
    pixelFormat->colorDepth = 16;
  }

  return (pixelFormat->redMax > 0)
    && (pixelFormat->greenMax > 0)
    && (pixelFormat->blueMax > 0);
}

int WindowsScreenGrabber::findFirstBit(const UINT32 bits)
{
  UINT32 b = bits;
  int shift;

  for (shift = 0; (shift < 32) && ((b & 1) == 0); shift++) {
    b >>= 1;
  }

  return shift;
}

void WindowsScreenGrabber::execute()
{
  applyNewProperties();
  m_hasStartedSignal.notify();
  m_threadStopper.waitForEvent();
}

void WindowsScreenGrabber::onTerminate()
{
  m_threadStopper.notify();
}
