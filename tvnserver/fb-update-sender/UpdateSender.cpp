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

#include "util/Log.h"
#include "UpdateSender.h"
#include "rfb/VendorDefs.h"
#include "rfb/EncodingDefs.h"
#include "rfb/MsgDefs.h"
#include <vector>
#include "util/inttypes.h"
#include "util/Exception.h"

UpdateSender::UpdateSender(RfbCodeRegistrator *codeRegtor,
                           UpdateRequestListener *updReqListener,
                           RfbOutputGate *output, int id)
: m_updReqListener(updReqListener),
  m_busy(false),
  m_blockCurPosTime(0),
  m_setColorMapEntr(false),
  m_output(output),
  m_enbox(&m_pixelConverter, m_output),
  m_id(id)
{
  m_updateKeeper = new UpdateKeeper(&Rect());

  codeRegtor->addEncCap(EncodingDefs::COPYRECT,          VendorDefs::STANDARD,
                        EncodingDefs::SIG_COPYRECT);
  codeRegtor->addEncCap(EncodingDefs::HEXTILE,           VendorDefs::STANDARD,
                        EncodingDefs::SIG_HEXTILE);
  codeRegtor->addEncCap(EncodingDefs::TIGHT,             VendorDefs::TIGHTVNC,
                        EncodingDefs::SIG_TIGHT);
  codeRegtor->addEncCap(PseudoEncDefs::COMPR_LEVEL_0,    VendorDefs::TIGHTVNC,
                        PseudoEncDefs::SIG_COMPR_LEVEL);
  codeRegtor->addEncCap(PseudoEncDefs::QUALITY_LEVEL_0,  VendorDefs::TIGHTVNC,
                        PseudoEncDefs::SIG_QUALITY_LEVEL);
  codeRegtor->addEncCap(PseudoEncDefs::RICH_CURSOR,      VendorDefs::TIGHTVNC,
                        PseudoEncDefs::SIG_RICH_CURSOR);
  codeRegtor->addEncCap(PseudoEncDefs::POINTER_POS,      VendorDefs::TIGHTVNC,
                        PseudoEncDefs::SIG_POINTER_POS);
  codeRegtor->addEncCap(PseudoEncDefs::DESKTOP_SIZE,     VendorDefs::TIGHTVNC,
                        PseudoEncDefs::SIG_DESKTOP_SIZE);

  codeRegtor->regCode(ClientMsgDefs::FB_UPDATE_REQUEST, this);
  codeRegtor->regCode(ClientMsgDefs::SET_PIXEL_FORMAT, this);
  codeRegtor->regCode(ClientMsgDefs::SET_ENCODINGS, this);

  resume();
}

UpdateSender::~UpdateSender()
{
  terminate();
  wait();
}

void UpdateSender::onTerminate()
{
  m_newUpdatesEvent.notify();
}

void UpdateSender::onRequest(UINT32 reqCode, RfbInputGate *input)
{
  switch (reqCode) {
  case ClientMsgDefs::FB_UPDATE_REQUEST:
    readUpdateRequest(input);
    break;
  case ClientMsgDefs::SET_PIXEL_FORMAT:
    readSetPixelFormat(input);
    break;
  case ClientMsgDefs::SET_ENCODINGS:
    readSetEncodings(input);
    break;
  default:
    StringStorage errMess;
    errMess.format(_T("Unknown %d protocol code received"), (int)reqCode);
    throw Exception(errMess.getString());
    break;
  }
}

void UpdateSender::init(const Dimension *fbDim, const PixelFormat *pf)
{
  setClientPixelFormat(pf, false);
  {
    AutoLock al(&m_viewPortMut);
    m_clientDim = *fbDim;
  }
  m_lastViewPortDim = *fbDim;
  m_updateKeeper->setBorderRect(&fbDim->getRect());
}

void UpdateSender::newUpdates(const UpdateContainer *updateContainer,
                              const FrameBuffer *frameBuffer,
                              const CursorShape *cursorShape,
                              const Rect *viewPort)
{
  Log::debug(_T("New updates passed to client #%d"), m_id);
  addUpdateContainer(updateContainer, frameBuffer, viewPort);

  {
    AutoLock al(&m_curShapeLocMut);
    m_cursorShape.clone(cursorShape);
  }

  AutoLock al(&m_reqRectLocMut);
  if (clientIsReady()) {
    Log::debug(_T("Client #%d is ready for updates, waking up"), m_id);
    m_busy = true;
    m_newUpdatesEvent.notify();
  } else {
    Log::debug(_T("Client #%d is not ready for updates, not waking"), m_id);
  }
}

