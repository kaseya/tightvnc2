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

#ifndef __COMMANDLINE_H__
#define __COMMANDLINE_H__

#include "util/CommonHeader.h"
#include <vector>

typedef enum UseArgument {
  NO_ARG,
  NEEDS_ARG
};

struct CommandLineFormat
{
  const TCHAR *keyName;
  UseArgument useArg;
};

class CommandLine
{
public:
  CommandLine();
  ~CommandLine();

  bool parse(const CommandLineFormat *format, int formatSize, const TCHAR *commandLine);

  bool optionSpecified(const TCHAR *key, StringStorage *arg = 0) const;

  bool getOption(int index, StringStorage *key, StringStorage *arg = 0) const;

protected:
  class KeyContainer
  {
  public:
    KeyContainer() : isArgument(false) {}
    StringStorage key;
    bool isArgument;
    StringStorage argument;
  };

  bool matchKey(const TCHAR *keyTemplate, TCHAR *key);
  bool removeKeyPrefix(TCHAR *key);

  std::vector<KeyContainer *> m_foundKeys;
};

#endif 
