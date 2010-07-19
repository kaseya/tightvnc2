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

#ifndef __RFB_PIXEL_CONVERTER_H_INCLUDED__
#define __RFB_PIXEL_CONVERTER_H_INCLUDED__

#include "FrameBuffer.h"
#include "region/Point.h"

class PixelConverter
{
public:
  PixelConverter(void);
  virtual ~PixelConverter(void);

  virtual void convert(const Rect *rect, FrameBuffer *dstFb,
                       const FrameBuffer *srcFb) const;

  virtual const FrameBuffer *convert(const Rect *rect,
                                     const FrameBuffer *srcFb);

  virtual void setPixelFormats(const PixelFormat *dstPf,
                               const PixelFormat *srcPf);

  virtual size_t getSrcBitsPerPixel() const;

  virtual size_t getDstBitsPerPixel() const;

protected:
  void reset();

  void fillHexBitsTable(const PixelFormat *dstPf, const PixelFormat *srcPf);
  void fill32BitsTable(const PixelFormat *dstPf, const PixelFormat *srcPf);

  enum ConvertMode
  {
    NO_CONVERT,
    CONVERT_FROM_16,
    CONVERT_FROM_32
  };

  ConvertMode m_convertMode;
  UINT32 *m_hexBitsTable;
  UINT32 *m_redTable;
  UINT32 *m_grnTable;
  UINT32 *m_bluTable;

  PixelFormat m_srcFormat;
  PixelFormat m_dstFormat;

  FrameBuffer *m_dstFrameBuffer;
};

#endif 
