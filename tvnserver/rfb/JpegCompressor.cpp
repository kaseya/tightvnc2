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

#include "JpegCompressor.h"

const int StandardJpegCompressor::ALLOC_CHUNK_SIZE = 65536;
const int StandardJpegCompressor::DEFAULT_JPEG_QUALITY = 75;

typedef struct {
  struct jpeg_destination_mgr pub;
  StandardJpegCompressor *_this;
} my_destination_mgr;

static void
init_destination(j_compress_ptr cinfo)
{
  my_destination_mgr *dest_ptr = (my_destination_mgr *)cinfo->dest;
  dest_ptr->_this->initDestination();
}

static boolean
empty_output_buffer (j_compress_ptr cinfo)
{
  my_destination_mgr *dest_ptr = (my_destination_mgr *)cinfo->dest;
  return (boolean)dest_ptr->_this->emptyOutputBuffer();
}

static void
term_destination (j_compress_ptr cinfo)
{
  my_destination_mgr *dest_ptr = (my_destination_mgr *)cinfo->dest;
  dest_ptr->_this->termDestination();
}

StandardJpegCompressor::StandardJpegCompressor()
  : m_quality(-1), 
    m_newQuality(DEFAULT_JPEG_QUALITY),
    m_outputBuffer(0),
    m_numBytesAllocated(0),
    m_numBytesReady(0)
{
  m_cinfo.err = jpeg_std_error(&m_jerr);
  jpeg_create_compress(&m_cinfo);

  my_destination_mgr *dest = new my_destination_mgr;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
  dest->_this = this;
  m_cinfo.dest = (jpeg_destination_mgr *)dest;

  m_cinfo.input_components = 3;
  m_cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&m_cinfo);

  m_cinfo.dct_method = JDCT_FASTEST;
}

StandardJpegCompressor::~StandardJpegCompressor()
{
  if (m_outputBuffer)
    free(m_outputBuffer);

  delete m_cinfo.dest;
  m_cinfo.dest = NULL;

  jpeg_destroy_compress(&m_cinfo);
}

void
StandardJpegCompressor::initDestination()
{
  if (!m_outputBuffer) {
    size_t newSize = ALLOC_CHUNK_SIZE;
    m_outputBuffer = (unsigned char *)malloc(newSize);
    m_numBytesAllocated = newSize;
  }

  m_numBytesReady = 0;
  m_cinfo.dest->next_output_byte = m_outputBuffer;
  m_cinfo.dest->free_in_buffer =  m_numBytesAllocated;
}

bool
StandardJpegCompressor::emptyOutputBuffer()
{
  size_t oldSize = m_numBytesAllocated;
  size_t newSize = oldSize + ALLOC_CHUNK_SIZE;

  m_outputBuffer = (unsigned char *)realloc(m_outputBuffer, newSize);
  m_numBytesAllocated = newSize;

  m_cinfo.dest->next_output_byte = &m_outputBuffer[oldSize];
  m_cinfo.dest->free_in_buffer = newSize - oldSize;

  return true;
}

void
StandardJpegCompressor::termDestination()
{
  m_numBytesReady = m_numBytesAllocated - m_cinfo.dest->free_in_buffer;
}

void
StandardJpegCompressor::setQuality(int level)
{
  if (level < 0) {
    level = 0;
  } else if (level > 100) {
    level = 100;
  }
  m_newQuality = level;
}

void
StandardJpegCompressor::resetQuality()
{
  m_newQuality = DEFAULT_JPEG_QUALITY;
}

void
StandardJpegCompressor::compress(const void *buf,
                                 const PixelFormat *fmt,
                                 int w, int h, int stride)
{
  bool useQuickConversion =
    (fmt->bitsPerPixel == 32 && fmt->colorDepth == 24 &&
     fmt->redMax == 255 && fmt->greenMax == 255 && fmt->blueMax == 255);

  m_cinfo.image_width = w;
  m_cinfo.image_height = h;

  if (m_newQuality != m_quality) {
    jpeg_set_quality(&m_cinfo, m_newQuality, true);
    m_quality = m_newQuality;
  }

  jpeg_start_compress(&m_cinfo, TRUE);

  const char *src = (const char *)buf;

  JSAMPLE *rgb = new JSAMPLE[w * 3 * 8];
  JSAMPROW rowPointer[8];
  for (int i = 0; i < 8; i++)
    rowPointer[i] = &rgb[w * 3 * i];

  while (m_cinfo.next_scanline < m_cinfo.image_height) {
    int maxRows = m_cinfo.image_height - m_cinfo.next_scanline;
    if (maxRows > 8) {
      maxRows = 8;
    }
    for (int dy = 0; dy < maxRows; dy++) {
      if (useQuickConversion) {
        convertRow24(rowPointer[dy], src, fmt, w);
      } else {
        convertRow(rowPointer[dy], src, fmt, w);
      }
      src += stride;
    }
    jpeg_write_scanlines(&m_cinfo, rowPointer, maxRows);
  }

  delete[] rgb;

  jpeg_finish_compress(&m_cinfo);
}

size_t StandardJpegCompressor::getOutputLength()
{
  return m_numBytesReady;
}

const char *StandardJpegCompressor::getOutputData()
{
  return (const char *)m_outputBuffer;
}

void
StandardJpegCompressor::convertRow24(JSAMPLE *dst, const void *src,
                                     const PixelFormat *fmt, int numPixels)
{
  const UINT32 *srcPixels = (const UINT32 *)src;
  while (numPixels--) {
    UINT32 pixel = *srcPixels++;
    *dst++ = (JSAMPLE)(pixel >> fmt->redShift);
    *dst++ = (JSAMPLE)(pixel >> fmt->greenShift);
    *dst++ = (JSAMPLE)(pixel >> fmt->blueShift);
  }
}

void
StandardJpegCompressor::convertRow(JSAMPLE *dst, const void *src,
                                   const PixelFormat *fmt, int numPixels)
{
  if (fmt->bitsPerPixel == 32) {
    const UINT32 *srcPixels = (const UINT32 *)src;
    for (int x = 0; x < numPixels; x++) {
      UINT32 pixel = *srcPixels++;
      *dst++ = (JSAMPLE)((pixel >> fmt->redShift & fmt->redMax) * 255 / fmt->redMax);
      *dst++ = (JSAMPLE)((pixel >> fmt->greenShift & fmt->greenMax) * 255 / fmt->greenMax);
      *dst++ = (JSAMPLE)((pixel >> fmt->blueShift & fmt->blueMax) * 255 / fmt->blueMax);
    }
  } else { 
    const UINT16 *srcPixels = (const UINT16 *)src;
    for (int x = 0; x < numPixels; x++) {
      UINT16 pixel = *srcPixels++;
      *dst++ = (JSAMPLE)((pixel >> fmt->redShift & fmt->redMax) * 255 / fmt->redMax);
      *dst++ = (JSAMPLE)((pixel >> fmt->greenShift & fmt->greenMax) * 255 / fmt->greenMax);
      *dst++ = (JSAMPLE)((pixel >> fmt->blueShift & fmt->blueMax) * 255 / fmt->blueMax);
    }
  }
}
