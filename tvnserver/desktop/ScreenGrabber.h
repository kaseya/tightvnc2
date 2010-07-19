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

#ifndef __SCREENGRABBER_H__
#define __SCREENGRABBER_H__

#include "region/Rect.h"
#include "region/Dimension.h"
#include "region/Point.h"
#include "rfb/PixelFormat.h"
#include "rfb/FrameBuffer.h"

class ScreenGrabber
{
public:
  ScreenGrabber(void);
  virtual ~ScreenGrabber(void);

  virtual bool grab(const Rect *rect = 0) = 0;

  virtual FrameBuffer *getScreenBuffer() { return &m_workFrameBuffer; }
  virtual void setWorkRect(Rect *workRect);
  virtual Rect getWorkRect() const { return Rect(m_offsetFrameBuffer.x,
                                                 m_offsetFrameBuffer.y,
                                                 m_workFrameBuffer.getDimension().width +
                                                 m_offsetFrameBuffer.x,
                                                 m_workFrameBuffer.getDimension().height +
                                                 m_offsetFrameBuffer.y); }
  virtual Rect getScreenRect() { return m_fullScreenRect; }

  inline virtual bool getPropertiesChanged() = 0;
  inline virtual bool getPixelFormatChanged() = 0;
  inline virtual bool getScreenSizeChanged() = 0;

  virtual bool applyNewProperties();

protected:
  virtual bool applyNewFullScreenRect() = 0;
  virtual bool applyNewPixelFormat() = 0;

  virtual bool setWorkRectDefault();

  Rect m_fullScreenRect;
  Point m_offsetFrameBuffer;

  FrameBuffer m_workFrameBuffer;
};

#endif 
