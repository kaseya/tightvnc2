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

#ifndef __UPDATEHANDLER_H__
#define __UPDATEHANDLER_H__

#include "UpdateContainer.h"
#include "UpdateKeeper.h"
#include "UpdateFilter.h"
#include "ScreenGrabber.h"
#include "WindowsMouseGrabber.h"
#include "rfb/FrameBuffer.h"
#include "thread/AutoLock.h"
#include "UpdateListener.h"
#include "UpdateDetector.h"
#include "CopyRectDetector.h"
#include "desktop-ipc/BlockingGate.h"

class UpdateHandler
{
public:
  UpdateHandler();
  virtual ~UpdateHandler(void);

  virtual void extract(UpdateContainer *updateContainer) = 0;

  virtual void setFullUpdateRequested(const Region *region) = 0;

  virtual bool checkForUpdates(Region *region) = 0;

  virtual void setExcludedRegion(const Region *excludedRegion) = 0;

  const FrameBuffer *getFrameBuffer() const { return &m_backupFrameBuffer; }
  const CursorShape *getCursorShape() const { return &m_cursorShape; }
  void getFrameBufferProp(Dimension *dim, PixelFormat *pf)
  {
    AutoLock al(&m_fbLocMut);
    *dim = m_backupFrameBuffer.getDimension();
    *pf = m_backupFrameBuffer.getPixelFormat();
  }

  void initFrameBuffer(const FrameBuffer *newFb);

  virtual void sendInit(BlockingGate *gate) {}

protected:
  FrameBuffer m_backupFrameBuffer;
  LocalMutex m_fbLocMut;

  CursorShape m_cursorShape;
};

#endif 
