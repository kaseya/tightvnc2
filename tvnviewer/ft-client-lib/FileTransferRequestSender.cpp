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

#include "FileTransferRequestSender.h"
#include "ft-common/FTMessage.h"

FileTransferRequestSender::FileTransferRequestSender(OutputStream *outputStream, omni_mutex *writeMutex)
: m_outputStream(outputStream), m_writeMutex(writeMutex)
{
  m_dataOutputStream = new DataOutputStream(m_outputStream);
}

FileTransferRequestSender::~FileTransferRequestSender()
{
  delete m_dataOutputStream;
}

void FileTransferRequestSender::sendCompressionSupportRequest()
    
{
  omni_mutex_lock l(*m_writeMutex);

  Log::info(_T("%s\n"), _T("Sending compresion support request"));

  m_dataOutputStream->writeUInt32(FTMessage::COMPRESSION_SUPPORT_REQUEST);
}

void FileTransferRequestSender::sendFileListRequest(const TCHAR *fullPath,
                                                    bool useCompression)
    
{
  omni_mutex_lock l(*m_writeMutex);

  UINT32 messageId = FTMessage::FILE_LIST_REQUEST;
  UINT8 compressionLevel = useCompression ? (UINT8)1 : (UINT8)0;

  Log::info(_T("Sending file list request with parameters:\n")
            _T("\tpath = %s\n")
            _T("\tuse compression = %d\n"),
            fullPath,
            useCompression ? 1 : 0);

  m_dataOutputStream->writeUInt32(messageId);
  m_dataOutputStream->writeUInt8(compressionLevel);
  m_dataOutputStream->writeUTF8(fullPath);
}

void FileTransferRequestSender::sendDownloadRequest(const TCHAR *fullPathName,
                                                    UINT64 offset)
    
{
  omni_mutex_lock l(*m_writeMutex);

  Log::info(_T("Sending download request with parameters:\n")
            _T("\tpath = %s\n")
            _T("\toffset = %ld\n"),
            fullPathName, offset);

  m_dataOutputStream->writeUInt32(FTMessage::DOWNLOAD_START_REQUEST);
  m_dataOutputStream->writeUTF8(fullPathName);
  m_dataOutputStream->writeUInt64(offset);
}

void FileTransferRequestSender::sendDownloadDataRequest(UINT32 size,
                                                        bool useCompression)
    
{
  omni_mutex_lock l(*m_writeMutex);

  UINT8 compressionLevel = useCompression ? (UINT8)1 : (UINT8)0;

  Log::info(_T("Sending download data request with parameters:\n")
            _T("\tsize = %d\n")
            _T("\tuse compression = %d\n"),
            size,
            compressionLevel);

  m_dataOutputStream->writeUInt32(FTMessage::DOWNLOAD_DATA_REQUEST);
  m_dataOutputStream->writeUInt8(compressionLevel);
  m_dataOutputStream->writeUInt32(size);
}

void FileTransferRequestSender::sendRmFileRequest(const TCHAR *fullPathName)
    
{
  omni_mutex_lock l(*m_writeMutex);

  Log::info(_T("Sending rm file request with parameters:\n\tpath = %s\n"),
            fullPathName);

  m_dataOutputStream->writeUInt32(FTMessage::REMOVE_REQUEST);
  m_dataOutputStream->writeUTF8(fullPathName);
}

void FileTransferRequestSender::sendMkDirRequest(const TCHAR *fullPathName)
    
{
  omni_mutex_lock l(*m_writeMutex);

  Log::info(_T("Sending mkdir request with parameters:\n\tpath = %s\n"),
            fullPathName);

  m_dataOutputStream->writeUInt32(FTMessage::MKDIR_REQUEST);
  m_dataOutputStream->writeUTF8(fullPathName);
}

void FileTransferRequestSender::sendMvFileRequest(const TCHAR *oldFileName,
                                                  const TCHAR *newFileName)
    
{
  omni_mutex_lock l(*m_writeMutex);

  Log::info(_T("Sending rename file request with parameters:\n")
            _T("\t old path = %s\n")
            _T("\t new path = %s\n"),
            oldFileName,
            newFileName);

  m_dataOutputStream->writeUInt32(FTMessage::RENAME_REQUEST);
  m_dataOutputStream->writeUTF8(oldFileName);
  m_dataOutputStream->writeUTF8(newFileName);
}

void FileTransferRequestSender::sendUploadRequest(const TCHAR *fullPathName,
                                                  bool overwrite,
                                                  UINT64 offset)
    
{
  omni_mutex_lock l(*m_writeMutex);

  UINT8 flags = 0;
  if (overwrite) {
    flags = 0x1;
  }

  Log::info(_T("Sending upload request with parameters:\n")
            _T("\tpath = %s\n")
            _T("\toverwrite flag = %d\n")
            _T("\toffset = %ld\n"),
            fullPathName,
            overwrite ? 1 : 0,
            offset);

  m_dataOutputStream->writeUInt32(FTMessage::UPLOAD_START_REQUEST);
  m_dataOutputStream->writeUTF8(fullPathName);
  m_dataOutputStream->writeUInt8(flags);
  m_dataOutputStream->writeUInt64(offset);
}

void FileTransferRequestSender::sendUploadDataRequest(const char *buffer,
                                                      UINT32 size,
                                                      bool useCompression)
    
{
  omni_mutex_lock l(*m_writeMutex);

  m_dataOutputStream->writeUInt32(FTMessage::UPLOAD_DATA_REQUEST);

  if (useCompression) {
    throw IOException(_T("Compression is not supported yet."));
  }

  UINT8 compressionLevel = useCompression ? (short)1 : (short)0;

  Log::info(_T("Sending upload data request with parameters:\n")
            _T("\tsize = %d\n")
            _T("\tuse compression = %d\n"),
            size,
            compressionLevel);

  m_dataOutputStream->writeUInt8(compressionLevel);
  m_dataOutputStream->writeUInt32(size);
  m_dataOutputStream->writeUInt32(size);
  m_dataOutputStream->writeFully(buffer, size);
}

void FileTransferRequestSender::sendUploadEndRequest(UINT8 fileFlags,
                                                     UINT64 modificationTime)
    
{
  omni_mutex_lock l(*m_writeMutex);

  Log::info(_T("Sending upload end request with parameters:\n")
            _T("\tflags = %d\n")
            _T("\tmodification time = %ld\n"),
            fileFlags,
            modificationTime);

  m_dataOutputStream->writeUInt32(FTMessage::UPLOAD_END_REQUEST);
  m_dataOutputStream->writeUInt16(fileFlags);
  m_dataOutputStream->writeUInt64(modificationTime);
}

void FileTransferRequestSender::sendFolderSizeRequest(const TCHAR *fullPath)
{
  omni_mutex_lock l(*m_writeMutex);

  Log::info(_T("Sending get folder size request with parameters:\n\tpath = %d\n"),
            fullPath);

  m_dataOutputStream->writeUInt32(FTMessage::DIRSIZE_REQUEST);
  m_dataOutputStream->writeUTF8(fullPath);
}
