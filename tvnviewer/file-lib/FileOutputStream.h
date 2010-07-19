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

#ifndef _FILE_OUTPUT_STREAM_H_
#define _FILE_OUTPUT_STREAM_H_

#include "io-lib/OutputStream.h"
#include "io-lib/IOException.h"

#include "FileNotFoundException.h"

#include "File.h"
#include "FileDescriptor.h"

class FileOutputStream : public OutputStream
{
public:
  FileOutputStream(const File *file) throw(IOException, FileNotFoundException);
  ~FileOutputStream();

  virtual size_t write(const void *buffer, size_t len) throw(IOException);
  virtual void close() throw(IOException);

  const FileDescriptor *getFD() const;

protected:
  const File *m_file;
  FileDescriptor m_fd;
};

#endif