void UpdateSender::addUpdateContainer(const UpdateContainer *updateContainer,
                                      const FrameBuffer *srcFb,
                                      const Rect *viewPort)
{
  UpdateContainer updCont = *updateContainer;

  {
    AutoLock al(&m_viewPortMut);
    m_viewPort = *viewPort;
  }

  updCont.videoRegion.translate(-viewPort->left, -viewPort->top);
  updCont.changedRegion.translate(-viewPort->left, -viewPort->top);
  updCont.copiedRegion.translate(-viewPort->left, -viewPort->top);
  updCont.copySrc.move(-viewPort->left, -viewPort->top);

  FrameBuffer *fbForReceive = m_fbAccessor.getFbForWriting(srcFb,
                                                           &m_viewPort);

  Region changedAndCopyRgns = updCont.changedRegion;
  changedAndCopyRgns.add(&updCont.copiedRegion);
  changedAndCopyRgns.add(&updCont.videoRegion);
  {
    AutoLock al(&m_reqRectLocMut);
    changedAndCopyRgns.add(&m_requestedFullReg);
  }
  std::vector<Rect> rects;
  std::vector<Rect>::iterator iRect;
  changedAndCopyRgns.getRectVector(&rects);

  for (iRect = rects.begin(); iRect < rects.end(); iRect++) {
    Rect *rect = &(*iRect);
    fbForReceive->copyFrom(rect, srcFb,
                           rect->left + viewPort->left,
                           rect->top + viewPort->top);
  }

  m_updateKeeper->addUpdateContainer(&updCont);
}

void UpdateSender::blockCursorPosSending()
{
  AutoLock al(&m_curPosLocMut);
  m_blockCurPosTime = DateTime::now();
}

bool UpdateSender::isCursorPosBlocked()
{
  AutoLock al(&m_curPosLocMut);
  if ((DateTime::now() - m_blockCurPosTime).getTime() > 1000) {
    return false; 
  } else {
    return true; 
  }
}

Rect UpdateSender::getViewPort()
{
  AutoLock al(&m_viewPortMut);
  return m_viewPort;
}

bool UpdateSender::clientIsReady()
{
  AutoLock al(&m_reqRectLocMut);
  return (!m_requestedIncrReg.isEmpty() || !m_requestedFullReg.isEmpty()) &&
          !m_busy;
}

void UpdateSender::sendRectHeader(const Rect *rect, INT32 encodingType)
{
  m_output->writeUInt16(rect->left);
  m_output->writeUInt16(rect->top);
  m_output->writeUInt16(rect->getWidth());
  m_output->writeUInt16(rect->getHeight());
  m_output->writeInt32(encodingType);
}

void UpdateSender::sendRectHeader(UINT16 x, UINT16 y, UINT16 w, UINT16 h,
                                  INT32 encodingType)
{
  m_output->writeUInt16(x);
  m_output->writeUInt16(y);
  m_output->writeUInt16(w);
  m_output->writeUInt16(h);
  m_output->writeInt32(encodingType);
}

void UpdateSender::sendNewFBSize(Dimension *dim)
{
  m_output->writeUInt8(ServerMsgDefs::FB_UPDATE); 
  m_output->writeUInt8(0); 
  m_output->writeUInt16(1); 

  Rect r(dim->width, dim->height);
  sendRectHeader(&r, PseudoEncDefs::DESKTOP_SIZE);
}

void UpdateSender::sendBlankScreen(const EncodeOptions *encodeOptions,
                                   const Dimension *dim,
                                   const PixelFormat *pf)
{
  FrameBuffer blankFrameBuffer;
  blankFrameBuffer.setProperties(dim, pf);
  blankFrameBuffer.setColor(0, 0, 0);

  Region region(&dim->getRect());
  std::vector<Rect> rects;
  splitRegion(m_enbox.getEncoder(), &region, &rects, &blankFrameBuffer, encodeOptions);

  m_output->writeUInt8(0); 
  m_output->writeUInt8(0); 
  m_output->writeUInt16(rects.size());
  sendRectangles(m_enbox.getEncoder(), &rects, &blankFrameBuffer, encodeOptions);
}

