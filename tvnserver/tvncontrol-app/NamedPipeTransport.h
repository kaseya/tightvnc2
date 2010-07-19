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

#ifndef _NAMED_PIPE_TRANSPORT_H_
#define _NAMED_PIPE_TRANSPORT_H_

#include "Transport.h"

#include "win-system/Pipe.h"
#include "win-system/PipeServer.h"

class NamedPipeTransport : public Transport
{
public:
  NamedPipeTransport(Pipe *client);
  NamedPipeTransport(PipeServer *server);
  virtual ~NamedPipeTransport();

  virtual Channel *getIOStream();

  virtual Transport *accept() throw(Exception);

  virtual void close() throw(Exception);

private:
  Pipe *m_pipe;
  PipeServer *m_pipeServer;
};

#endif
