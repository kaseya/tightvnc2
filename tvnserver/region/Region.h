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

#ifndef __REGION_REGION_H_INCLUDED__
#define __REGION_REGION_H_INCLUDED__

#include <vector>
#include <list>

#include "Rect.h"

extern "C" {
#include "x11region.h"
}

class Region {
public:
  Region();
  Region(const Rect *rect);
  Region(const Region &src);
  virtual ~Region();

  void clear();
  void set(const Region *src);
  Region & operator=(const Region &src);

  void addRect(const Rect *rect);
  void translate(int dx, int dy);
  void add(const Region *other);
  void subtract(const Region *other);
  void intersect(const Region *other);
  void crop(const Rect *rect);

  bool isEmpty() const;
  bool equals(const Region *other) const;

  void getRectVector(std::vector<Rect> *dst) const;
  void getRectList(std::list<Rect> *dst) const;

private:
  RegionRec m_reg;
};

#endif 
