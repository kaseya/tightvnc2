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

#ifndef __RFB_HOST_PATH_H_INCLUDED__
#define __RFB_HOST_PATH_H_INCLUDED__

class HostPath {
public:
  HostPath();
  HostPath(const char *path, int defaultPort = 5900);
  virtual ~HostPath();

  bool set(const char *path);

  bool isValid() const               { return (m_path != 0); }
  bool isSshHostSpecified() const    { return (m_sshHost != 0); }

  const char* get() const            { return m_path; }
  const char* getSshHost() const     { return m_sshHost; }
  const int getSshPort() const       { return m_sshPort; }
  const char* getVncHost() const     { return m_vncHost; }
  const int getVncPort() const       { return m_vncPort; }

private:
  static const size_t m_SSH_USER_MAX_CHARS;
  static const size_t m_SSH_HOST_MAX_CHARS;
  static const size_t m_SSH_PORT_MAX_CHARS;
  static const size_t m_VNC_HOST_MAX_CHARS;
  static const size_t m_VNC_PORT_MAX_CHARS;

  char* m_path;
  char* m_sshHost;
  int m_sshPort;
  char* m_vncHost;
  int m_vncPort;

  void clear();

  void parsePath(size_t results[]) const;

  bool validateHostNames() const;

  int m_defaultPort;
};

#endif 
