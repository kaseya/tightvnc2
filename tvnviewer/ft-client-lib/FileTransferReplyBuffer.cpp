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

#include "FileTransferReplyBuffer.h"
#include "io-lib/ByteArrayInputStream.h"
#include <crtdbg.h>

FileTransferReplyBuffer::FileTransferReplyBuffer(InputStream *inputStream, omni_mutex *readMutex)
: m_isCompressionSupported(false), m_readMutex(readMutex),
  m_filesInfoCount(0), m_filesInfo(NULL), m_inflater(NULL),
  m_downloadBufferSize(0), m_downloadBuffer(0),
  m_downloadFileFlags(0), m_downloadLastModified(0),
  m_dirSize(0)
{
  m_lastErrorMessage.setString(_T(""));

  m_inputStream = inputStream;
  m_dataInputStream = new DataInputStream(m_inputStream);

  m_inflater = new Inflater();
}

FileTransferReplyBuffer::~FileTransferReplyBuffer()
{
  delete m_dataInputStream;
  if (m_filesInfo != NULL) {
    delete[] m_filesInfo;
  }
  if (m_downloadBuffer != NULL) {
    delete[] m_downloadBuffer;
  }
  delete m_inflater;
}

void FileTransferReplyBuffer::setInflater(Inflater *inflater)
{
  m_inflater = inflater;
}

void FileTransferReplyBuffer::getLastErrorMessage(StringStorage *storage)
{
  *storage = m_lastErrorMessage;
}

bool FileTransferReplyBuffer::isCompressionSupported()
{
  return m_isCompressionSupported;
}

UINT32 FileTransferReplyBuffer::getFilesInfoCount()
{
  return m_filesInfoCount;
}

FileInfo *FileTransferReplyBuffer::getFilesInfo()
{
  return m_filesInfo;
}

UINT32 FileTransferReplyBuffer::getDownloadBufferSize()
{
  return m_downloadBufferSize;
}

UINT8 FileTransferReplyBuffer::getDownloadFileFlags()
{
  return m_downloadFileFlags;
}

UINT64 FileTransferReplyBuffer::getDownloadLastModified()
{
  return m_downloadLastModified;
}

UINT64 FileTransferReplyBuffer::getDirSize()
{
  return m_dirSize;
}

UINT8 *FileTransferReplyBuffer::getDownloadBuffer()
{
  return m_downloadBuffer;
}

void FileTransferReplyBuffer::onCompressionSupportReply()
{
  omni_mutex_lock l(*m_readMutex);

  m_isCompressionSupported = (m_dataInputStream->readUInt8() == 1);

  Log::info(_T("Recieved comppression support reply: %s\n"),
            m_isCompressionSupported ? _T("supported") : _T("not supported"));
}

void FileTransferReplyBuffer::onFileListReply()
{
  UINT8 compressionLevel = 0;
  UINT32 compressedSize = 0;
  UINT32 uncompressedSize = 0;

  UINT8 *buffer = NULL;

  {
    omni_mutex_lock l(*m_readMutex);

    compressionLevel = m_dataInputStream->readUInt8();
    compressedSize = m_dataInputStream->readUInt32();
    uncompressedSize = m_dataInputStream->readUInt32();

    buffer = readCompressedDataBlock(compressedSize,
                                     uncompressedSize,
                                     compressionLevel);
  }

  ByteArrayInputStream memoryInputStream((char *)buffer, uncompressedSize);
  DataInputStream filesInfoReader(&memoryInputStream);

  if (m_filesInfo != NULL) {
    delete[] m_filesInfo;
  }

  try {
    m_filesInfoCount = filesInfoReader.readUInt32();
    m_filesInfo = new FileInfo[m_filesInfoCount];

    for (UINT32 i = 0; i < m_filesInfoCount; i++) {
      FileInfo *fileInfo = &m_filesInfo[i];

      fileInfo->setSize(filesInfoReader.readUInt64());
      fileInfo->setLastModified(filesInfoReader.readUInt64());
      fileInfo->setFlags(filesInfoReader.readUInt16());

      StringStorage t;
      filesInfoReader.readUTF8(&t);

      fileInfo->setFileName(t.getString());
    } 
  } catch (IOException &ioEx) {

    delete[] buffer;

    throw ioEx;
  } 

  Log::info(_T("Recieved file list reply: \n")
            _T("\t files count = %d\n")
            _T("\t use compression = %d\n"),
            m_filesInfoCount, compressionLevel);

  delete[] buffer;
}

