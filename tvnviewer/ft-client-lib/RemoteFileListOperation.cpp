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

#include "RemoteFileListOperation.h"

RemoteFileListOperation::RemoteFileListOperation(const TCHAR *remotePath)
: m_isOk(false), m_isFinished(false)
{
  m_remotePath.setString(remotePath);
}

RemoteFileListOperation::~RemoteFileListOperation()
{
}

void RemoteFileListOperation::start()
{
  m_isOk = false;
  m_isFinished = false;

  m_sender->sendFileListRequest(m_remotePath.getString(),
                                m_replyBuffer->isCompressionSupported());
  notifyStart();
}

void RemoteFileListOperation::onFileListReply()
{
  m_isOk = true;
  m_isFinished = true;
  notifyFinish();
}

void RemoteFileListOperation::onLastRequestFailedReply()
{
  m_isOk = false;
  m_isFinished = true;

  StringStorage message;

  message.format(_T("Error: failed to get file list of remote '%s' folder"),
                 m_remotePath.getString());

  notifyError(message.getString());

  notifyFinish();
}

bool RemoteFileListOperation::isOk()
{
  return m_isFinished && m_isOk;
}
