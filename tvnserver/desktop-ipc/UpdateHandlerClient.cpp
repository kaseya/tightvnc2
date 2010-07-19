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

#include "win-system/PipeClient.h"
#include "util/Log.h"
#include "UpdateHandlerClient.h"
#include "ReconnectException.h"

UpdateHandlerClient::UpdateHandlerClient(BlockingGate *forwGate,
                                         DesktopSrvDispatcher *dispatcher,
                                         UpdateListener *externalUpdateListener)
: DesktopServerProto(forwGate),
  m_externalUpdateListener(externalUpdateListener)
{
  dispatcher->registerNewHandle(UPDATE_DETECTED, this);

  PixelFormat termPF;
  Dimension termDim;
  getScreenProperties(&termPF, &termDim);

  m_backupFrameBuffer.setProperties(&termDim, &termPF);

  sendInit(m_forwGate);
}

UpdateHandlerClient::~UpdateHandlerClient()
{
}

void UpdateHandlerClient::onRequest(UINT8 reqCode, BlockingGate *backGate)
{
  switch (reqCode) {
  case UPDATE_DETECTED:
    m_externalUpdateListener->onUpdate();
    break;
  default:
    StringStorage errMess;
    errMess.format(_T("Unknown %d protocol code received from a pipe ")
                   _T("update detector"), (int)reqCode);
    throw Exception(errMess.getString());
    break;
  }
}

void UpdateHandlerClient::extract(UpdateContainer *updateContainer)
{
  updateContainer->clear();

  AutoLock al(m_forwGate);

  UpdateContainer updCont;
  try {
    Log::info(_T("UpdateHandlerClient: send EXTRACT_REQ"));
    m_forwGate->writeUInt8(EXTRACT_REQ); 

    updCont.screenSizeChanged = m_forwGate->readUInt8() != 0;

    if (updCont.screenSizeChanged) {
      Log::info(_T("UpdateHandlerClient: screen size changed"));
      PixelFormat oldPf = m_backupFrameBuffer.getPixelFormat();
      Dimension oldDim = m_backupFrameBuffer.getDimension();
      PixelFormat newPf;
      readPixelFormat(&newPf, m_forwGate);
      Dimension newDim = readDimension(m_forwGate);;
      if (!newPf.isEqualTo(&oldPf) || !newDim.isEqualTo(&oldDim)) {
        Log::info(_T("UpdateHandlerClient: new screen size: %dx%d"), newDim.width,
                                                                     newDim.height);
        Log::info(_T("UpdateHandlerClient: new pixel format: ")
                  _T("%d, %d, %d, %d, %d, %d, %d, %d"),
                                (int)newPf.bigEndian,
                                (int)newPf.bitsPerPixel,
                                (int)newPf.redMax,
                                (int)newPf.greenMax,
                                (int)newPf.blueMax,
                                (int)newPf.redShift,
                                (int)newPf.greenShift,
                                (int)newPf.blueShift);
        m_backupFrameBuffer.setProperties(&newDim, &newPf);
      }
    } else {
      readRegion(&updCont.videoRegion, m_forwGate);
      unsigned int countChangedRect = m_forwGate->readUInt32();
      Log::info(_T("UpdateHandlerClient: count changed rectangles = %u"), countChangedRect);
      for (unsigned int i = 0; i < countChangedRect; i++) {
        Rect r = readRect(m_forwGate);
        updCont.changedRegion.addRect(&r);
        readFrameBuffer(&m_backupFrameBuffer, &r, m_forwGate);
      }

      unsigned char hasCopyRect = m_forwGate->readUInt8();
      if (hasCopyRect) {
        Log::info(_T("UpdateHandlerClient: has \"CopyRect\""));
        updCont.copySrc = readPoint(m_forwGate);
        Rect r = readRect(m_forwGate);
        updCont.copiedRegion.addRect(&r);
        readFrameBuffer(&m_backupFrameBuffer, &r, m_forwGate);
      }

      updCont.cursorPosChanged = m_forwGate->readUInt8() != 0;
      if (updCont.cursorPosChanged) {
        Log::info(_T("UpdateHandlerClient: cursor pos changed"));
        updCont.cursorPos = readPoint(m_forwGate);
      }

      updCont.cursorShapeChanged = m_forwGate->readUInt8() != 0;
      if (updCont.cursorShapeChanged) {
        Log::info(_T("UpdateHandlerClient: cursor shape changed"));
        PixelFormat newPf = m_backupFrameBuffer.getPixelFormat();
        Dimension newDim = readDimension(m_forwGate);
        Point newHotSpot = readPoint(m_forwGate);

        m_cursorShape.setProperties(&newDim, &newPf);
        m_cursorShape.setHotSpot(newHotSpot.x, newHotSpot.y);

        m_forwGate->readFully(m_cursorShape.getPixels()->getBuffer(),
                              m_cursorShape.getPixelsSize());
        m_forwGate->readFully((void *)m_cursorShape.getMask(),
                              m_cursorShape.getMaskSize());
      }
    }
  } catch (ReconnectException &) {
    Log::info(_T("UpdateHandlerClient: ReconnectException catching in the extract function"));
  }
  *updateContainer = updCont;
}

void UpdateHandlerClient::setFullUpdateRequested(const Region *region)
{
  AutoLock al(m_forwGate);

  try {
    m_forwGate->writeUInt8(SET_FULL_UPD_REQ_REGION);
    sendRegion(region, m_forwGate);
  } catch (ReconnectException &) {
  }
}

void UpdateHandlerClient::setExcludedRegion(const Region *excludedRegion)
{
  AutoLock al(m_forwGate);

  try {
    m_forwGate->writeUInt8(SET_EXCLUDING_REGION);
    sendRegion(excludedRegion, m_forwGate);
  } catch (ReconnectException &) {
  }
}

bool UpdateHandlerClient::checkForUpdates(Region *region)
{
  return false;
}

void UpdateHandlerClient::getScreenProperties(PixelFormat *pf, Dimension *dim)
{
  AutoLock al(m_forwGate);

  m_forwGate->writeUInt8(SCREEN_PROP_REQ);
  readPixelFormat(pf, m_forwGate);
  *dim = readDimension(m_forwGate);
}

void UpdateHandlerClient::sendInit(BlockingGate *gate)
{
  AutoLock al(gate);
  gate->writeUInt8(FRAME_BUFFER_INIT);

  sendPixelFormat(&m_backupFrameBuffer.getPixelFormat(), gate);
  Dimension dim = m_backupFrameBuffer.getDimension();
  sendDimension(&dim, gate);
  sendFrameBuffer(&m_backupFrameBuffer, &dim.getRect(), gate);
}
