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

#ifndef _SPIN_CONTROL_H_
#define _SPIN_CONTROL_H_

#include "Control.h"
#include <commctrl.h>

class SpinControl : public Control
{
public:
  SpinControl();
  ~SpinControl();

  void setBuddy(Control *buddyControl);
  void setRange(short lower, short upper);
  void setRange32(int lower, int upper);
  void setAccel(UINT nSec, UINT nInc);

  void autoAccelerationHandler(LPNMUPDOWN message);
  void enableAutoAcceleration(bool enabled);
  void setAutoAccelerationParams(const int *limitters, const int *deltas, size_t size, int maxDelta);
protected:
  Control *m_buddy;

  bool m_isAutoAccelerationEnabled;
  int *m_limitters;
  int *m_deltas;
  size_t m_size; 
  int m_maxDelta;
};

#endif
