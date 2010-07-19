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

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include "io-lib/DataInputStream.h"

#include "ArgList.h"

class HttpRequest
{
public:
  HttpRequest(DataInputStream *dataInput);
  virtual ~HttpRequest();

  void readHeader() throw(IOException);

  bool parseHeader();

  const char *getRequest() const;
  const char *getFilename() const;
  bool hasArguments() const;
  ArgList *getArguments() const;

protected:
  void skipHeader(bool lastWasEndLn) throw(IOException);
  void readLine(char endLnChar, char *buffer, size_t maxSize) throw(IOException);

protected:
  static const size_t REQUEST_BUFFER_SIZE = 2048;

protected:
  DataInputStream *m_dataInput;
  char m_request[REQUEST_BUFFER_SIZE];
  char m_filename[REQUEST_BUFFER_SIZE];
  char m_args[REQUEST_BUFFER_SIZE];
  ArgList *m_argList;
};

#endif