void UpdateSender::sendCursorShapeUpdate(const PixelFormat *fmt)
{
  CursorShape cursorShape;
  {
    AutoLock al(&m_curShapeLocMut);
    cursorShape.clone(&m_cursorShape);
  }

  Point hotSpot = cursorShape.getHotSpot();
  Dimension dim = cursorShape.getDimension();
  sendRectHeader(hotSpot.x, hotSpot.y, dim.width, dim.height,
                 PseudoEncDefs::RICH_CURSOR);

  FrameBuffer fbConverted;
  fbConverted.setProperties(&dim, fmt);
  m_pixelConverter.convert(&dim.getRect(), &fbConverted,
                           cursorShape.getPixels());

  m_output->writeFully(fbConverted.getBuffer(), fbConverted.getBufferSize());
  m_output->writeFully(cursorShape.getMask(), cursorShape.getMaskSize());
}

void UpdateSender::sendCursorPosUpdate()
{
  Point pos;
  {
    AutoLock al(&m_curPosLocMut);
    pos = m_cursorPos;
  }
  sendRectHeader(pos.x, pos.y, 0, 0, PseudoEncDefs::POINTER_POS);
}

void UpdateSender::sendCopyRect(const std::vector<Rect> *rects, const Point *source)
{
  std::vector<Rect>::const_iterator iRect;

  for (iRect = rects->begin(); iRect != rects->end(); iRect++) {
    const Rect *rect = &(*iRect);

    sendRectHeader(rect, EncodingDefs::COPYRECT);

    m_output->writeUInt16(source->x);
    m_output->writeUInt16(source->y);
  }
}

void UpdateSender::sendPalette(PixelFormat *pf)
{
  m_output->writeUInt8(1); 
  m_output->writeUInt8(0); 
  m_output->writeUInt16(0); 
  m_output->writeUInt16(256); 
  for (unsigned int i = 0; i < 256; i++) {
    m_output->writeUInt16(((i >> pf->redShift) & pf->redMax) * 65535 / pf->redMax); 
    m_output->writeUInt16(((i >> pf->greenShift) & pf->greenMax) * 65535 / pf->greenMax); 
    m_output->writeUInt16(((i >> pf->blueShift) & pf->blueMax) * 65535 / pf->blueMax); 
  }
}

void UpdateSender::checkCursorPos(UpdateContainer *updCont,
                                  const Rect *viewPort)
{
  AutoLock al(&m_curPosLocMut);
  Point cursorPos = updCont->cursorPos;
  cursorPos.x -= viewPort->left;
  cursorPos.y -= viewPort->top;

  if (cursorPos.x < 0) {
    cursorPos.x = 0;
  } else if (cursorPos.x >= viewPort->getWidth()) {
    cursorPos.x = viewPort->getWidth() - 1;
  }
  if (cursorPos.y < 0) {
    cursorPos.y = 0;
  } else if (cursorPos.y >= viewPort->getHeight()) {
    cursorPos.y = viewPort->getHeight() - 1;
  }

  if (cursorPos.x == m_cursorPos.x && cursorPos.y == m_cursorPos.y ||
      isCursorPosBlocked()) {
    updCont->cursorPosChanged = false;
  } else {
    m_cursorPos.x = cursorPos.x;
    m_cursorPos.y = cursorPos.y;
  }
}

