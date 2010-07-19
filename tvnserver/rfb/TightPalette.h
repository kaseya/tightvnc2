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

#ifndef __RFB_TIGHTPALETTE_H_INCLUDED__
#define __RFB_TIGHTPALETTE_H_INCLUDED__

#include <string.h>
#include "util/inttypes.h"

struct TightColorList {
  TightColorList *next;
  int idx;
  UINT32 rgb;
};

struct TightPaletteEntry {
  TightColorList *listNode;
  int numPixels;
};

class TightPalette {

protected:

  inline static int hashFunc(UINT32 rgb) {
    return (rgb ^ (rgb >> 13)) & 0xFF;
  }

public:

  TightPalette(int maxColors = 254);

  void reset();

  void setMaxColors(int maxColors);

  int insert(UINT32 rgb, int numPixels);

  inline int getNumColors() const {
    return m_numColors;
  }

  inline UINT32 getEntry(int i) const {
    return (i < m_numColors) ? m_entry[i].listNode->rgb : (UINT32)-1;
  }

  inline int getCount(int i) const {
    return (i < m_numColors) ? m_entry[i].numPixels : 0;
  }

  inline UINT8 getIndex(UINT32 rgb) const {
    TightColorList *pnode = m_hash[hashFunc(rgb)];
    while (pnode != NULL) {
      if (pnode->rgb == rgb) {
        return (UINT8)pnode->idx;
      }
      pnode = pnode->next;
    }
    return 0xFF;  
  }

protected:

  int m_maxColors;
  int m_numColors;

  TightPaletteEntry m_entry[256];
  TightColorList *m_hash[256];
  TightColorList m_list[256];

};

#endif 