void FileTransferReplyBuffer::onMd5DataReply()
{
  throw OperationNotSupportedException();
}

void FileTransferReplyBuffer::onUploadReply()
{
  Log::info(_T("Recieved upload reply\n"));
}

void FileTransferReplyBuffer::onUploadDataReply()
{
  Log::info(_T("Recieved upload data reply\n"));
}

void FileTransferReplyBuffer::onUploadEndReply()
{
  Log::info(_T("Recieved upload end reply\n"));
}

void FileTransferReplyBuffer::onDownloadReply()
{
  Log::info(_T("Recieved download reply\n"));
}

void FileTransferReplyBuffer::onDownloadDataReply()
{
  omni_mutex_lock l(*m_readMutex);

  if (m_downloadBuffer != NULL) {
    delete[] m_downloadBuffer;
  }

  UINT8 coLevel = m_dataInputStream->readUInt8();
  UINT32 coBufferSize = m_dataInputStream->readUInt32();
  UINT32 uncoBufferSize = m_dataInputStream->readUInt32();

  m_downloadBuffer = readCompressedDataBlock(coBufferSize, uncoBufferSize, coLevel);
  m_downloadBufferSize = uncoBufferSize;

  Log::info(_T("Recieved download data reply:\n")
            _T("\tcompressed size: %d\n")
            _T("\tuncompressed size: %d\n")
            _T("\tuse compression: %d\n"),
            coBufferSize, uncoBufferSize, coLevel);
}

void FileTransferReplyBuffer::onDownloadEndReply()
{
  omni_mutex_lock l(*m_readMutex);

  m_downloadFileFlags = m_dataInputStream->readUInt8();
  m_downloadLastModified = m_dataInputStream->readUInt64();

  Log::info(_T("Recieved download end reply:\n")
            _T("\tfile flags: %d\n")
            _T("\tmodification time: %ld\n"),
            m_downloadFileFlags, m_downloadLastModified);
}

void FileTransferReplyBuffer::onMkdirReply()
{
  Log::info(_T("Recieved mkdir reply\n"));
}

void FileTransferReplyBuffer::onRmReply()
{
  Log::info(_T("Recieved rm reply\n"));
}

void FileTransferReplyBuffer::onMvReply()
{
  Log::info(_T("Recieved rename reply\n"));
}

void FileTransferReplyBuffer::onDirSizeReply()
{
  omni_mutex_lock l(*m_readMutex);

  m_dirSize = m_dataInputStream->readUInt64();

  Log::info(_T("Recieved dirsize reply\n"));
}

void FileTransferReplyBuffer::onLastRequestFailedReply()
{
  omni_mutex_lock l(*m_readMutex);

  m_dataInputStream->readUTF8(&m_lastErrorMessage);

  Log::info(_T("Recieved last request failed reply:\n")
            _T("\terror message: %s\n"),
            m_lastErrorMessage.getString());
}

UINT8 *FileTransferReplyBuffer::readCompressedDataBlock(UINT32 compressedSize,
                                                        UINT32 uncompressedSize,
                                                        UINT8 compressionLevel)
{

  UINT32 coSize = compressedSize;
  UINT32 uncoSize = uncompressedSize;

  UINT8 *coBuffer = new UINT8[coSize];
  UINT8 *uncoBuffer = NULL;

  try {
    m_dataInputStream->readFully(coBuffer, coSize);
  } catch (IOException &ioEx) {

    delete[] coBuffer;

    throw ioEx;
  } 

  if (compressionLevel == 0) {
    uncoBuffer = coBuffer;
  } else {
    _ASSERT(m_inflater != NULL);

    uncoBuffer = new UINT8[uncoSize];

    m_inflater->setUnpackedSize(uncoSize);
    m_inflater->setInput((const char *)coBuffer, coSize);

    try {
      m_inflater->inflate();
    } catch (ZLibException &zEx) {

      delete[] uncoBuffer;
      delete[] coBuffer;

      throw zEx;
    } 

    _ASSERT(m_inflater->getOutputSize() == uncoSize);

    memcpy(uncoBuffer, m_inflater->getOutput(), uncoSize);

    delete[] coBuffer;
  } 

  return uncoBuffer;
}
