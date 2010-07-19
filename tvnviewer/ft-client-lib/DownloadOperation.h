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

#ifndef _DOWNLOAD_OPERATION_H_
#define _DOWNLOAD_OPERATION_H_

#include "ft-common/FileInfo.h"
#include "file-lib/FileOutputStream.h"
#include "FileInfoList.h"
#include "CopyOperation.h"

class DownloadOperation : public CopyOperation
{
public:

  DownloadOperation(const FileInfo *filesToDownload, size_t filesCount,
                    const TCHAR *pathToTargetRoot,
                    const TCHAR *pathToSourceRoot);

  virtual ~DownloadOperation();

  virtual void start() throw(IOException);

protected:

  virtual void onFileListReply() throw(IOException);
  virtual void onDownloadReply() throw(IOException);
  virtual void onDownloadDataReply() throw(IOException);
  virtual void onDownloadEndReply() throw(IOException);
  virtual void onLastRequestFailedReply() throw(IOException);
  virtual void onDirSizeReply() throw(IOException);

private:

  void startDownload() throw(IOException);

  void processFile() throw(IOException);

  void processFolder() throw(IOException);

  void gotoNext() throw(IOException);

  void tryCalcInputFilesSize() throw(IOException);

  void killOp();

  void decFoldersToCalcSizeCount();

  void notifyFailedToDownload(const TCHAR *errorDescription);

  void changeFileToDownload(FileInfoList *toDownload);

protected:
  File *m_file;
  FileOutputStream *m_fos;

  UINT64 m_fileOffset;

  UINT32 m_foldersToCalcSizeLeft;
};

#endif
