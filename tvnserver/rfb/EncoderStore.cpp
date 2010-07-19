// Copyright (C) 2009, 2010 Constantin Kaplinsky.
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

#include "EncoderStore.h"

#include "HextileEncoder.h"
#include "TightEncoder.h"

EncoderStore::EncoderStore(PixelConverter *pixelConverter, DataOutputStream *output)
: m_encoder(0),
  m_jpegEncoder(0),
  m_pixelConverter(pixelConverter),
  m_output(output)
{
}

EncoderStore::~EncoderStore()
{
  if (m_jpegEncoder != 0) {
    delete m_jpegEncoder;
  }
  std::map<int, Encoder *>::iterator it;
  for (it = m_map.begin(); it != m_map.end(); it++) {
    delete it->second;
  }
}

Encoder *EncoderStore::getEncoder() const
{
  return m_encoder;
}

JpegEncoder *EncoderStore::getJpegEncoder() const
{
  return m_jpegEncoder;
}

void EncoderStore::selectEncoder(int encType)
{
  m_encoder = validateEncoder(encType);
}

void EncoderStore::validateJpegEncoder()
{
  if (m_jpegEncoder == 0) {
    TightEncoder *tight = (TightEncoder *)validateEncoder(EncodingDefs::TIGHT);
    m_jpegEncoder = new JpegEncoder(tight);
  }
}

//---------------------------- Internal methods ----------------------------

Encoder *EncoderStore::validateEncoder(int encType)
{
  if (!encodingSupported(encType)) {
    encType = EncodingDefs::RAW;
  }
  std::map<int, Encoder *>::iterator it = m_map.find(encType);
  if (it != m_map.end()) {
    return it->second;
  }
  Encoder *newEncoder = allocateEncoder(encType);
  try {
    m_map[encType] = newEncoder;
  } catch (...) {
    delete newEncoder;
    throw;
  }
  return newEncoder;
}

bool EncoderStore::encodingSupported(int encType)
{
  return (encType == EncodingDefs::RAW ||
          encType == EncodingDefs::HEXTILE ||
          encType == EncodingDefs::TIGHT);
}

Encoder *EncoderStore::allocateEncoder(int encType) const
{
  switch (encType) {
  case EncodingDefs::HEXTILE:
    return new HextileEncoder(m_pixelConverter, m_output);
  case EncodingDefs::TIGHT:
    return new TightEncoder(m_pixelConverter, m_output);
  case EncodingDefs::RAW:
    return new Encoder(m_pixelConverter, m_output);
  default:
    throw Exception(_T("Cannot create encoder of the specified type"));
  }
}
