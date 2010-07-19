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

#ifndef __RFB_TIGHT_ENCODER_H_INCLUDED__
#define __RFB_TIGHT_ENCODER_H_INCLUDED__

#include "zlib/zlib.h"

#include "Encoder.h"
#include "TightPalette.h"
#include "JpegCompressor.h"

class TightEncoder : public Encoder
{
  friend class JpegEncoder;

public:
  TightEncoder(PixelConverter *conv, DataOutputStream *output);
  virtual ~TightEncoder();

  virtual int getCode() const;

  virtual void splitRectangle(const Rect *rect,
                              std::vector<Rect> *rectList,
                              const FrameBuffer *serverFb,
                              const EncodeOptions *options);

  virtual void sendRectangle(const Rect *rect,
                             const FrameBuffer *serverFb,
                             const EncodeOptions *options) throw(IOException);

protected:
  template <class PIXEL_T>
    void sendAnyRect(const Rect *rect,
                     const FrameBuffer *serverFb,
                     const FrameBuffer *clientFb,
                     const EncodeOptions *options) throw(IOException);

  void sendSolidRect(const Rect *r, const FrameBuffer *fb) throw(IOException);

  template <class PIXEL_T>
    void sendMonoRect(const Rect *rect,
                      const FrameBuffer *fb,
                      const EncodeOptions *options) throw(IOException);

  template <class PIXEL_T>
    void sendIndexedRect(const Rect *rect,
                         const FrameBuffer *fb,
                         const EncodeOptions *options) throw(IOException);

  template <class PIXEL_T>
    void sendFullColorRect(const Rect *rect,
                           const FrameBuffer *fb,
                           const EncodeOptions *options) throw(IOException);

  void sendJpegRect(const Rect *rect,
                    const FrameBuffer *serverFb,
                    const EncodeOptions *options) throw(IOException);

  bool shouldPackPixels(const PixelFormat *pf) const;

  static void packPixels(UINT8 *buf, int count, const PixelFormat *pf);

  template <class PIXEL_T>
    void fillPalette(const Rect *r, const FrameBuffer *fb, int maxColors);

  template <class PIXEL_T>
    void copyPixels(const Rect *rect, const FrameBuffer *fb, UINT8 *dst);

  template <class PIXEL_T>
    void encodeMonoRect(const Rect *rect, const FrameBuffer *fb,
                        DataOutputStream *out) throw(IOException);

  template <class PIXEL_T>
    void encodeIndexedRect(const Rect *rect, const FrameBuffer *fb,
                           DataOutputStream *out) throw(IOException);

  void sendCompressed(const char *data, size_t dataLen,
                      int streamId, int zlibLevel) throw(IOException);

  void sendCompactLength(size_t dataLen) throw(IOException);

  static const struct Conf {
    int maxRectSize;
    int maxRectWidth;
    int monoMinRectSize;
    int idxZlibLevel;
    int monoZlibLevel;
    int rawZlibLevel;
    int idxMaxColorsDivisor;
  } m_conf[10];

  static const Conf &getConf(const EncodeOptions *options);

  static const UINT8 SUBENCODING_FILL = 0x80;
  static const UINT8 SUBENCODING_JPEG = 0x90;
  static const UINT8 EXPLICIT_FILTER = 0x40;
  static const UINT8 FILTER_PALETTE = 0x01;

  static const int TIGHT_MIN_TO_COMPRESS = 12;

  static const int DEFAULT_COMPRESSION_LEVEL = 6;
  static const int JPEG_MIN_RECT_SIZE = 4096;
  static const int JPEG_MIN_RECT_WIDTH = 8;
  static const int JPEG_MIN_RECT_HEIGHT = 8;

  static const int NUM_ZLIB_STREAMS = 3;

  static const int ZLIB_STREAM_RAW = 0;
  static const int ZLIB_STREAM_MONO = 1;
  static const int ZLIB_STREAM_IDX = 2;

  z_stream m_zsStruct[NUM_ZLIB_STREAMS];

  bool m_zsActive[NUM_ZLIB_STREAMS];
  int m_zsLevel[NUM_ZLIB_STREAMS];

  TightPalette m_pal;

  StandardJpegCompressor m_compressor;
};

#endif 
