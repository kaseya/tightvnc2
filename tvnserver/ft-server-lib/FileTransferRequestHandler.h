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

#ifndef _FILE_TRANSFER_REQUEST_HANDLER_H_
#define _FILE_TRANSFER_REQUEST_HANDLER_H_

#include "util/inttypes.h"
#include "network/RfbInputGate.h"
#include "network/RfbOutputGate.h"
#include "ft-common/FileInfo.h"
#include "file-lib/FileInputStream.h"
#include "file-lib/FileOutputStream.h"
#include "util/Inflater.h"
#include "util/Deflater.h"
#include "desktop/WinDesktop.h"
#include "rfb-sconn/RfbCodeRegistrator.h"
#include "rfb-sconn/RfbDispatcherListener.h"
#include "FileTransferSecurity.h"
#include "util/Log.h"

class FileTransferRequestHandler : public RfbDispatcherListener
{
public:
  FileTransferRequestHandler(RfbCodeRegistrator *registrator,
                             RfbOutputGate *output,
                             WinDesktop *desktop,
                             bool enabled = true);

  virtual ~FileTransferRequestHandler();

  virtual void onRequest(UINT32 reqCode, RfbInputGate *backGate);

protected:

  bool isFileTransferEnabled();

  void compressionSupportRequested();
  void fileListRequested();
  void mkDirRequested();
  void rmFileRequested();
  void mvFileRequested();
  void dirSizeRequested();
  void md5Requested();

  void uploadStartRequested();
  void uploadDataRequested();
  void uploadEndRequested();

  void downloadStartRequested();
  void downloadDataRequested();

  void lastRequestFailed(StringStorage *storage);
  void lastRequestFailed(const TCHAR *description);

  bool getDirectorySize(const TCHAR *pathname, UINT64 *dirSize);

protected:
  void checkAccess() throw(SystemException);

protected:

  RfbInputGate *m_input;
  RfbOutputGate *m_output;

  File *m_downloadFile;
  FileInputStream *m_fileInputStream;

  File *m_uploadFile;
  FileOutputStream *m_fileOutputStream;

  Deflater m_deflater;
  Inflater m_inflater;

  FileTransferSecurity *m_security;

  bool m_enabled;
};

#endif