void UpdateSender::sendUpdate()
{
  Log::debug(_T("Entering sendUpdate() function"));

  Region requestedFullReg, requestedIncrReg;
  {
    AutoLock al(&m_reqRectLocMut);
    if (m_requestedFullReg.isEmpty() && m_requestedIncrReg.isEmpty()) {
      Log::debug(_T("Requested regions are empty, exiting sendUpdate()"));
      return;
    } else {
      requestedFullReg = m_requestedFullReg;
      requestedIncrReg = m_requestedIncrReg;
      m_requestedFullReg.clear();
      m_requestedIncrReg.clear();
    }
  }

  Log::debug(_T("Requested regions are not empty, continuing"));

  EncodeOptions encodeOptions;
  {
    AutoLock lock(&m_newEncodeOptionsLocker);
    encodeOptions = m_newEncodeOptions;
  }

  m_enbox.selectEncoder(encodeOptions.getPreferredEncoding());

  AutoLock al(&m_fbAccessor);
  FrameBuffer *frameBuffer = m_fbAccessor.getFbForReading();

  UpdateContainer updCont;
  m_updateKeeper->extract(&updCont);
  Region outRegion = updCont.changedRegion;
  Region combinedReqRegion = requestedIncrReg;
  combinedReqRegion.add(&requestedFullReg);
  outRegion.subtract(&combinedReqRegion);
  updCont.changedRegion.intersect(&combinedReqRegion);
  m_updateKeeper->addChangedRegion(&outRegion);

  AutoLock l(m_output);

  Dimension clientDim, lastViewPortDim;
  {
    AutoLock al(&m_viewPortMut);
    clientDim = m_clientDim;
    lastViewPortDim = m_lastViewPortDim;
  }

  Rect viewPort;
  {
    AutoLock al(&m_viewPortMut);
    viewPort = m_viewPort;
  }
  if (!encodeOptions.desktopSizeEnabled()) {
    Rect clientRect = clientDim.getRect();
    clientRect.setLocation(viewPort.left, viewPort.top);
    viewPort = viewPort.intersection(&clientRect);
  }

  if (lastViewPortDim != Dimension(&viewPort) ||
      updCont.screenSizeChanged) {
    updCont.screenSizeChanged = true;

    AutoLock al(&m_viewPortMut);
    m_lastViewPortDim.setDim(&viewPort);
    lastViewPortDim = m_lastViewPortDim;
    if (encodeOptions.desktopSizeEnabled()) {
      m_clientDim.setDim(&viewPort);
      clientDim = m_clientDim;
      m_updateKeeper->setBorderRect(&clientDim.getRect());
      updCont.changedRegion.crop(&clientDim.getRect());
      updCont.changedRegion.addRect(&clientDim.getRect());
    } else {
      m_updateKeeper->setBorderRect(&lastViewPortDim.getRect());
      updCont.changedRegion.crop(&lastViewPortDim.getRect());
      updCont.changedRegion.addRect(&lastViewPortDim.getRect());
    }
  }

  const PixelFormat serverPixelFormat = frameBuffer->getPixelFormat();
  bool setColorMapEntr;
  PixelFormat clientPixelFormat;
  {
    AutoLock lock(&m_newPixelFormatLocker);
    clientPixelFormat = m_newPixelFormat;
    setColorMapEntr = m_setColorMapEntr;
    m_setColorMapEntr = false;
  }
  if (setColorMapEntr) {
    sendPalette(&clientPixelFormat);
  }
  m_pixelConverter.setPixelFormats(&clientPixelFormat, &serverPixelFormat);

  if (updCont.screenSizeChanged || (!requestedFullReg.isEmpty() &&
                                    !encodeOptions.desktopSizeEnabled())) {
    Log::debug(_T("Screen size changed or full region requested"));
    if (encodeOptions.desktopSizeEnabled()) {
      Log::debug(_T("Desktop resize is enabled, sending NewFBSize"));
      sendNewFBSize(&lastViewPortDim);
      Log::debug(_T("Dazzle changed region"));
      m_updateKeeper->dazzleChangedReg();
    } else {
      Log::debug(_T("Desktop resize is disabled, sending blank screen"));
      sendBlankScreen(&encodeOptions, &clientDim,
                      &frameBuffer->getPixelFormat());
      Log::debug(_T("Dazzle changed region"));
      m_updateKeeper->dazzleChangedReg();
    }
  } else {
    Log::debug(_T("Processing normal updates"));
    if (!encodeOptions.richCursorEnabled() ||
        !encodeOptions.pointerPosEnabled()) {
      Log::debug(_T("Clearing cursorPosChanged")
                 _T(" (RichCursor or PointerPos are not requested)"));
      updCont.cursorPosChanged = false;
    } else if (!requestedFullReg.isEmpty()) {
      Log::debug(_T("Raising cursorPosChanged (full region requested)"));
      updCont.cursorPosChanged = true;
    }
    if (!encodeOptions.richCursorEnabled()) {
      Log::debug(_T("Clearing cursorShapeChanged (RichCursor disabled)"));
      updCont.cursorShapeChanged = false;
    } else if (!requestedFullReg.isEmpty()) {
      Log::debug(_T("Raising cursorShapeChanged (RichCursor enabled")
                 _T(" and full region requested)"));
      updCont.cursorShapeChanged = true;
    }
    if (!encodeOptions.copyRectEnabled()) {
      Log::debug(_T("CopyRect is disabled, converting to normal updates"));
      updCont.changedRegion.add(&updCont.copiedRegion);
      updCont.copiedRegion.clear();
    }

    if (updCont.cursorPosChanged) {
      Log::debug(_T("Checking cursor position"));
      checkCursorPos(&updCont, &viewPort);
    }

    Region videoRegion = updCont.videoRegion;
    Region changedRegion = updCont.changedRegion;

    videoRegion.subtract(&requestedFullReg);
    changedRegion.subtract(&videoRegion);
    changedRegion.add(&requestedFullReg);

    Rect frameBufferRect = frameBuffer->getDimension().getRect();
    videoRegion.crop(&frameBufferRect);
    changedRegion.crop(&frameBufferRect);

    if (!encodeOptions.encodingEnabled(EncodingDefs::TIGHT)) {
      changedRegion.add(&videoRegion);
      videoRegion.clear();
    }

    std::vector<Rect> normalRects;
    splitRegion(m_enbox.getEncoder(), &changedRegion, &normalRects,
                frameBuffer, &encodeOptions);

    std::vector<Rect> videoRects;
    if (!videoRegion.isEmpty()) {
      Log::debug(_T("Video region is not empty"));
      m_enbox.validateJpegEncoder(); 
      splitRegion(m_enbox.getJpegEncoder(), &videoRegion, &videoRects,
                  frameBuffer, &encodeOptions);
    }

    std::vector<Rect> copyRects;
    updCont.copiedRegion.getRectVector(&copyRects);

    Log::debug(_T("Number of normal rectangles: %d"), normalRects.size());
    Log::debug(_T("Number of video rectangles: %d"), videoRects.size());
    Log::debug(_T("Number of CopyRect rectangles: %d"), copyRects.size());
    int numTotalRects =
      normalRects.size() + videoRects.size() + copyRects.size();

    if (updCont.cursorPosChanged) {
      numTotalRects++;
      Log::debug(_T("Adding a pseudo-rectangle for cursor position update"));
    }
    if (updCont.cursorShapeChanged) {
      numTotalRects++;
      Log::debug(_T("Adding a pseudo-rectangle for cursor shape update"));
    }
    Log::debug(_T("Total number of rectangles and pseudo-rectangles: %d"),
               numTotalRects);

    _ASSERT(numTotalRects <= 65534);

    if (numTotalRects != 0) {
      Log::debug(_T("Sending FramebufferUpdate message header"));
      m_output->writeUInt8(0); 
      m_output->writeUInt8(0); 
      m_output->writeUInt16((UINT16)numTotalRects);

      if (updCont.cursorPosChanged) {
        Log::debug(_T("Sending cursor position update"));
        sendCursorPosUpdate();
      }
      if (updCont.cursorShapeChanged) {
        Log::debug(_T("Sending cursor shape update"));
        sendCursorShapeUpdate(&clientPixelFormat);
      }
      if (copyRects.size() > 0) {
        Log::debug(_T("Sending CopyRect rectangles"));
        sendCopyRect(&copyRects, &updCont.copySrc);
      }

      Log::debug(_T("Sending video rectangles"));
      sendRectangles(m_enbox.getJpegEncoder(), &videoRects, frameBuffer, &encodeOptions);
      Log::debug(_T("Sending normal rectangles"));
      sendRectangles(m_enbox.getEncoder(), &normalRects, frameBuffer, &encodeOptions);
    } else {
      Log::debug(_T("Nothing to send, restoring requested regions"));
      AutoLock al(&m_reqRectLocMut);
      m_requestedFullReg.add(&requestedFullReg);
      m_requestedIncrReg.add(&requestedIncrReg);
    }
  }

  Log::debug(_T("Flushing output"));
  m_output->flush();
}

