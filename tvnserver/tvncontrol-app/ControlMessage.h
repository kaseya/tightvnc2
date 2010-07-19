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

#ifndef _CONTROL_MESSAGE_H_
#define _CONTROL_MESSAGE_H_

#include "io-lib/DataOutputStream.h"
#include "io-lib/ByteArrayOutputStream.h"

#include "util/CommonHeader.h"

#include "RemoteException.h"
#include "ControlGate.h"

class ControlMessage : public DataOutputStream
{
public:
  ControlMessage(UINT32 messageId, ControlGate *gate,
                 const TCHAR *passwordFile = 0,
                 bool getPassFromConfigEnabled = false,
                 bool forService = false);
  virtual ~ControlMessage();

  virtual void send() throw(IOException, RemoteException);

private:
  void sendData() throw(IOException);

  void checkRetCode() throw(IOException, RemoteException);

  void authFromFile();

  void authFromRegistry();

private:
  ControlGate *m_gate;
  ByteArrayOutputStream *m_tunnel;
  UINT32 m_messageId;

  StringStorage m_passwordFile;
  bool m_getPassFromConfigEnabled;
  bool m_forService;
};

#endif
