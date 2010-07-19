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

#include "SpinControl.h"
#include <CommCtrl.h>
#include "util/StringParser.h"

SpinControl::SpinControl()
: m_buddy(NULL), m_limitters(NULL), m_deltas(NULL), m_size(0),
  m_isAutoAccelerationEnabled(false), m_maxDelta(0)
{
}

SpinControl::~SpinControl()
{
  if (m_limitters != NULL) {
    delete[] m_limitters;
  }
  if (m_deltas != NULL) {
    delete[] m_deltas;
  }
}

void SpinControl::setBuddy(Control *buddyControl)
{
  SendMessage(m_hwnd, UDM_SETBUDDY, (WPARAM)buddyControl->getWindow(), NULL);
  m_buddy = buddyControl;
}

void SpinControl::setRange(short lower, short upper)
{
  SendMessage(m_hwnd, UDM_SETRANGE, NULL, (LPARAM)MAKELONG(upper, lower));
}

void SpinControl::setRange32(int lower, int upper)
{
  SendMessage(m_hwnd, UDM_SETRANGE32, lower, upper);
}

void SpinControl::setAccel(UINT nSec, UINT nInc)
{
  UDACCEL accel = {0};
  accel.nSec = nSec;
  accel.nInc = nInc;

  SendMessage(m_hwnd, UDM_SETACCEL, 1, (LPARAM)&accel);
}

void SpinControl::autoAccelerationHandler(LPNMUPDOWN message)
{
  if (m_size == 0 || m_limitters == NULL ||
      m_deltas == NULL || m_buddy == NULL || !m_isAutoAccelerationEnabled) {
    return ;
  }

  int currentValue;
  int delta = m_maxDelta;

  StringStorage storage;
  m_buddy->getText(&storage);
  if (!StringParser::parseInt(storage.getString(), &currentValue)) {
    return ;
  }

  if (message->iDelta < 0) {
    for (size_t i = 0; i < m_size; i++) {
      if (currentValue <= m_limitters[i]) {
        delta = m_deltas[i];
        break;
      } 
    } 
    delta = -delta;
  } 

  if (message->iDelta > 0) {
    for (size_t i = 0; i < m_size; i++) {
      if (currentValue < m_limitters[i]) {
        delta = m_deltas[i];
        break;
      } 
    } 
  } 

  int mod = (currentValue + delta) % delta;
  if (mod != 0) {
    delta -= mod;
  }

  message->iDelta = delta;
}

void SpinControl::enableAutoAcceleration(bool enabled)
{
  m_isAutoAccelerationEnabled = enabled;
}

void SpinControl::setAutoAccelerationParams(const int *limitters, const int *deltas, size_t size, int maxDelta)
{
  if (m_limitters != NULL) {
    delete[] m_limitters;
  }
  if (m_deltas != NULL) {
    delete[] m_deltas;
  }
  m_size = size;
  m_limitters = new int[m_size];
  m_deltas = new int[m_size];
  memcpy(m_limitters, limitters, sizeof(int) * size);
  memcpy(m_deltas, deltas, sizeof(int) * size);
  m_maxDelta = maxDelta;
}
