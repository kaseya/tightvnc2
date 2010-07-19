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

#include "FileLog.h"

#include "util/Unicode.h"
#include "thread/AutoLock.h"
#include "file-lib/File.h"

FileLog::FileLog(const TCHAR *pathToLogFile, bool createBackup)
: Log(),
  m_file(INVALID_HANDLE_VALUE),
  m_createBackup(createBackup),
  m_filename(pathToLogFile),
  m_backupIsCreated(false),
  m_logLevel(0)
{
}

FileLog::~FileLog()
{
  if (isFileOpened()) {
    closeFile();
  }
}

void FileLog::changeLevel(int level)
{
  level = max(0, min(level, 9));

  {
    AutoLock l(&m_llMutex);

    m_logLevel = level;
  }

  AutoLock l(&m_fMutex);

  if (m_filename.isNull()) {
    return;
  }

  if ((level > 0) && (m_createBackup) && (!m_backupIsCreated)) {
    backupLogFile(m_filename.getString());

    m_backupIsCreated = true;
  }

  if ((level == 0) && isFileOpened()) {
    closeFile();
  }

  if ((level > 0) && !isFileOpened()) {
    openFile();
  }
}

int FileLog::getLevel()
{
  AutoLock l(&m_llMutex);

  return m_logLevel;
}

void FileLog::changeFilename(const TCHAR *pathToLogFile)
{
  {
    AutoLock l(&m_fMutex);

    if (m_filename.isEqualTo(pathToLogFile)) {
      return;
    }

    m_filename.setString(pathToLogFile);

    m_backupIsCreated = false;

    closeFile();
  }

  changeLevel(getLevel());
}

void FileLog::backupLogFile(const TCHAR *pathToLogFile, int currentBackupIndex)
{
  if (currentBackupIndex >= MAX_BACKUP_COUNT) {
    return;
  }

  StringStorage backupPath;
  getBackupFileName(pathToLogFile, currentBackupIndex, &backupPath);
  const TCHAR *backupPathString = backupPath.getString();

  File backupLogFile(backupPathString);
  if (backupLogFile.exists()) {
    FileLog::backupLogFile(backupPathString, currentBackupIndex + 1);
  }

  bool wasError = MoveFile(pathToLogFile, backupPathString) == 0;
  if (wasError && GetLastError() == ERROR_ALREADY_EXISTS) {
    DeleteFile(backupPathString);
    MoveFile(pathToLogFile, backupPathString);
  }
}

void FileLog::getBackupFileName(const TCHAR *pathToLogFile, int backupNumber,
                                StringStorage *pathToBackup)
{
  File oldFile(pathToLogFile);

  StringStorage ext;
  StringStorage path;
  StringStorage fileName;

  oldFile.getPath(&path);
  oldFile.getFileExtension(&ext);
  oldFile.getName(&fileName);

  if (ext.isEmpty()) {
    pathToBackup->format(_T("%s.%d"), path.getString(), backupNumber);
  } else {
    path.getSubstring(&path, 0, path.getLength() - fileName.getLength() - 1);
    fileName.getSubstring(&fileName, 0, fileName.findChar(_T('.')) - 1);
    pathToBackup->format(_T("%s%s.%d.%s"), path.getString(), fileName.getString(), backupNumber, ext.getString());
  }
}

bool FileLog::isFileOpened() const
{
  return m_file != INVALID_HANDLE_VALUE;
}

void FileLog::openFile()
{
  m_file = CreateFile(m_filename.getString(),
                      GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL,
                      OPEN_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL);

  if (!isFileOpened()) {
    return ;
  }

  SetEndOfFile(m_file);

  if (Unicode::isEnabled()) {
    checkUnicodeSignature();
  }
}

void FileLog::closeFile()
{
  CloseHandle(m_file);

  m_file = INVALID_HANDLE_VALUE;
}

void FileLog::checkUnicodeSignature()
{
  unsigned short firstTwoBytes = 0;
  DWORD read;
  if (!(ReadFile(m_file, &firstTwoBytes, 2, &read, 0) && read == 2 && firstTwoBytes == Unicode::SIGNATURE)) {
    DWORD written;
    WriteFile(m_file, &Unicode::SIGNATURE, sizeof(unsigned short), &written, 0);
  }
}

void FileLog::flushLine(int logLevel, const TCHAR *line)
{
  if (logLevel > getLevel() || !isFileOpened()) {
    return;
  }

  DWORD written = 0;

  AutoLock l(&m_fMutex);

  WriteFile(m_file, line, _tcslen(line) * sizeof(TCHAR),
            &written, NULL);
}
