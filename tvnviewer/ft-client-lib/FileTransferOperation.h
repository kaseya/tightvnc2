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

#ifndef _FILE_TRANSFER_OPERATION_H_
#define _FILE_TRANSFER_OPERATION_H_

#include "FileTransferEventAdapter.h"
#include "OperationNotPermittedException.h"
#include "FileTransferRequestSender.h"
#include "FileTransferReplyBuffer.h"
#include "FileInfoList.h"
#include "ListenerContainer.h"

#include "util/Log.h"

class OperationEventListener;

class FileTransferOperation : public FileTransferEventAdapter,
                              public ListenerContainer<OperationEventListener *>
{
public:
  FileTransferOperation();
  virtual ~FileTransferOperation();

  void setRequestSender(FileTransferRequestSender *sender);

  void setReplyBuffer(FileTransferReplyBuffer *replyBuffer);

  virtual void start() throw(IOException) = 0;

  virtual void terminate();

protected:

  virtual bool isTerminating();

  void notifyStart();

  void notifyFinish();

  void notifyError(const TCHAR *message);

  void notifyInformation(const TCHAR *message);

  void getLocalPath(FileInfoList *currentFile, const TCHAR *localFolder, StringStorage *out);

  void getRemotePath(FileInfoList *currentFile, const TCHAR *remoteFolder, StringStorage *out);

protected:
  FileTransferRequestSender *m_sender;
  FileTransferReplyBuffer *m_replyBuffer;

  bool m_isTerminating;
};

#endif
