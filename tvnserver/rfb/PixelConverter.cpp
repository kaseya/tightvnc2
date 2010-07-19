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

#include "PixelConverter.h"
#include "util/inttypes.h"
#include <crtdbg.h>

PixelConverter::PixelConverter(void)
: m_convertMode(NO_CONVERT),
  m_hexBitsTable(0),
  m_redTable(0),
  m_grnTable(0),
  m_bluTable(0),
  m_dstFrameBuffer(0)
{
}

PixelConverter::~PixelConverter(void)
{
  reset();
}

void PixelConverter::convert(const Rect *rect, FrameBuffer *dstFb,
                             const FrameBuffer *srcFb) const
{
  if (m_convertMode == NO_CONVERT) {
    dstFb->copyFrom(rect, srcFb, rect->left, rect->top);
  } else {
    int rectHeight = rect->getHeight();
    int rectWidth = rect->getWidth();
    int fbWidth = dstFb->getDimension().width;
    PixelFormat dstPf = dstFb->getPixelFormat();
    PixelFormat srcPf = srcFb->getPixelFormat();

    UINT32 dstPixelSize = dstPf.bitsPerPixel / 8;
    UINT32 srcPixelSize = srcPf.bitsPerPixel / 8;

    UINT8 *dstPixP = (UINT8 *)dstFb->getBuffer() +
                     (fbWidth * rect->top + rect->left) * dstPixelSize;
    UINT8 *srcPixP = (UINT8 *)srcFb->getBuffer() +
                     (fbWidth * rect->top + rect->left) * srcPixelSize;
    if (m_convertMode == CONVERT_FROM_16) {
      for (int i = 0; i < rectHeight; i++,
           dstPixP += (fbWidth - rectWidth) * dstPixelSize,
           srcPixP += (fbWidth - rectWidth) * srcPixelSize) {
        for (int j = 0; j < rectWidth; j++,
                                       dstPixP += dstPixelSize,
                                       srcPixP += srcPixelSize) {
          UINT32 dstPixel = m_hexBitsTable[*(UINT16 *)srcPixP];
          if (dstPixelSize == 4) {
            *(UINT32 *)dstPixP = (UINT32)dstPixel;
          } else if (dstPixelSize == 2) {
            *(UINT16 *)dstPixP = (UINT16)dstPixel;
          } else if (dstPixelSize == 1) {
            *(UINT8 *)dstPixP = (UINT8)dstPixel;
          }
        }
      }
    } else if (m_convertMode == CONVERT_FROM_32) {
      UINT32 srcRedMax = srcPf.redMax;
      UINT32 srcGrnMax = srcPf.greenMax;
      UINT32 srcBluMax = srcPf.blueMax;

      for (int i = 0; i < rectHeight; i++,
           dstPixP += (fbWidth - rectWidth) * dstPixelSize,
           srcPixP += (fbWidth - rectWidth) * srcPixelSize) {
        for (int j = 0; j < rectWidth; j++,
                                       dstPixP += dstPixelSize,
                                       srcPixP += srcPixelSize) {
          UINT32 dstPixel = m_redTable[*(UINT32 *)srcPixP >>
                                       srcPf.redShift & srcRedMax] |
                            m_grnTable[*(UINT32 *)srcPixP >>
                                       srcPf.greenShift & srcGrnMax] |
                            m_bluTable[*(UINT32 *)srcPixP >>
                                       srcPf.blueShift & srcBluMax];
          if (dstPixelSize == 4) {
            *(UINT32 *)dstPixP = dstPixel;
          } else if (dstPixelSize == 2) {
            *(UINT16 *)dstPixP = dstPixel;
          } else if (dstPixelSize == 1) {
            *(UINT8 *)dstPixP = dstPixel;
          }
        }
      } 
    } else {
      _ASSERT(0);
    }
  }
}

const FrameBuffer *
PixelConverter::convert(const Rect *rect, const FrameBuffer *srcFb)
{
  if (m_convertMode == NO_CONVERT) {
    return srcFb;
  }

  const Dimension fbSize = srcFb->getDimension();
  if (m_dstFrameBuffer == 0) {
    m_dstFrameBuffer = new FrameBuffer;
    m_dstFrameBuffer->setProperties(&fbSize, &m_dstFormat);
  } else if (!m_dstFrameBuffer->getDimension().isEqualTo(&fbSize)) {
    m_dstFrameBuffer->setDimension(&fbSize);
  }

  convert(rect, m_dstFrameBuffer, srcFb);
  return m_dstFrameBuffer;
}

