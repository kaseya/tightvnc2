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

#include "TvnLogFilename.h"

#include "file-lib/File.h"

#include "win-system/Environment.h"

TvnLogFilename::TvnLogFilename()
{
}

void TvnLogFilename::queryLogFilePath(bool forService, bool shareLog, StringStorage *logFilePath)
{
  StringStorage root;

  queryLogFileDirectory(forService, shareLog, &root);

  logFilePath->format(_T("%s\\tvnserver.log"), root.getString());
}

void TvnLogFilename::queryLogFileDirectory(bool forService,
                                           bool shareToAll,
                                           StringStorage *logFileDirectory)
{
  int specialFolderId = (shareToAll) ? Environment::COMMON_APPLICATION_DATA_SPECIAL_FOLDER : Environment::APPLICATION_DATA_SPECIAL_FOLDER;

  StringStorage specialFolder(_T(""));

  Environment::getSpecialFolderPath(specialFolderId, &specialFolder);

  logFileDirectory->format(_T("%s\\TightVNC"), specialFolder.getString());
}
