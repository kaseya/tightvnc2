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

#include "ClipboardExchange.h"
#include "rfb/MsgDefs.h"
#include "util/Log.h"

ClipboardExchange::ClipboardExchange(RfbCodeRegistrator *codeRegtor,
                                     WinDesktop *desktop,
                                     RfbOutputGate *output,
                                     bool viewOnly)
: m_desktop(desktop),
  m_output(output),
  m_viewOnly(viewOnly)
{
  codeRegtor->regCode(ClientMsgDefs::CLIENT_CUT_TEXT, this);
  resume();
}

ClipboardExchange::~ClipboardExchange()
{
  terminate();
  wait();
}

void ClipboardExchange::onRequest(UINT32 reqCode, RfbInputGate *input)
{
  switch (reqCode) {
  case ClientMsgDefs::CLIENT_CUT_TEXT:
    input->readUInt8(); 
    input->readUInt16(); 
    {
      UINT32 length = input->readUInt32();
      char *receivedText = new char[length + 1];
      try {
        input->readFully(receivedText, length);
      } catch (...) {
        delete[] receivedText;
        throw;
      }
      if (!m_viewOnly) {
        receivedText[length] = '\0';
        StringStorage clipText;
        clipText.fromAnsiString(receivedText);
        delete[] receivedText;
        m_desktop->setNewClipText(&clipText);
      } else {
        delete[] receivedText;
      }
    }
    break;
  default:
    StringStorage errMess;
    errMess.format(_T("Unknown %d protocol code received"), (int)reqCode);
    throw Exception(errMess.getString());
    break;
  }
}

void ClipboardExchange::sendClipboard(const StringStorage *newClipboard)
{
  AutoLock al(&m_storedClipMut);
  m_storedClip = *newClipboard;
  m_newClipWaiter.notify();
}

void ClipboardExchange::onTerminate()
{
  m_newClipWaiter.notify();
}

void ClipboardExchange::execute()
{
  while (!isTerminating()) {
    m_newClipWaiter.waitForEvent();

    if (m_storedClip.getString() != 0 && !isTerminating() && !m_viewOnly) {
      size_t length = 0;
      char *ansiClip = 0;

      try {
        AutoLock al(m_output);
        m_output->writeUInt8(ServerMsgDefs::SERVER_CUT_TEXT); 
        m_output->writeUInt8(0); 
        m_output->writeUInt16(0); 

        {
          AutoLock al(&m_storedClipMut);
          length = m_storedClip.getLength();
          ansiClip = new char[length + 1];
          m_storedClip.toAnsiString(ansiClip, length + 1);
          m_storedClip.setString(0);
        }
        m_output->writeUInt32(length);
        m_output->writeFully(ansiClip, length);

        m_output->flush();
      } catch (Exception &e) {
        Log::error(_T("The clipboard thread force to terminate because")
                   _T(" it caught the error: %s"), e.getMessage());
        terminate();
      }

      if (ansiClip != 0) {
        delete[] ansiClip;
      }
    }
  }
}
