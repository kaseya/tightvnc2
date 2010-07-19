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

#include "DownloadOperation.h"

#include "file-lib/FileSeeker.h"

DownloadOperation::DownloadOperation(const FileInfo *filesToDownload,
                                     size_t filesCount,
                                     const TCHAR *pathToTargetRoot,
                                     const TCHAR *pathToSourceRoot)
: m_file(0), m_fos(0), m_fileOffset(0)
{
  m_pathToSourceRoot.setString(pathToSourceRoot);
  m_pathToTargetRoot.setString(pathToTargetRoot);

  m_toCopy = new FileInfoList(filesToDownload, filesCount);

  changeFileToDownload(m_toCopy);
}

DownloadOperation::~DownloadOperation()
{
  if (m_toCopy != NULL) {
    delete m_toCopy->getRoot();
  }
  if (m_fos != NULL) {
    try { m_fos->close(); } catch (IOException) { }
    delete m_fos;
  }
  if (m_file != NULL) {
    delete m_file;
  }
}

void DownloadOperation::start()
{
  m_foldersToCalcSizeLeft = 0;

  m_totalBytesToCopy = 0;
  m_totalBytesCopied = 0;

  notifyStart();

  tryCalcInputFilesSize();

  if (m_foldersToCalcSizeLeft == 0) {
    startDownload();
  }
}

void DownloadOperation::onFileListReply()
{
  m_toCopy->setChild(m_replyBuffer->getFilesInfo(),
                         m_replyBuffer->getFilesInfoCount());
  gotoNext();
}

void DownloadOperation::onDownloadReply()
{

  if (m_fos != NULL) {
    try { m_fos->close(); } catch (...) { }
    delete m_fos;
  }
  if (m_file != NULL) {
    delete m_file;
  }

  m_file = new File(m_pathToTargetFile.getString());

  if (m_fileOffset == 0) {
    m_file->truncate();
  }

  try {
    m_fos = new FileOutputStream(m_file);

    FileSeeker fileSeeker(m_fos->getFD());
    if (!fileSeeker.seek((INT64)m_fileOffset)) {
      throw IOException(_T("Cannot seek to initial file position"));
    }

    m_totalBytesCopied += m_fileOffset;
  } catch (IOException &ioEx) {
    notifyFailedToDownload(ioEx.getMessage());
    gotoNext();
    return ;
  }

  UINT32 dataSize = 1024 * 8;
  bool compression = m_replyBuffer->isCompressionSupported();

  m_sender->sendDownloadDataRequest(dataSize,
                                    compression);
}

void DownloadOperation::onDownloadDataReply()
{
  if (isTerminating()) {
    gotoNext();
    return ;
  }

  UINT32 bufferSize = 1024 * 8;

  try {
    DataOutputStream dos(m_fos);
    dos.writeFully(m_replyBuffer->getDownloadBuffer(),
              m_replyBuffer->getDownloadBufferSize());
  } catch (IOException &ioEx) {
    notifyFailedToDownload(ioEx.getMessage());
    gotoNext();
    return ;
  }

  m_totalBytesCopied += m_replyBuffer->getDownloadBufferSize();

  if (m_copyListener != NULL) {
    m_copyListener->dataChunkCopied(m_totalBytesCopied, m_totalBytesToCopy);
  }

  bool compression = m_replyBuffer->isCompressionSupported();

  m_sender->sendDownloadDataRequest(bufferSize, compression);
}

void DownloadOperation::onDownloadEndReply()
{

  try { m_fos->close(); } catch (...) { }
  delete m_fos;
  m_fos = NULL;

  try {
    m_file->setLastModified(m_replyBuffer->getDownloadLastModified());
  } catch (...) {
    notifyFailedToDownload(_T("Cannot set modification time"));
  }

  delete m_file;
  m_file = NULL;

  gotoNext();
}

void DownloadOperation::onLastRequestFailedReply()
{

  if (m_foldersToCalcSizeLeft > 0) {
    decFoldersToCalcSizeCount();
  } else {
    StringStorage message;

    m_replyBuffer->getLastErrorMessage(&message);

    notifyFailedToDownload(message.getString());

    gotoNext();
  }
}

void DownloadOperation::onDirSizeReply()
{
  m_totalBytesToCopy += m_replyBuffer->getDirSize();
  decFoldersToCalcSizeCount();
}

