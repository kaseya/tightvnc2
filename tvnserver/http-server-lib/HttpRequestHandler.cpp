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

#include "HttpRequestHandler.h"
#include "HttpRequest.h"
#include "HttpReply.h"
#include "AppletParameter.h"
#include "CharString.h"
#include "VncViewerJarBody.h"
#include "win-system/Environment.h"
#include "server-config-lib/Configurator.h"
#include "util/Log.h"

const char HTTP_INDEX_PAGE_FORMAT[] =
"<HTML>\n"
"  <HEAD><TITLE>TightVNC desktop [%.256s]</TITLE></HEAD>\n"
"  <BODY>\n"
"    <APPLET ARCHIVE=\"VncViewer.jar\" CODE=VncViewer WIDTH=1 HEIGHT=1>\n"
"      <PARAM NAME=\"PORT\" VALUE=\"%d\">\n"
"      <PARAM NAME=\"Open new window\" VALUE=\"YES\">\n"
"%.1024s"
"    </APPLET><BR>\n"
"    <A HREF=\"http://www.tightvnc.com/\">www.TightVNC.com</A>\n"
"  </BODY>\n"
"</HTML>\n";

const char HTTP_MSG_BADPARAMS [] =
"<HTML>\n"
"  <HEAD><TITLE>TightVNC desktop</TITLE></HEAD>\n"
"  <BODY>\n"
"    <H1>Bad Parameters</H1>\n"
"    The sequence of applet parameters specified within the URL is invalid.\n"
"  </BODY>\n"
"</HTML>\n";


HttpRequestHandler::HttpRequestHandler(DataInputStream *dataInput,
                                       DataOutputStream *dataOutput,
                                       const TCHAR *peerHost)
: m_dataInput(dataInput), m_dataOutput(dataOutput),
  m_peerHost(peerHost)
{
}

HttpRequestHandler::~HttpRequestHandler()
{
}

void HttpRequestHandler::processRequest()
{
  HttpRequest httpRequest(m_dataInput);

  httpRequest.readHeader();

  StringStorage request;

  request.fromAnsiString(httpRequest.getRequest());

  if (!httpRequest.parseHeader()) {
    Log::warning(_T("invalid http request from %s"), m_peerHost.getString());
    return ;
  }

  request.replaceChar(_T('\n'), _T(' '));
  request.replaceChar(_T('\t'), _T(' '));

  Log::message(_T("\"%s\" from %s"), request.getString(), m_peerHost.getString());

  HttpReply reply(m_dataOutput);

  bool pageFound = false;

  if (strcmp(httpRequest.getFilename(), "/") == 0) {

    CharString paramsString("\n");

    bool isAppletArgsValid = true;

    bool paramsInUrlIsEnabled = Configurator::getInstance()->getServerConfig()->isAppletParamInUrlEnabled();

    if (httpRequest.hasArguments() && paramsInUrlIsEnabled) {
      ArgList *args = httpRequest.getArguments();

      for (size_t i = 0; i < args->getCount(); i++) {
        const char *key = args->getKey(i);

        AppletParameter parameter(key, args->getValue(key));

        if (!parameter.isValid()) {
          isAppletArgsValid = false;
          break;
        }

        paramsString.format("%s%s", paramsString.getString(),
                            parameter.getFormattedString());
      } 
    } 

    reply.send200();

    if (!isAppletArgsValid) {
      m_dataOutput->writeFully(HTTP_MSG_BADPARAMS, strlen(HTTP_MSG_BADPARAMS));
    } else {
      CharString page;

      StringStorage computerName(_T("TightVNC Server"));

      Environment::getComputerName(&computerName);

      size_t computerNameANSILength = computerName.getLength() + 1;

      char *computerNameANSI = new char[computerNameANSILength];

      computerName.toAnsiString(computerNameANSI, computerNameANSILength);

      page.format(HTTP_INDEX_PAGE_FORMAT,
                  computerNameANSI,
                  Configurator::getInstance()->getServerConfig()->getRfbPort(),
                  paramsString.getString());

      delete[] computerNameANSI;

      m_dataOutput->writeFully(page.getString(), page.getLength());
    } 

    pageFound = true;
  } else if ((strcmp(httpRequest.getFilename(), "/VncViewer.jar") == 0)) {
    reply.send200();
    m_dataOutput->writeFully(VNC_VIEWER_JAR_BODY, sizeof(VNC_VIEWER_JAR_BODY));

    pageFound = true;
  } 

  if (!pageFound) {
    reply.send404();
  }
}
