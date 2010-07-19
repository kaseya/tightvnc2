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

#include "UpdateHandlerServer.h"

UpdateHandlerServer::UpdateHandlerServer(BlockingGate *forwGate,
                                         DesktopSrvDispatcher *dispatcher,
                                         AnEventListener *extTerminationListener)
: DesktopServerProto(forwGate),
  IpcServer(dispatcher),
  m_extTerminationListener(extTerminationListener)
{
  m_updateHandler = new LocalUpdateHandler(this);

  m_dispatcher->registerNewHandle(EXTRACT_REQ, this);
  m_dispatcher->registerNewHandle(SCREEN_PROP_REQ, this);
  m_dispatcher->registerNewHandle(SET_FULL_UPD_REQ_REGION, this);
  m_dispatcher->registerNewHandle(SET_EXCLUDING_REGION, this);
  m_dispatcher->registerNewHandle(FRAME_BUFFER_INIT, this);
}

UpdateHandlerServer::~UpdateHandlerServer()
{
  m_dispatcher->unregisterHandle(EXTRACT_REQ);
  m_dispatcher->unregisterHandle(SCREEN_PROP_REQ);
  m_dispatcher->unregisterHandle(SET_FULL_UPD_REQ_REGION);
  m_dispatcher->unregisterHandle(SET_EXCLUDING_REGION);
  m_dispatcher->unregisterHandle(FRAME_BUFFER_INIT);

  delete m_updateHandler;
}

void UpdateHandlerServer::onUpdate()
{
  AutoLock al(m_forwGate);
  try {
    m_forwGate->writeUInt8(UPDATE_DETECTED);
  } catch (...) {
    m_extTerminationListener->onAnObjectEvent();
  }
}

void UpdateHandlerServer::onRequest(UINT8 reqCode, BlockingGate *backGate)
{
  switch (reqCode) {
  case EXTRACT_REQ:
    extractReply(backGate);
    break;
  case SCREEN_PROP_REQ:
    screenPropReply(backGate);
    break;
  case SET_FULL_UPD_REQ_REGION:
    receiveFullReqReg(backGate);
    break;
  case SET_EXCLUDING_REGION:
    receiveExcludingReg(backGate);
    break;
  case FRAME_BUFFER_INIT:
    serverInit(backGate);
    break;
  default:
    StringStorage errMess;
    errMess.format(_T("Unknown %d protocol code received from a pipe client"),
                   (int)reqCode);
    throw Exception(errMess.getString());
    break;
  }
}

void UpdateHandlerServer::extractReply(BlockingGate *backGate)
{
  UpdateContainer updCont;
  m_updateHandler->extract(&updCont);

  const FrameBuffer *fb = m_updateHandler->getFrameBuffer();

  PixelFormat newPf = fb->getPixelFormat();

  if (!m_oldPf.isEqualTo(&newPf)) {
    updCont.screenSizeChanged = true;
    m_oldPf = newPf;
  }

  backGate->writeUInt8(updCont.screenSizeChanged);
  if (updCont.screenSizeChanged) {
    sendPixelFormat(&newPf, backGate);
    sendDimension(&fb->getDimension(), backGate);
  } else {
    sendRegion(&updCont.videoRegion, backGate);
    std::vector<Rect> rects;
    std::vector<Rect>::iterator iRect;
    updCont.changedRegion.getRectVector(&rects);
    unsigned int countChangedRect = rects.size();
    backGate->writeUInt32(countChangedRect);

    for (iRect = rects.begin(); iRect < rects.end(); iRect++) {
      Rect *rect = &(*iRect);
      sendRect(rect, backGate);
      sendFrameBuffer(fb, rect, backGate);
    }

    bool hasCopyRect = !updCont.copiedRegion.isEmpty();
    backGate->writeUInt8(hasCopyRect);
    if (hasCopyRect) {
      sendPoint(&updCont.copySrc, backGate);
      updCont.copiedRegion.getRectVector(&rects);
      iRect = rects.begin();
      sendRect(&(*iRect), backGate);
      sendFrameBuffer(fb, &(*iRect), backGate);
    }

    backGate->writeUInt8(updCont.cursorPosChanged);
    if (updCont.cursorPosChanged) {
      sendPoint(&updCont.cursorPos, backGate);
    }

    backGate->writeUInt8(updCont.cursorShapeChanged);
    if (updCont.cursorShapeChanged) {
      const CursorShape *curSh = m_updateHandler->getCursorShape();
      sendDimension(&curSh->getDimension(), backGate);
      sendPoint(&curSh->getHotSpot(), backGate);

      backGate->writeFully(curSh->getPixels()->getBuffer(), curSh->getPixelsSize());
      backGate->writeFully((void *)curSh->getMask(), curSh->getMaskSize());
    }
  }
}

void UpdateHandlerServer::screenPropReply(BlockingGate *backGate)
{
  const FrameBuffer *fb = m_updateHandler->getFrameBuffer();
  sendPixelFormat(&fb->getPixelFormat(), backGate);
  sendDimension(&fb->getDimension(), backGate);
}

void UpdateHandlerServer::receiveFullReqReg(BlockingGate *backGate)
{
  Region region;
  readRegion(&region, backGate);
  m_updateHandler->setFullUpdateRequested(&region);
}

void UpdateHandlerServer::receiveExcludingReg(BlockingGate *backGate)
{
  Region region;
  readRegion(&region, backGate);
  m_updateHandler->setExcludedRegion(&region);
}

void UpdateHandlerServer::serverInit(BlockingGate *backGate)
{
  FrameBuffer fb;
  readPixelFormat(&m_oldPf, backGate);
  Dimension dim = readDimension(backGate);
  fb.setProperties(&dim, &m_oldPf);

  readFrameBuffer(&fb, &dim.getRect(), backGate);
  m_updateHandler->initFrameBuffer(&fb);
}