void PixelConverter::reset()
{
  if (m_hexBitsTable != 0) {
    delete[] m_hexBitsTable;
    m_hexBitsTable = 0;
  }
  if (m_redTable != 0) {
    delete[] m_redTable;
    m_redTable = 0;
  }
  if (m_grnTable != 0) {
    delete[] m_grnTable;
    m_grnTable = 0;
  }
  if (m_bluTable != 0) {
    delete[] m_bluTable;
    m_bluTable = 0;
  }

  if (m_dstFrameBuffer != 0) {
    delete m_dstFrameBuffer;
    m_dstFrameBuffer = 0;
  }
}

void PixelConverter::setPixelFormats(const PixelFormat *dstPf,
                                     const PixelFormat *srcPf)
{
  if (!srcPf->isEqualTo(&m_srcFormat) || !dstPf->isEqualTo(&m_dstFormat)) {
    reset();

    if (srcPf->isEqualTo(dstPf)) {
      m_convertMode = NO_CONVERT;
    } else if (srcPf->bitsPerPixel == 16) { 
      m_convertMode = CONVERT_FROM_16;
      fillHexBitsTable(dstPf, srcPf);
    } else if (srcPf->bitsPerPixel == 32) { 
      m_convertMode = CONVERT_FROM_32;
      fill32BitsTable(dstPf, srcPf);
    }

    m_srcFormat = *srcPf;
    m_dstFormat = *dstPf;
  }
}

size_t PixelConverter::getSrcBitsPerPixel() const
{
  return m_srcFormat.bitsPerPixel;
}

size_t PixelConverter::getDstBitsPerPixel() const
{
  return m_dstFormat.bitsPerPixel;
}

void PixelConverter::fillHexBitsTable(const PixelFormat *dstPf,
                                      const PixelFormat *srcPf)
{
  m_hexBitsTable = new UINT32[65536];

  UINT32 dstRedMax = dstPf->redMax;
  UINT32 dstGrnMax = dstPf->greenMax;
  UINT32 dstBluMax = dstPf->blueMax;

  UINT32 dstRedShift = dstPf->redShift;
  UINT32 dstGrnShift = dstPf->greenShift;
  UINT32 dstBluShift = dstPf->blueShift;

  UINT32 srcRedMax = srcPf->redMax;
  UINT32 srcGrnMax = srcPf->greenMax;
  UINT32 srcBluMax = srcPf->blueMax;

  UINT32 srcRedMask = srcRedMax << srcPf->redShift;
  UINT32 srcGrnMask = srcGrnMax << srcPf->greenShift;
  UINT32 srcBluMask = srcBluMax << srcPf->blueShift;

  for (UINT32 i = 0; i < 65536; i++) {
    UINT32 srcRed = (i & srcRedMask) >> srcPf->redShift;
    UINT32 srcGrn = (i & srcGrnMask) >> srcPf->greenShift;
    UINT32 srcBlu = (i & srcBluMask) >> srcPf->blueShift;

    UINT32 dstRed = (srcRed * dstRedMax / srcRedMax) << dstRedShift;
    UINT32 dstGrn = (srcGrn * dstGrnMax / srcGrnMax) << dstGrnShift;
    UINT32 dstBlu = (srcBlu * dstBluMax / srcBluMax) << dstBluShift;
    m_hexBitsTable[i] = dstRed | dstGrn | dstBlu;
  }
}

void PixelConverter::fill32BitsTable(const PixelFormat *dstPf,
                                     const PixelFormat *srcPf)
{
  UINT32 dstRedMax = dstPf->redMax;
  UINT32 dstGrnMax = dstPf->greenMax;
  UINT32 dstBluMax = dstPf->blueMax;

  UINT32 dstRedShift = dstPf->redShift;
  UINT32 dstGrnShift = dstPf->greenShift;
  UINT32 dstBluShift = dstPf->blueShift;

  UINT32 srcRedMax = srcPf->redMax;
  UINT32 srcGrnMax = srcPf->greenMax;
  UINT32 srcBluMax = srcPf->blueMax;

  m_redTable = new UINT32[srcRedMax + 1];
  m_grnTable = new UINT32[srcGrnMax + 1];
  m_bluTable = new UINT32[srcBluMax + 1];

  for (UINT32 i = 0; i <= srcRedMax; i++) {
    m_redTable[i] = ((i * dstRedMax + srcRedMax / 2) / srcRedMax) << dstRedShift;
  }
  for (UINT32 i = 0; i <= srcGrnMax; i++) {
    m_grnTable[i] = ((i * dstGrnMax + srcGrnMax / 2) / srcGrnMax) << dstGrnShift;
  }
  for (UINT32 i = 0; i <= srcBluMax; i++) {
    m_bluTable[i] = ((i * dstBluMax + srcBluMax / 2) / srcBluMax) << dstBluShift;
  }
}
