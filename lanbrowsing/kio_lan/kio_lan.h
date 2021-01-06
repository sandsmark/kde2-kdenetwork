/*  This file is part of the KDE project
    Copyright (C) 2000 Alexander Neundorf <neundorf@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KIO_XLAN_H
#define KIO_XLAN_H

#include <kio/slavebase.h>
#include <kio/tcpslavebase.h>
#include <kio/global.h>

#include <qcstring.h>
#include <qstring.h>
#include <qdict.h>

#include <iostream.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define KIOLAN_HTTP 0
#define KIOLAN_FTP 1
#define KIOLAN_SMB 2
#define KIOLAN_NFS 3
#define KIOLAN_MAX 4

struct MyProtocolInfo
{
   int enabled;
   int port;
   //this should be large enough for things like "FTP" and so on
   char name[8];
};

struct HostInfo
{
   time_t created;
   int ports[KIOLAN_MAX];
};

class LANProtocol : public KIO::TCPSlaveBase
{
   public:
      LANProtocol (int isLanIoSlave, const QCString &pool, const QCString &app );
      virtual ~LANProtocol();

      virtual void setHost( const QString& host, int port, const QString& user, const QString& pass );
      virtual void mimetype( const KURL& );

      virtual void listDir( const KURL& url);
      virtual void stat( const KURL & url);
      virtual void get( const KURL& url );

protected:
      QDict<HostInfo> m_hostInfoCache;
      int readDataFromServer();
      int lanReadDataFromServer();
      int rlanReadDataFromServer();
      int checkHost(const QString& host);
      int checkPort(int _port, in_addr ip);
      QString m_currentHost;
      unsigned short int m_port;
      MyProtocolInfo m_protocolInfo[KIOLAN_MAX];
      int m_maxAge;
      bool m_isLanIoslave;
      bool m_shortHostnames;
};

#endif
