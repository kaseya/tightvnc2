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

#include "FrameBufferAccessor.h"
#include <crtdbg.h>

FrameBufferAccessor::FrameBufferAccessor()
: m_readLocked(false),
  m_fbForWrite(0),
  m_fbForRead(0)
{
}

FrameBufferAccessor::~FrameBufferAccessor()
{
  _ASSERT(m_fbForWrite == m_fbForRead); 
  if (m_fbForWrite) {
    delete m_fbForWrite;
  }
}

FrameBuffer *FrameBufferAccessor::getFbForWriting(const FrameBuffer *srcFb,
                                                  const Rect *viewPort)
{
  Dimension vpDim = Dimension(viewPort);
  PixelFormat srcPf = srcFb->getPixelFormat();
  if (m_fbForWrite == 0 ||
      !m_fbForWrite->getDimension().isEqualTo(&vpDim) || 
      !m_fbForWrite->getPixelFormat().isEqualTo(&srcPf)) {
    prepareToReconstruction(&vpDim, &srcPf);
  }
  return m_fbForWrite;
}

FrameBuffer *FrameBufferAccessor::getFbForReading()
{
  AutoLock al(&m_fbLocMut);
  return m_fbForRead;
}

void FrameBufferAccessor::prepareToReconstruction(const Dimension *newDim,
                                                  const PixelFormat *newPf)
{
  AutoLock al(&m_fbLocMut);

  if (m_fbForWrite == 0) { 
    m_fbForWrite = new FrameBuffer;
    m_fbForWrite->setProperties(newDim, newPf);
    _ASSERT(m_fbForRead == 0);
    m_fbForRead = m_fbForWrite; 
  } else if (m_readLocked) {
    if (m_fbForWrite != m_fbForRead) { 
      delete m_fbForWrite;
    }
    m_fbForWrite = new FrameBuffer;
    m_fbForWrite->setProperties(newDim, newPf);
  } else {
    delete m_fbForWrite;
    m_fbForWrite = new FrameBuffer;
    m_fbForWrite->setProperties(newDim, newPf);
    m_fbForRead = m_fbForWrite;
  }
}

void FrameBufferAccessor::lock()
{
  AutoLock al(&m_fbLocMut);
  _ASSERT(!m_readLocked);
  m_readLocked = true;
}

void FrameBufferAccessor::unlock()
{
  AutoLock al(&m_fbLocMut);
  _ASSERT(m_readLocked);
  if (m_fbForWrite != m_fbForRead) {
    _ASSERT(m_fbForWrite != 0);
    _ASSERT(m_fbForRead != 0);
    delete m_fbForRead;
    m_fbForRead = m_fbForWrite;
  }
  m_readLocked = false;
}