void UpdateSender::splitRegion(Encoder *encoder,
                               const Region *region,
                               std::vector<Rect> *rects,
                               const FrameBuffer *frameBuffer,
                               const EncodeOptions *encodeOptions)
{
  std::vector<Rect> baseRects;
  region->getRectVector(&baseRects);
  std::vector<Rect>::iterator i;
  for (i = baseRects.begin(); i != baseRects.end(); i++) {
    encoder->splitRectangle(&*i, rects, frameBuffer, encodeOptions);
  }
}

void UpdateSender::sendRectangles(Encoder *encoder,
                                  const std::vector<Rect> *rects,
                                  const FrameBuffer *frameBuffer,
                                  const EncodeOptions *encodeOptions)
{
  std::vector<Rect>::const_iterator i;
  for (i = rects->begin(); i != rects->end(); i++) {
    sendRectHeader(&*i, encoder->getCode());
    encoder->sendRectangle(&*i, frameBuffer, encodeOptions);
  }
}

void UpdateSender::execute()
{
  Log::info(_T("Starting update sender thread for client #%d"), m_id);

  while(!isTerminating()) {
    m_newUpdatesEvent.waitForEvent();
    Log::debug(_T("Update sender thread of client #%d is awake"), m_id);
    if (!isTerminating()) {
      try {
        Log::debug(_T("Trying to call the sendUpdate() function"));
        sendUpdate();
        Log::debug(_T("The sendUpdate() function has finished"));
        m_busy = false;
      } catch(Exception &e) {
        Log::debug(_T("The update sender thread caught an error and will")
                   _T(" be terminated: %s"), e.getMessage());
        Thread::terminate();
      }
    }
  }
}

