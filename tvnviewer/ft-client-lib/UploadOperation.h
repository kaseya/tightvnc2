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

#ifndef _UPLOAD_OPERATION_H_
#define _UPLOAD_OPERATION_H_

#include "file-lib/File.h"
#include "file-lib/FileInputStream.h"
#include "FileTransferOperation.h"
#include "FileInfoList.h"
#include "CopyOperation.h"

class UploadOperation : public CopyOperation
{
public:

  UploadOperation(FileInfo fileToUpload,
                  const TCHAR *pathToSourceRoot,
                  const TCHAR *pathToTargetRoot);

  UploadOperation(const FileInfo *filesToUpload,
                  size_t filesCount,
                  const TCHAR *pathToSourceRoot,
                  const TCHAR *pathToTargetRoot);

  virtual ~UploadOperation();

  virtual void start() throw(IOException);

  virtual void onUploadReply() throw(IOException);
  virtual void onUploadDataReply() throw(IOException);
  virtual void onUploadEndReply() throw(IOException);
  virtual void onMkdirReply() throw(IOException);
  virtual void onLastRequestFailedReply() throw(IOException);
  virtual void onFileListReply() throw(IOException);

private:

  UINT64 getInputFilesSize();
  UINT64 getFileSize(const TCHAR *pathToFile);

protected:

  void killOp();

  bool specialHandler() throw(IOException);

  void startUpload() throw(IOException);

  void processFolder() throw(IOException);  

  void processFile() throw(IOException);

  void gotoNext();

  void gotoNext(bool fake) throw(IOException);

  void sendFileDataChunk() throw(IOException);

  void releaseRemoteFilesInfo();
  void initRemoteFiles(FileInfo *remoteFilesInfo, UINT32 count);

  void notifyFailedToUpload(const TCHAR *errorDescription);

  void changeFileToUpload(FileInfoList *toUpload);

protected:
  File *m_file;
  FileInputStream *m_fis;

  FileInfo *m_remoteFilesInfo;
  UINT32 m_remoteFilesCount;

  bool m_gotoChild;
  bool m_gotoParent;
  bool m_firstUpload;
};

#endif
