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

#include "LocalUpdateHandler.h"
#include "Poller.h"
#include "ConsolePoller.h"
#include "HooksUpdateDetector.h"
#include "MouseShapeDetector.h"
#include "server-config-lib/Configurator.h"
#include "util/Log.h"
#include "gui/WindowFinder.h"

LocalUpdateHandler::LocalUpdateHandler(UpdateListener *externalUpdateListener)
: m_externalUpdateListener(externalUpdateListener),
  m_fullUpdateRequested(false)
{
  m_updateFilter = new UpdateFilter(&m_screenGrabber, &m_backupFrameBuffer,
                                    &m_fbLocMut);
  m_updateKeeper = new UpdateKeeper(&m_screenGrabber.getScreenBuffer()->
                                    getDimension().getRect());
  m_poller = new Poller(m_updateKeeper, this,
                        &m_screenGrabber,
                        &m_backupFrameBuffer, &m_fbLocMut);
  m_consolePoller = new ConsolePoller(m_updateKeeper, this,
                                      &m_screenGrabber,
                                      &m_backupFrameBuffer, &m_fbLocMut);
  m_hooks = new HooksUpdateDetector(m_updateKeeper, this);
  m_mouseDetector = new MouseDetector(m_updateKeeper, this);
  m_mouseShapeDetector = new MouseShapeDetector(m_updateKeeper, this,
                                                &m_mouseGrabber,
                                                &m_mouseGrabLocMut);

  executeDetectors();

  m_absoluteRect = m_screenGrabber.getScreenBuffer()->getDimension().getRect();
  m_updateKeeper->addChangedRect(&m_absoluteRect);
  doUpdate();
}

LocalUpdateHandler::~LocalUpdateHandler(void)
{
  terminateDetectors();
  delete m_mouseShapeDetector;
  delete m_mouseDetector;
  delete m_poller;
  delete m_hooks;
  delete m_updateKeeper;
  delete m_updateFilter;
}

void LocalUpdateHandler::extract(UpdateContainer *updateContainer)
{
  {
    AutoLock al(&m_fbLocMut);
    Rect copyRect;
    Point copySrc;
    m_copyRectDetector.detectWindowMovements(&copyRect, &copySrc);

    {
      AutoLock al(m_updateKeeper);
      m_updateKeeper->addCopyRect(&copyRect, &copySrc);
      m_updateKeeper->extract(updateContainer);
    }
    m_updateFilter->filter(updateContainer);

    updateVideoRegion();
    updateContainer->videoRegion = m_vidRegion;

    if (!m_absoluteRect.isEmpty()) {
      updateContainer->changedRegion.addRect(&m_screenGrabber.getScreenBuffer()->
                                             getDimension().getRect());
      m_absoluteRect.clear();
    }

    if (m_screenGrabber.getPropertiesChanged() ||
        !m_backupFrameBuffer.isEqualTo(m_screenGrabber.getScreenBuffer())) {
      if (m_screenGrabber.getScreenSizeChanged()) {
        updateContainer->screenSizeChanged = true;
      }
      m_screenGrabber.applyNewProperties();
      m_backupFrameBuffer.clone(m_screenGrabber.getScreenBuffer());
      updateContainer->changedRegion.clear();
      updateContainer->copiedRegion.clear();
      m_absoluteRect = m_backupFrameBuffer.getDimension().getRect();
      m_updateKeeper->setBorderRect(&m_absoluteRect);
    }
  }
  updateContainer->cursorPos = m_mouseDetector->getCursorPos();
  if (updateContainer->cursorShapeChanged || m_fullUpdateRequested) {
    AutoLock al(&m_mouseGrabLocMut);
    m_mouseGrabber.grab(&m_backupFrameBuffer.getPixelFormat());
    m_cursorShape.clone(m_mouseGrabber.getCursorShape());

    m_fullUpdateRequested = false;
  }
}

void LocalUpdateHandler::setFullUpdateRequested(const Region *region)
{
  m_updateKeeper->addChangedRegion(region);
  m_fullUpdateRequested = true;
}

void LocalUpdateHandler::executeDetectors()
{
  m_backupFrameBuffer.assignProperties(m_screenGrabber.getScreenBuffer());
  m_poller->resume();
  m_consolePoller->resume();
  m_hooks->resume();
  m_mouseDetector->resume();
  m_mouseShapeDetector->resume();
}

void LocalUpdateHandler::terminateDetectors()
{
  m_poller->terminate();
  m_consolePoller->terminate();
  m_hooks->terminate();
  m_mouseDetector->terminate();
  m_poller->wait();
  m_consolePoller->wait();
  m_hooks->wait();
  m_mouseDetector->wait();
}

void LocalUpdateHandler::onUpdate()
{
  AutoLock al(&m_fbLocMut);

  UpdateContainer updCont;
  m_updateKeeper->getUpdateContainer(&updCont);
  if (!updCont.isEmpty()) {
    doUpdate();
  }
}

bool LocalUpdateHandler::checkForUpdates(Region *region)
{
  UpdateContainer updateContainer;
  m_updateKeeper->getUpdateContainer(&updateContainer);

  Region resultRegion = updateContainer.changedRegion;
  resultRegion.add(&updateContainer.copiedRegion);
  resultRegion.intersect(region);

  bool result = updateContainer.cursorPosChanged ||
                updateContainer.cursorShapeChanged ||
                updateContainer.screenSizeChanged ||
                !resultRegion.isEmpty();

  return result;
}

void LocalUpdateHandler::setExcludedRegion(const Region *excludedRegion)
{
  m_updateKeeper->setExcludedRegion(excludedRegion);
}

void LocalUpdateHandler::updateVideoRegion()
{
  ServerConfig *srvConf = Configurator::getInstance()->getServerConfig();
  unsigned int interval = srvConf->getVideoRecognitionInterval();

  DateTime curTime = DateTime::now();
  if ((curTime - m_lastVidUpdTime).getTime() > interval) {
    m_lastVidUpdTime = DateTime::now();
    m_vidRegion.clear();
    AutoLock al(srvConf);
    StringVector *classNames = srvConf->getVideoClassNames();
    std::vector<HWND> hwndVector;
    std::vector<HWND>::iterator hwndIter;

    WindowFinder::findWindowsByClass(classNames, &hwndVector);

    for (hwndIter = hwndVector.begin(); hwndIter != hwndVector.end(); hwndIter++) {
      HWND videoHWND = *hwndIter;
      if (videoHWND != 0) {
        WINDOWINFO wi;
        wi.cbSize = sizeof(WINDOWINFO);
        if (GetWindowInfo(videoHWND, &wi)) {
          Rect videoRect(wi.rcClient.left, wi.rcClient.top,
                         wi.rcClient.right, wi.rcClient.bottom);
          videoRect.move(-GetSystemMetrics(SM_XVIRTUALSCREEN),
                         -GetSystemMetrics(SM_YVIRTUALSCREEN));
          m_vidRegion.addRect(&videoRect);
        }
      }
    }
  }
}
