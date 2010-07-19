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

#ifndef __UPDATEFILTER_H__
#define __UPDATEFILTER_H__

#include "WindowsScreenGrabber.h"
#include "rfb/FrameBuffer.h"
#include "thread/LocalMutex.h"
#include "UpdateContainer.h"

class UpdateFilter
{
public:
  UpdateFilter(ScreenGrabber *screenGrabber,
               FrameBuffer *frameBuffer,
               LocalMutex *frameBufferCriticalSection);
  ~UpdateFilter(void);

  void filter(UpdateContainer *updateContainer);

private:
  void getChangedRegion(Region *rgn, const Rect *rect);
  void updateChangedRect(Region *rgn, const Rect *rect);
  void updateChangedSubRect(Region *rgn, const Rect *rect);

  ScreenGrabber *m_screenGrabber;
  FrameBuffer *m_frameBuffer;
  LocalMutex *m_fbMutex;
};

#endif 
