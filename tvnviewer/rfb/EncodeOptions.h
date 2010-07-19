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

#ifndef __RFB_ENCODE_OPTIONS_H_INCLUDED__
#define __RFB_ENCODE_OPTIONS_H_INCLUDED__

#include <vector>

class EncodeOptions
{
public:
  EncodeOptions();
  virtual ~EncodeOptions();

  void reset();

  void setEncodings(std::vector<int> *list);

  int getPreferredEncoding() const;

  bool encodingEnabled(int code) const;

  int getCompressionLevel(int defaultLevel = EO_DEFAULT) const;

  int getJpegQualityLevel(int defaultLevel = EO_DEFAULT) const;

  bool jpegEnabled() const;

  bool copyRectEnabled() const;
  bool richCursorEnabled() const;
  bool pointerPosEnabled() const;
  bool desktopSizeEnabled() const;

protected:

  static bool normalEncoding(int code);

  static const int EO_DEFAULT = -1;

  int m_preferredEncoding;

  bool m_enableHextile;
  bool m_enableTight;

  int m_compressionLevel;
  int m_jpegQualityLevel;

  bool m_enableCopyRect;
  bool m_enableRichCursor;
  bool m_enablePointerPos;
  bool m_enableDesktopSize;
};

#endif 
