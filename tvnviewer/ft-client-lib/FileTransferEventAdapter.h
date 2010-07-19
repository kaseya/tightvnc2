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

#ifndef _FILE_TRANSFER_EVENT_ADAPTER_H_
#define _FILE_TRANSFER_EVENT_ADAPTER_H_

#include "util/inttypes.h"
#include "FileTransferEventHandler.h"
#include "OperationNotPermittedException.h"

#include <vector>

using namespace std;

class FileTransferEventAdapter : public FileTransferEventHandler
{
public:
  FileTransferEventAdapter();
  virtual ~FileTransferEventAdapter();

  virtual void onCompressionSupportReply() throw(OperationNotPermittedException);
  virtual void onFileListReply() throw(OperationNotPermittedException);
  virtual void onMd5DataReply() throw(OperationNotPermittedException);

  virtual void onUploadReply() throw(OperationNotPermittedException);
  virtual void onUploadDataReply() throw(OperationNotPermittedException);
  virtual void onUploadEndReply() throw(OperationNotPermittedException);

  virtual void onDownloadReply() throw(OperationNotPermittedException);
  virtual void onDownloadDataReply() throw(OperationNotPermittedException);
  virtual void onDownloadEndReply() throw(OperationNotPermittedException);

  virtual void onMkdirReply() throw(OperationNotPermittedException);
  virtual void onRmReply() throw(OperationNotPermittedException);
  virtual void onMvReply() throw(OperationNotPermittedException);

  virtual void onDirSizeReply() throw(OperationNotPermittedException);
  virtual void onLastRequestFailedReply() throw(OperationNotPermittedException);
};

#endif
