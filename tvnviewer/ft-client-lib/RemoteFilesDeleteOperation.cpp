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

#include "RemoteFilesDeleteOperation.h"

RemoteFilesDeleteOperation::RemoteFilesDeleteOperation(const FileInfo *filesInfoToDelete,
                                                       size_t filesCount,
                                                       const TCHAR *pathToTargetRoot)
{
  m_toDelete = new FileInfoList(filesInfoToDelete, filesCount);
  m_pathToTargetRoot.setString(pathToTargetRoot);
}

RemoteFilesDeleteOperation::RemoteFilesDeleteOperation(FileInfo fileInfoToDelete,
                                                       const TCHAR *pathToTargetRoot)
{
  m_toDelete = new FileInfoList(fileInfoToDelete);
  m_pathToTargetRoot.setString(pathToTargetRoot);
}

RemoteFilesDeleteOperation::~RemoteFilesDeleteOperation()
{
  if (m_toDelete != NULL) {
    delete m_toDelete;
  }
}

void RemoteFilesDeleteOperation::start()
{
  notifyStart();

  remove(false);
}

void RemoteFilesDeleteOperation::onFileListReply()
{
  FileInfoList *current = m_toDelete;

  m_toDelete->setChild(m_replyBuffer->getFilesInfo(),
                       m_replyBuffer->getFilesInfoCount());
  FileInfoList *child = m_toDelete->getChild();

  if (child != NULL) {
    m_toDelete = child;
    remove(false);
  } else {
    remove(true);
  }
}

void RemoteFilesDeleteOperation::onRmReply()
{
  gotoNext();
}

void RemoteFilesDeleteOperation::onLastRequestFailedReply()
{

  StringStorage errorMessage;
  m_replyBuffer->getLastErrorMessage(&errorMessage);

  StringStorage remotePath;
  getRemotePath(m_toDelete, m_pathToTargetRoot.getString(), &remotePath);

  StringStorage message;
  message.format(_T("Error: %s ('%s')"), errorMessage.getString(),
                 remotePath.getString());

  notifyError(message.getString());

  gotoNext();
}

void RemoteFilesDeleteOperation::remove(bool removeIfFolder)
{
  if (isTerminating()) {
    killOp();
    return ;
  }

  if (m_toDelete == NULL) {
    return ;
  }

  FileInfo *fileInfo = m_toDelete->getFileInfo();

  StringStorage remotePath;

  getRemotePath(m_toDelete,
                m_pathToTargetRoot.getString(),
                &remotePath);

  if (((fileInfo->isDirectory() && !removeIfFolder) ||
      (!fileInfo->isDirectory())) && (m_toDelete->getFirst()->getParent() == NULL)) {
    StringStorage message;

    message.format(_T("Deleting remote '%s' %s"), remotePath.getString(),
                   fileInfo->isDirectory() ? _T("folder") : _T("file"));

    notifyInformation(message.getString());
  }

  if (!fileInfo->isDirectory() || removeIfFolder) {
    m_sender->sendRmFileRequest(remotePath.getString());

    if ((removeIfFolder) && (fileInfo->isDirectory())) {
      m_toDelete->setChild(NULL, 0);
    }
  } else {
    m_sender->sendFileListRequest(remotePath.getString(), m_replyBuffer->isCompressionSupported());
  }
}

void RemoteFilesDeleteOperation::gotoNext()
{
  FileInfoList *current = m_toDelete;

  bool hasChild = current->getChild() != NULL;
  bool hasNext = current->getNext() != NULL;
  bool hasParent = current->getFirst()->getParent() != NULL;

  if (hasChild) {
    m_toDelete = current->getChild();
    remove(false);
  } else if (hasNext) {
    m_toDelete = current->getNext();
    remove(false);
  } else {

    FileInfoList *first = current->getFirst();
    if (hasParent) {
      m_toDelete = first->getParent();
      remove(true);
    } else {
      killOp();
    } 
  } 
} 

void RemoteFilesDeleteOperation::killOp()
{
  delete m_toDelete->getRoot();
  m_toDelete = NULL;

  notifyFinish();
}
