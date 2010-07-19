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

#include "LocalFilesDeleteOperation.h"
#include "file-lib/File.h"

LocalFilesDeleteOperation::LocalFilesDeleteOperation(const FileInfo *filesToDelete,
                                                     UINT32 filesCount,
                                                     const TCHAR *pathToTargetRoot)
{
  m_filesCount = filesCount;
  m_filesToDelete = new FileInfo[m_filesCount];
  for (UINT32 i = 0; i < m_filesCount; i++) {
    m_filesToDelete[i] = filesToDelete[i];
  }
  m_pathToTargetRoot.setString(pathToTargetRoot);
}

LocalFilesDeleteOperation::~LocalFilesDeleteOperation()
{
  if (m_filesToDelete != 0) {
    delete[] m_filesToDelete;
  }
}

void LocalFilesDeleteOperation::start()
{
  notifyStart();

  Thread::resume();
}

void LocalFilesDeleteOperation::terminate()
{
  FileTransferOperation::terminate();
  Thread::terminate();
}

void LocalFilesDeleteOperation::execute()
{
  for (UINT32 i = 0; i < m_filesCount; i++) {
    StringStorage pathToTargetFile(m_pathToTargetRoot);
    if (!pathToTargetFile.endsWith(_T('\\'))) {
      pathToTargetFile.appendString(_T("\\"));
    }
    pathToTargetFile.appendString(m_filesToDelete[i].getFileName());

    File file(pathToTargetFile.getString());

    StringStorage message;

    message.format(_T("Deleting local '%s' %s"), pathToTargetFile.getString(),
                   file.isDirectory() ? _T("folder") : _T("file"));

    notifyInformation(message.getString());

    deleteFile(&file);
  } 

  notifyFinish();
}

bool LocalFilesDeleteOperation::deleteFile(File *file)
{
  if (Thread::isTerminating()) {
    return false;
  }

  StringStorage pathToTargetFile;
  file->getPath(&pathToTargetFile);

  if (file->isDirectory()) {
    UINT32 filesCount;
    StringStorage *files = NULL;

    if (file->list(NULL, &filesCount)) {
      files = new StringStorage[filesCount];
      file->list(files, NULL);

      for (UINT32 i = 0; i < filesCount; i++) {

        if (Thread::isTerminating()) {
          return false;
        }

        StringStorage pathToTargetSubFile(pathToTargetFile.getString());

        pathToTargetSubFile.appendString(_T("\\"));
        pathToTargetSubFile.appendString(files[i].getString());

        File subFile(pathToTargetSubFile.getString());
        deleteFile(&subFile);
      } 

      delete[] files;
    } else {
      StringStorage message;

      message.format(_T("Error: failed to get file list of local '%s' folder"),
                     pathToTargetFile.getString());

      notifyError(message.getString());
    }
  } 

  bool returnVal = file->remove();

  if (!returnVal) {
    StringStorage message;

    message.format(_T("Error: failed to remove local '%s' %s"),
                   pathToTargetFile.getString(),
                   file->isDirectory() ? _T("folder") : _T("file"));

    notifyError(message.getString());
  } 

  return returnVal;
}
