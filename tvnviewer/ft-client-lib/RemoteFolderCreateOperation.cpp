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

#include "RemoteFolderCreateOperation.h"

RemoteFolderCreateOperation::RemoteFolderCreateOperation(const TCHAR *pathToTargetFile)
{
  m_pathToTargetFile.setString(pathToTargetFile);
}

RemoteFolderCreateOperation::RemoteFolderCreateOperation(FileInfo file, const TCHAR *pathToTargetRoot)
{
  FileInfoList *list = new FileInfoList(file);
  getRemotePath(list, pathToTargetRoot, &m_pathToTargetFile);
  delete list;
}

RemoteFolderCreateOperation::~RemoteFolderCreateOperation()
{
}

void RemoteFolderCreateOperation::start()
{
  StringStorage message;

  message.format(_T("Creating remote '%s' folder"),
                 m_pathToTargetFile.getString());

  notifyInformation(message.getString());

  notifyStart();

  m_sender->sendMkDirRequest(m_pathToTargetFile.getString());
}

void RemoteFolderCreateOperation::onMkdirReply()
{
  notifyFinish();
}

void RemoteFolderCreateOperation::onLastRequestFailedReply()
{
  StringStorage message;

  message.format(_T("Error: failed to create remote '%s' folder"),
                 m_pathToTargetFile.getString());

  notifyError(message.getString());

  notifyFinish();
}
