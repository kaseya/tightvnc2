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

#ifndef __RFB_ENCODER_H_INCLUDED__
#define __RFB_ENCODER_H_INCLUDED__

#include <vector>

#include "FrameBuffer.h"
#include "io-lib/IOException.h"
#include "network/RfbOutputGate.h"
#include "EncodingDefs.h"
#include "EncodeOptions.h"
#include "PixelConverter.h"

class Encoder
{
public:

  Encoder(PixelConverter *conv, DataOutputStream *output);

  virtual ~Encoder(void);

  virtual int getCode() const;

  virtual void splitRectangle(const Rect *rect,
                              std::vector<Rect> *rectList,
                              const FrameBuffer *serverFb,
                              const EncodeOptions *options);

  virtual void sendRectangle(const Rect *rect,
                             const FrameBuffer *serverFb,
                             const EncodeOptions *options) throw(IOException);

protected:

  PixelConverter *m_pixelConverter;

  DataOutputStream *m_output;

private:
  Encoder(const Encoder &other);
  Encoder &operator=(const Encoder &other);
};

#endif 
