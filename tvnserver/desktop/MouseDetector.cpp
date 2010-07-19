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

#include "MouseDetector.h"
#include "util/Log.h"

#define MOUSE_SLEEP_TIME 10

MouseDetector::MouseDetector(UpdateKeeper *updateKeeper,
                             UpdateListener *updateListener)
: UpdateDetector(updateKeeper, updateListener)
{
  m_sleepTime = MOUSE_SLEEP_TIME;
}

MouseDetector::~MouseDetector(void)
{
}

Point MouseDetector::getCursorPos() const
{
  POINT curPoint;
  GetCursorPos(&curPoint);
  curPoint.x -= GetSystemMetrics(SM_XVIRTUALSCREEN);
  curPoint.y -= GetSystemMetrics(SM_YVIRTUALSCREEN);
  return Point(curPoint.x, curPoint.y);
}

void MouseDetector::execute()
{
  Log::info(_T("mouse detector thread id = %d"), getThreadId());

  Point curPoint;

  while (!isTerminating()) {
    curPoint = getCursorPos();
    if (!m_lastCursorPos.isEqualTo(&curPoint)) {
      m_lastCursorPos = curPoint;
      m_updateKeeper->setCursorPosChanged(&m_lastCursorPos);
      doUpdate();
    }
    Sleep(m_sleepTime);
  }
}