void UpdateSender::readUpdateRequest(RfbInputGate *io)
{
  bool incremental = io->readUInt8() != 0;
  Rect reqRect;
  reqRect.left = io->readUInt16();
  reqRect.top = io->readUInt16();
  reqRect.setWidth(io->readUInt16());
  reqRect.setHeight(io->readUInt16());

  {
    AutoLock al(&m_reqRectLocMut);
    if (incremental) {
      m_requestedIncrReg.addRect(&reqRect);
    } else {
      m_requestedFullReg.addRect(&reqRect);
    }
  }

  Log::detail(_T("update requested by client (client #%d)"), m_id);

  _ASSERT(m_updReqListener != 0);
  m_updReqListener->onUpdateRequest(&reqRect, incremental);
}

void UpdateSender::readSetPixelFormat(RfbInputGate *io)
{
  PixelFormat pf;
  io->readUInt16();
  io->readUInt8();

  int bpp = io->readUInt8();
  if (bpp == 8 || bpp == 16 || bpp == 32) {
    pf.bitsPerPixel = bpp;
  } else {
    throw Exception(_T("Only 8, 16 or 32 bits per pixel supported!"));
  }
  pf.colorDepth = io->readUInt8();
  pf.bigEndian = io->readUInt8() != 0;
  bool setColorMapEntr = io->readUInt8() == 0;
  if (setColorMapEntr && bpp != 8) {
    throw Exception(_T("Only 8 bits per pixel supported with set color map ")
                    _T("entries request."));
  }
  pf.redMax = io->readUInt16();
  pf.greenMax = io->readUInt16();
  pf.blueMax = io->readUInt16();
  pf.redShift = io->readUInt8();
  pf.greenShift = io->readUInt8();
  pf.blueShift = io->readUInt8();

  io->readUInt16();
  io->readUInt8();

  if (setColorMapEntr) {
    pf.redMax = 7;
    pf.greenMax = 7;
    pf.blueMax = 3;
    pf.redShift = 0;
    pf.greenShift = 3;
    pf.blueShift = 6;
  }
  setClientPixelFormat(&pf, setColorMapEntr);
}

void UpdateSender::setClientPixelFormat(const PixelFormat *pf,
                                        bool clrMapEntries)
{
  AutoLock al(&m_newPixelFormatLocker);
  m_newPixelFormat = *pf;
  m_setColorMapEntr = clrMapEntries;
}

void UpdateSender::readSetEncodings(RfbInputGate *io)
{
  io->readUInt8(); 
  int numCodes = io->readUInt16();

  std::vector<int> list;
  list.reserve(numCodes);
  for (int i = 0; i < numCodes; i++) {
    int code = (int)io->readUInt32();
    list.push_back(code);
  }

  AutoLock lock(&m_newEncodeOptionsLocker);
  m_newEncodeOptions.setEncodings(&list);
}