void DownloadOperation::startDownload()
{
  if (isTerminating()) {
    killOp();
    return ;
  } 

  FileInfo *fileInfo = m_toCopy->getFileInfo();

  if (m_toCopy->getFirst()->getParent() == NULL) {
    StringStorage message;

    message.format(_T("Downloading '%s' %s"), m_pathToTargetFile.getString(),
                   fileInfo->isDirectory() ? _T("folder") : _T("file"));

    notifyInformation(message.getString());
  } 

  if (fileInfo->isDirectory()) {
    processFolder();
  } else {
    processFile();
  } 

  if (isTerminating()) {
    killOp();
    return ;
  } 
}

void DownloadOperation::processFile()
{
  m_fileOffset = 0;

  File targetFile(m_pathToTargetFile.getString());

  if (targetFile.exists()) {
    FileInfo *sourceFileInfo = m_toCopy->getFileInfo();
    FileInfo targetFileInfo(&targetFile);

    int action = m_copyListener->targetFileExists(sourceFileInfo,
                                                  &targetFileInfo,
                                                  m_pathToTargetFile.getString());
    switch (action) {
    case CopyFileEventListener::TFE_OVERWRITE:
      break;
    case CopyFileEventListener::TFE_SKIP:
      m_totalBytesCopied += sourceFileInfo->getSize();
      gotoNext();
      return ;
    case CopyFileEventListener::TFE_APPEND:
      m_fileOffset = targetFileInfo.getSize();
      break;
    case CopyFileEventListener::TFE_CANCEL:
      if (!isTerminating()) {
        terminate();
      } 
      return ;
    default:
      _ASSERT(FALSE);
    } 
  } 

  m_sender->sendDownloadRequest(m_pathToSourceFile.getString(), m_fileOffset);
}

void DownloadOperation::processFolder()
{
  File local(m_pathToTargetFile.getString());
  if (local.exists() && local.isDirectory()) {
  } else {
    if (!local.mkdir()) {
      StringStorage message;

      message.format(_T("Error: failed to create local '%s' folder"),
                     m_pathToTargetFile.getString());

      notifyError(message.getString());

      gotoNext();
      return ;
    }
  }

  m_sender->sendFileListRequest(m_pathToSourceFile.getString(),
                                m_replyBuffer->isCompressionSupported());
}

void DownloadOperation::gotoNext()
{
  FileInfoList *current = m_toCopy;

  bool hasChild = current->getChild() != NULL;
  bool hasNext = current->getNext() != NULL;
  bool hasParent = current->getFirst()->getParent() != NULL;

  if (hasChild) {
    changeFileToDownload(current->getChild());
    startDownload();
  } else if (hasNext) {
    changeFileToDownload(current->getNext());
    startDownload();
  } else {

    FileInfoList *first = current->getFirst();
    if (hasParent) {
      changeFileToDownload(first->getParent());
      m_toCopy->setChild(NULL, 0);

      gotoNext();
    } else {
      killOp();
    } 
  } 
} 

void DownloadOperation::tryCalcInputFilesSize()
{
  FileInfoList *fil = m_toCopy;

  while (fil != NULL) {
    if (fil->getFileInfo()->isDirectory()) {
      StringStorage pathNoRoot;
      StringStorage pathToFile;

      fil->getAbsolutePath(&pathNoRoot, _T('/'));

      pathToFile.setString(m_pathToSourceRoot.getString());
      if (!pathToFile.endsWith(_T('/'))) {
        pathToFile.appendString(_T("/"));
      }
      pathToFile.appendString(pathNoRoot.getString());

      m_foldersToCalcSizeLeft++;
      m_sender->sendFolderSizeRequest(pathToFile.getString());
    } else {
      m_totalBytesToCopy += fil->getFileInfo()->getSize();
    }
    fil = fil->getNext();
  }
}

void DownloadOperation::killOp()
{

  delete m_toCopy->getRoot();
  m_toCopy = NULL;

  notifyFinish();
}

void DownloadOperation::decFoldersToCalcSizeCount()
{
  m_foldersToCalcSizeLeft--;

  if (m_foldersToCalcSizeLeft == 0) {
    startDownload();
  }
}

void DownloadOperation::notifyFailedToDownload(const TCHAR *errorDescription)
{
  StringStorage message;

  message.format(_T("Error: failed to download '%s' (%s)"),
                 m_pathToSourceFile.getString(),
                 errorDescription);

  notifyError(message.getString());
}

void DownloadOperation::changeFileToDownload(FileInfoList *toDownload)
{
  m_toCopy = toDownload;

  getRemotePath(m_toCopy, m_pathToSourceRoot.getString(), &m_pathToSourceFile);
  getLocalPath(m_toCopy, m_pathToTargetRoot.getString(), &m_pathToTargetFile);
}
