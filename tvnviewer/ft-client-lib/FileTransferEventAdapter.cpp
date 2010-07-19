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

#include "FileTransferEventAdapter.h"
#include "ft-common/FTMessage.h"

FileTransferEventAdapter::FileTransferEventAdapter()
{
}

FileTransferEventAdapter::~FileTransferEventAdapter()
{
}

void FileTransferEventAdapter::onCompressionSupportReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onFileListReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onMd5DataReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onUploadReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onUploadDataReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onUploadEndReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onDownloadReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onDownloadDataReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onDownloadEndReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onMkdirReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onRmReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onMvReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onDirSizeReply()
{
  throw OperationNotPermittedException();
}

void FileTransferEventAdapter::onLastRequestFailedReply()
{
  throw OperationNotPermittedException();
}
