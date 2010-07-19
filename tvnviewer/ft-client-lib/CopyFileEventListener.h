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

#ifndef _COPY_FILE_EVENT_LISTENER_H_
#define _COPY_FILE_EVENT_LISTENER_H_

#include "util/inttypes.h"
#include "ft-common/FileInfo.h"

class CopyFileEventListener
{
public:

  friend class CopyOperation;
  friend class UploadOperation;
  friend class DownloadOperation;

  static const int TFE_OVERWRITE = 0x0;
  static const int TFE_SKIP      = 0x1;
  static const int TFE_APPEND    = 0x2;
  static const int TFE_CANCEL    = 0x3;

protected:

  virtual void dataChunkCopied(UINT64 totalBytesCopied, UINT64 totalBytesToCopy) = 0;

  virtual int targetFileExists(FileInfo *sourceFileInfo,
                               FileInfo *targetFileInfo,
                               const TCHAR *pathToTargetFile) = 0;
};

#endif
