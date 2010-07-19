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

#ifndef __FRAMEBUFFERACCESSOR_H__
#define __FRAMEBUFFERACCESSOR_H__

#include "rfb/FrameBuffer.h"
#include "thread/AutoLock.h"
#include "ViewPort.h"

class FrameBufferAccessor : public Lockable
{
public:
  FrameBufferAccessor();
  ~FrameBufferAccessor();

  FrameBuffer *getFbForWriting(const FrameBuffer *srcFb,
                               const Rect *viewPort);
  FrameBuffer *getFbForReading();

  virtual void lock();
  virtual void unlock();

private:
  void prepareToReconstruction(const Dimension *newDim,
                               const PixelFormat *newPf);

  bool m_readLocked;

  FrameBuffer *m_fbForWrite;
  FrameBuffer *m_fbForRead;
  LocalMutex m_fbLocMut;
};

#endif 
