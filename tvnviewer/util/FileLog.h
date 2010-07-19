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

#ifndef _FILE_LOG_H_
#define _FILE_LOG_H_

#include "Log.h"
#include "util/CommonHeader.h"
#include "thread/LocalMutex.h"

class FileLog : public Log
{
public:
  static const int MAX_BACKUP_COUNT = 5;

public:
  FileLog(const TCHAR *pathToLogFile = 0, bool createBackup = true);
  virtual ~FileLog();

  void changeFilename(const TCHAR *pathToLogFile);

  void changeLevel(int level);

  int getLevel();

  static void getBackupFileName(const TCHAR *pathToLogFile, int backupNumber,
                                StringStorage *pathToBackup);

protected:
  virtual void flushLine(int logLevel, const TCHAR *line);

  bool isFileOpened() const;
  void openFile();
  void closeFile();
  void checkUnicodeSignature();

  static void backupLogFile(const TCHAR *pathToLogFile, int currentBackupIndex = 0);

private:
  HANDLE m_file;
  bool m_createBackup;
  bool m_backupIsCreated;
  StringStorage m_filename;
  int m_logLevel;
  LocalMutex m_fMutex;
  LocalMutex m_llMutex;
};

#endif
