// Copyright (C) 2000-2010 Constantin Kaplinsky.
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

#ifndef __RFB_JPEG_COMPRESSOR_H_INCLUDED__
#define __RFB_JPEG_COMPRESSOR_H_INCLUDED__

#include <stdio.h>

extern "C" {
#include "libjpeg/jpeglib.h"
}

#include "util/CommonHeader.h"
#include "PixelFormat.h"

class JpegCompressor
{
public:
  virtual ~JpegCompressor() {}

  virtual void setQuality(int level) = 0;
  virtual void resetQuality() = 0;

  virtual void compress(const void *buf, const PixelFormat *fmt,
                        int w, int h, int stride) = 0;

  virtual size_t getOutputLength() = 0;

  virtual const char *getOutputData() = 0;
};

class StandardJpegCompressor : public JpegCompressor
{
public:
  StandardJpegCompressor();
  virtual ~StandardJpegCompressor();

  virtual void setQuality(int level);
  virtual void resetQuality();

  virtual void compress(const void *buf, const PixelFormat *fmt,
                        int w, int h, int stride);

  virtual size_t getOutputLength();
  virtual const char *getOutputData();

public:
  void initDestination();
  bool emptyOutputBuffer();
  void termDestination();

protected:
  static const int ALLOC_CHUNK_SIZE;
  static const int DEFAULT_JPEG_QUALITY;

  int m_quality;
  int m_newQuality;

  struct jpeg_compress_struct m_cinfo;
  struct jpeg_error_mgr m_jerr;

  unsigned char *m_outputBuffer;
  size_t m_numBytesAllocated;
  size_t m_numBytesReady;

  void convertRow(JSAMPLE *dst, const void *src,
                  const PixelFormat *fmt, int numPixels);

  void convertRow24(JSAMPLE *dst, const void *src,
                    const PixelFormat *fmt, int numPixels);
};

#endif 
