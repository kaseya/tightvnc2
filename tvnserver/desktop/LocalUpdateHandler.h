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

#ifndef __LOCALUPDATEHANDLER_H__
#define __LOCALUPDATEHANDLER_H__

#include "UpdateHandler.h"
#include "UpdateListener.h"
#include "util/DateTime.h"
#include "MouseDetector.h"

class LocalUpdateHandler : public UpdateHandler, public UpdateListener
{
public:
  LocalUpdateHandler(UpdateListener *externalUpdateListener);
  virtual ~LocalUpdateHandler();

  virtual void extract(UpdateContainer *updateContainer);
  virtual void setFullUpdateRequested(const Region *region);
  bool checkForUpdates(Region *region);

  virtual void setExcludedRegion(const Region *excludedRegion);

protected:
  virtual void executeDetectors();
  virtual void terminateDetectors();

  void doUpdate()
  {
    if (m_externalUpdateListener) {
      m_externalUpdateListener->onUpdate();
    }
  }

  virtual void onUpdate();

  void updateVideoRegion();

  UpdateKeeper *m_updateKeeper;
  UpdateFilter *m_updateFilter;
  UpdateDetector *m_poller;
  UpdateDetector *m_consolePoller;
  UpdateDetector *m_hooks;
  MouseDetector *m_mouseDetector;
  CopyRectDetector m_copyRectDetector;
  WindowsScreenGrabber m_screenGrabber;
  WindowsMouseGrabber m_mouseGrabber;
  UpdateListener *m_externalUpdateListener;

  UpdateDetector *m_mouseShapeDetector;
  LocalMutex m_mouseGrabLocMut;

  DateTime m_lastVidUpdTime;
  Region m_vidRegion;

  Rect m_absoluteRect;

  bool m_fullUpdateRequested;
};

#endif 
