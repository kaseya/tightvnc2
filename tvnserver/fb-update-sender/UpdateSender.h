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

#ifndef __UPDATESENDER_H__
#define __UPDATESENDER_H__

#include "thread/AutoLock.h"
#include "thread/Thread.h"
#include "desktop/UpdateKeeper.h"
#include "UpdateRequestListener.h"
#include "rfb/FrameBuffer.h"
#include "desktop/WindowsMouseGrabber.h"
#include "ViewPort.h"
#include "FrameBufferAccessor.h"
#include "network/RfbOutputGate.h"
#include "network/RfbInputGate.h"
#include "rfb/Encoder.h"
#include "rfb/HextileEncoder.h"
#include "rfb/JpegEncoder.h"
#include "rfb/EncoderStore.h"
#include "rfb-sconn/RfbCodeRegistrator.h"
#include "util/DateTime.h"

class UpdateSender : public Thread, public RfbDispatcherListener
{
public:
  UpdateSender(RfbCodeRegistrator *codeRegtor,
               UpdateRequestListener *updReqListener, RfbOutputGate *output,
               int id);
  virtual ~UpdateSender(void);

  void init(const Dimension *fbDim, const PixelFormat *pf);

  void newUpdates(const UpdateContainer *updateContainer,
                  const FrameBuffer *frameBuffer,
                  const CursorShape *cursorShape,
                  const Rect *viewPort);

  void blockCursorPosSending();

  bool isCursorPosBlocked();

  Rect getViewPort();

  bool clientIsReady();

protected:
  virtual void onRequest(UINT32 reqCode, RfbInputGate *input);

  void readUpdateRequest(RfbInputGate *io);
  void readSetPixelFormat(RfbInputGate *io);
  void readSetEncodings(RfbInputGate *io);

  void addUpdateContainer(const UpdateContainer *updateContainer,
                          const FrameBuffer *srcFb,
                          const Rect *viewPort);

  virtual void execute();
  virtual void onTerminate();

  void checkCursorPos(UpdateContainer *updCont,
                      const Rect *viewPort);

  void sendUpdate();

  void sendPalette(PixelFormat *pf);

  void setClientPixelFormat(const PixelFormat *pf,
                            bool clrMapEntries);

  void sendRectHeader(const Rect *rect, INT32 encodingType);
  void sendRectHeader(UINT16 x, UINT16 y, UINT16 w, UINT16 h,
                      INT32 encodingType);
  void sendNewFBSize(Dimension *dim);
  void sendBlankScreen(const EncodeOptions *encodeOptions,
                       const Dimension *dim,
                       const PixelFormat *pf);
  void sendCursorShapeUpdate(const PixelFormat *fmt);
  void sendCursorPosUpdate();
  void sendCopyRect(const std::vector<Rect> *rects, const Point *source);

  void sendRectangles(Encoder *encoder,
                      const std::vector<Rect> *rects,
                      const FrameBuffer *frameBuffer,
                      const EncodeOptions *encodeOptions);

  void splitRegion(Encoder *encoder,
                   const Region *region,
                   std::vector<Rect> *rects,
                   const FrameBuffer *frameBuffer,
                   const EncodeOptions *encodeOptions);

  WindowsEvent m_newUpdatesEvent;

  UpdateRequestListener *m_updReqListener;
  Region m_requestedIncrReg;
  Region m_requestedFullReg;
  bool m_busy;
  LocalMutex m_reqRectLocMut;

  Rect m_viewPort;
  Dimension m_clientDim;
  Dimension m_lastViewPortDim;
  LocalMutex m_viewPortMut;

  UpdateKeeper *m_updateKeeper;

  FrameBufferAccessor m_fbAccessor;

  CursorShape m_cursorShape;
  LocalMutex m_curShapeLocMut;

  Point m_cursorPos;
  DateTime m_blockCurPosTime;
  LocalMutex m_curPosLocMut;

  EncodeOptions m_newEncodeOptions;
  LocalMutex m_newEncodeOptionsLocker;

  PixelFormat m_newPixelFormat;
  LocalMutex m_newPixelFormatLocker;

  bool m_setColorMapEntr;

  RfbOutputGate *m_output;

  PixelConverter m_pixelConverter;

  EncoderStore m_enbox;

  int m_id;
};

#endif 
