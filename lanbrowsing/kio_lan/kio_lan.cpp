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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kdebug.h>
#include <kio/global.h>
#include <klocale.h>
#include <kinstance.h>
#include <kconfig.h>
#include <kprocess.h>

#include <qfile.h>
#include <qstringlist.h>

#include <iostream.h>
#include <string.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>

#include "kio_lan.h"

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

#define PORTSETTINGS_CHECK 0
#define PORTSETTINGS_PROVIDE 1
#define PORTSETTINGS_DISABLE 2

using namespace KIO;

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

extern "C" { int kdemain(int argc, char **argv); }

int kdemain( int argc, char **argv )
{
   KInstance instance( "kio_lan" );

  if (argc != 4)
  {
     fprintf(stderr, "Usage: kio_lan protocol domain-socket1 domain-socket2\n");
     exit(-1);
  }
  int isLanIoslave=(strcmp("lan",argv[1])==0);

  kdDebug(7101) << "LAN: kdemain: starting" << endl;

  LANProtocol slave(isLanIoslave, argv[2], argv[3]);
  slave.dispatchLoop();
  return 0;
}

LANProtocol::LANProtocol(int isLanIoslave, const QCString &pool, const QCString &app )
:TCPSlaveBase(7741,isLanIoslave?"lan":"rlan", pool, app)
,m_currentHost("")
,m_port(7741)
,m_maxAge(15*60)
,m_isLanIoslave(isLanIoslave?true:false)
{
   KConfig *config=new KConfig("kio_lanrc");

   m_protocolInfo[KIOLAN_FTP].enabled=config->readNumEntry("Support_FTP",PORTSETTINGS_CHECK);
   m_protocolInfo[KIOLAN_HTTP].enabled=config->readNumEntry("Support_HTTP",PORTSETTINGS_CHECK);
   m_protocolInfo[KIOLAN_NFS].enabled=config->readNumEntry("Support_NFS",PORTSETTINGS_CHECK);
   m_protocolInfo[KIOLAN_SMB].enabled=config->readNumEntry("Support_SMB",PORTSETTINGS_CHECK);

   m_shortHostnames=config->readBoolEntry("ShowShortHostnames",false);

   m_maxAge=config->readNumEntry("MaxAge",15)*60;
   if (m_maxAge<0) m_maxAge=0;

   strcpy(m_protocolInfo[KIOLAN_NFS].name,"NFS");
   strcpy(m_protocolInfo[KIOLAN_FTP].name,"FTP");
   strcpy(m_protocolInfo[KIOLAN_SMB].name,"SMB");
   strcpy(m_protocolInfo[KIOLAN_HTTP].name,"HTTP");

   m_protocolInfo[KIOLAN_NFS].port=2049;
   m_protocolInfo[KIOLAN_FTP].port=21;
   m_protocolInfo[KIOLAN_SMB].port=139;
   m_protocolInfo[KIOLAN_HTTP].port=80;

   m_hostInfoCache.setAutoDelete(true);

   delete config;
};

LANProtocol::~LANProtocol()
{
   m_hostInfoCache.clear();
};

int LANProtocol::readDataFromServer()
{
   if (m_isLanIoslave)
      return lanReadDataFromServer();
   else
      return rlanReadDataFromServer();
   return 0;
};

int LANProtocol::lanReadDataFromServer()
{
   kdDebug(7101)<<"LANProtocol::lanReadDataFromServer() host: "<<m_currentHost<<" port: "<<m_port<<endl;
   ConnectToHost(m_currentHost.latin1(), m_port);
   kdDebug(7101)<<"LANProtocol::lanReadDataFromServer() connected"<<endl;

   int receivedBytes(0);
   char* receiveBuffer(0);
   char tmpBuf[64*1024];
   int bytesRead(0);
   do
   {
      fd_set tmpFDs;
      FD_ZERO(&tmpFDs);
      FD_SET(m_iSock,&tmpFDs);
      timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      select(m_iSock+1,&tmpFDs,0,0,&tv);
      if (FD_ISSET(m_iSock,&tmpFDs))
      {
         bytesRead=Read(tmpBuf,64*1024);
         kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: read "<<bytesRead<<" bytes"<<endl;

         if (bytesRead>0)
         {
            char *newBuf=new char[receivedBytes+bytesRead];
            if (receiveBuffer!=0) memcpy(newBuf,receiveBuffer,receivedBytes);
            memcpy(newBuf+receivedBytes,tmpBuf,bytesRead);
            receivedBytes+=bytesRead;
            if (receiveBuffer!=0) delete [] receiveBuffer;
            receiveBuffer=newBuf;
         };
      };
   } while (bytesRead>0);
   CloseDescriptor();
   if ((bytesRead<0) || (receivedBytes<4))
   {
      delete [] receiveBuffer;
      error(ERR_INTERNAL_SERVER,i18n("Received unexpected data from %1").arg(m_currentHost));
      return 0;
   };

   UDSEntry entry;

   char *currentBuf=receiveBuffer;
   int bytesLeft=receivedBytes;
   int tmpIP;
   //this should be large enough for a name
   char tmpName[1024];
   //this should be large enough for the hostname
   char tmpHostname[512];
   while (bytesLeft>0)
   {
      if ((memchr(currentBuf,0,bytesLeft)==0) || (memchr(currentBuf,int('\n'),bytesLeft)==0))
      {
         delete [] receiveBuffer;
         error(ERR_INTERNAL_SERVER,i18n("Received unexpected data from %1").arg(m_currentHost));
         return 0;
      };
      kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: processing "<<currentBuf;
      sscanf(currentBuf,"%u %s\n",&tmpIP,tmpName);
      //since we check for 0 and \n with memchr() we can be sure
      //at this point that tmpBuf is correctly terminated
      int length=strlen(currentBuf)+1;
      bytesLeft-=length;
      currentBuf+=length;
      if ((bytesLeft==0) && (strstr(tmpName,"succeeded")!=0) && ((tmpIP==0) ||(tmpIP==1)))
      {
         kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: succeeded"<<endl;
      }
      else
      {
         kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: listing host: "<<tmpName<<" with ip: "<<tmpIP<<endl;
         UDSAtom atom;

         atom.m_uds = KIO::UDS_NAME;
         if (m_shortHostnames)
         {
            if (inet_addr(tmpName)!=-1)
               atom.m_str=tmpName;
            else
            {
               sscanf(tmpName,"%[^.]",tmpHostname);
               kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: Hostname of " << tmpName <<  " is " << tmpHostname << "\n";
               atom.m_str = tmpHostname;
            };
         }
         else
            atom.m_str = tmpName;

         entry.append( atom );
         atom.m_uds = KIO::UDS_SIZE;
         atom.m_long = 1024;
         entry.append(atom);
         atom.m_uds = KIO::UDS_ACCESS;
         atom.m_long = S_IRUSR | S_IRGRP | S_IROTH ;
         //atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
         entry.append(atom);
         atom.m_uds = KIO::UDS_FILE_TYPE;
         atom.m_long = S_IFDIR; // it is always a directory
         entry.append( atom );
         listEntry(entry,false);
      };
   };

   listEntry( entry, true ); // ready
   delete [] receiveBuffer;
   return 1;
};

int LANProtocol::rlanReadDataFromServer()
{
   kdDebug(7101)<<"RLANProtocol::readDataFromServer"<<endl;

   int sockFD=socket(AF_LOCAL, SOCK_STREAM, 0);
   sockaddr_un addr;
   memset((char*)&addr,0,sizeof(addr));
   addr.sun_family=AF_LOCAL;
   QCString socketname="/tmp/resLisa-";

   struct passwd *user = getpwuid( getuid() );
   if ( user )
      socketname+=user->pw_name;
   else
      //should never happen
      socketname+="???";

   strncpy(addr.sun_path,socketname,sizeof(addr.sun_path));
   addr.sun_path[sizeof(addr.sun_path)-1] = '\0';
   int result=::connect(sockFD,(sockaddr*)&addr, sizeof(addr));

   kdDebug(7101)<<"readDataFromServer(): result: "<<result<<" name: "<<addr.sun_path<<" socket: "<<sockFD<<endl;

   if (result!=0)
   {
      ::close(sockFD);
      KProcess proc;
      proc<<"reslisa";

      bool ok=proc.start(KProcess::DontCare);
      if (!ok)
      {
         error( ERR_CANNOT_LAUNCH_PROCESS, "reslisa" );
         return 0;
      };
      //wait a moment
      //reslisa starts kde-config, then does up to 64
      //name lookups and then starts to ping
      //results won't be available before this is done
      kdDebug(7101)<<"sleeping..."<<endl;
      ::sleep(1);
      kdDebug(7101)<<"sleeping again..."<<endl;
      ::sleep(5);
      kdDebug(7101)<<"woke up "<<endl;
      sockFD=socket(AF_LOCAL, SOCK_STREAM, 0);

      memset((char*)&addr,0,sizeof(addr));
      addr.sun_family=AF_LOCAL;
      strncpy(addr.sun_path,socketname,sizeof(addr.sun_path));
      addr.sun_path[sizeof(addr.sun_path)-1] = '\0';

      kdDebug(7101)<<"connecting..."<<endl;
      result=::connect(sockFD,(sockaddr*)&addr, sizeof(addr));
      kdDebug(7101)<<"readDataFromServer() after starting reslisa: result: "<<result<<" name: "<<addr.sun_path<<" socket: "<<sockFD<<endl;
      if (result!=0)
      {
         error( ERR_CANNOT_OPEN_FOR_READING, socketname );
         return 0;
      };
      kdDebug(7101)<<"succeeded :-)"<<endl;
   };

   int receivedBytes(0);
   char* receiveBuffer(0);
   char tmpBuf[64*1024];
   int bytesRead(0);
   do
   {
      fd_set tmpFDs;
      FD_ZERO(&tmpFDs);
      FD_SET(sockFD,&tmpFDs);
      timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      select(sockFD+1,&tmpFDs,0,0,&tv);
      if (FD_ISSET(sockFD,&tmpFDs))
      {
         bytesRead=::read(sockFD,tmpBuf,64*1024);
         kdDebug(7101)<<"RLANProtocol::readDataFromServer: read "<<bytesRead<<" bytes"<<endl;

         if (bytesRead>0)
         {
            char *newBuf=new char[receivedBytes+bytesRead];
            if (receiveBuffer!=0) memcpy(newBuf,receiveBuffer,receivedBytes);
            memcpy(newBuf+receivedBytes,tmpBuf,bytesRead);
            receivedBytes+=bytesRead;
            if (receiveBuffer!=0) delete [] receiveBuffer;
            receiveBuffer=newBuf;
         };
      };
   } while (bytesRead>0);
   ::close(sockFD);


   if ((bytesRead<0) || (receivedBytes<4))
   {
      delete [] receiveBuffer;
      error(ERR_CANNOT_OPEN_FOR_READING,socketname);
      return 0;
   };


   UDSEntry entry;

   char *currentBuf=receiveBuffer;
   int bytesLeft=receivedBytes;
   int tmpIP;
   //this should be large enough for a name
   char tmpName[1024];
   //this should be large enough for the hostname
   char tmpHostname[512];
   while (bytesLeft>0)
   {
      if ((memchr(currentBuf,0,bytesLeft)==0) || (memchr(currentBuf,int('\n'),bytesLeft)==0))
      {
         delete [] receiveBuffer;
         error(ERR_INTERNAL_SERVER,i18n("Received unexpected data from %1").arg(socketname));
         return 0;
      };
      kdDebug(7101)<<"RLANProtocol::readDataFromServer: processing "<<currentBuf;
      sscanf(currentBuf,"%u %s\n",&tmpIP,tmpName);
      //since we check for 0 and \n with memchr() we can be sure
      //at this point that tmpBuf is correctly terminated
      int length=strlen(currentBuf)+1;
      bytesLeft-=length;
      currentBuf+=length;
      if ((bytesLeft==0) && (strstr(tmpName,"succeeded")!=0) && ((tmpIP==0) ||(tmpIP==1)))
      {
         kdDebug(7101)<<"RLANProtocol::readDataFromServer: succeeded"<<endl;
      }
      else
      {
         kdDebug(7101)<<"RLANProtocol::readDataFromServer: listing host: "<<tmpName<<" with ip: "<<tmpIP<<endl;
         UDSAtom atom;

         atom.m_uds = KIO::UDS_NAME;
         if (m_shortHostnames)
         {
            if (inet_addr(tmpName)!=-1)
               atom.m_str=tmpName;
            else
            {
               sscanf(tmpName,"%[^.]",tmpHostname);
               kdDebug(7101)<<"LANProtocol::lanReadDataFromServer: Hostname of " << tmpName <<  " is " << tmpHostname << "\n";
               atom.m_str = tmpHostname;
            };
         }
         else
            atom.m_str = tmpName;
         entry.append( atom );
         atom.m_uds = KIO::UDS_SIZE;
         atom.m_long = 1024;
         entry.append(atom);
         atom.m_uds = KIO::UDS_ACCESS;
         atom.m_long = S_IRUSR | S_IRGRP | S_IROTH ;
         //atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
         entry.append(atom);
         atom.m_uds = KIO::UDS_FILE_TYPE;
         atom.m_long = S_IFDIR; // it is always a directory
         entry.append( atom );
         listEntry(entry,false);
      };
   };

   listEntry( entry, true ); // ready
   delete [] receiveBuffer;
   return 1;
};

int LANProtocol::checkHost(const QString& host)
{
   kdDebug(7101)<<"LAN::checkHost() "<<host<<endl;

   QString hostUpper=host.upper();
   HostInfo* hostInfo=m_hostInfoCache[hostUpper];
   if (hostInfo!=0)
   {
      kdDebug(7101)<<"LAN::checkHost() getting from cache"<<endl;
      //this entry is to old, we delete it !
      if ((time(0)-hostInfo->created)>m_maxAge)
      {
         kdDebug(7101)<<"LAN::checkHost() cache content to old, deleting it"<<endl;
         m_hostInfoCache.remove(hostUpper);
         hostInfo=0;
      };
   };
   if (hostInfo==0)
   {
      hostInfo=new HostInfo;
      in_addr ip;

      struct hostent *hp=gethostbyname(host.latin1());
      if (hp==0)
      {
         error( ERR_UNKNOWN_HOST, host.latin1() );
         delete hostInfo;
         return 0;
      }
      memcpy(&ip, hp->h_addr, hp->h_length);

      for (int i=0; i<KIOLAN_MAX; i++)
      {
         int result(0);
         if (m_protocolInfo[i].enabled==PORTSETTINGS_DISABLE)
            result=0;
         else if (m_protocolInfo[i].enabled==PORTSETTINGS_PROVIDE)
            result=1;
         else if (m_protocolInfo[i].enabled==PORTSETTINGS_CHECK)
            result=checkPort(m_protocolInfo[i].port,ip);

         //host not reachable
         if (result==-1)
         {
            delete hostInfo;
            hostInfo=0;
            error( ERR_UNKNOWN_HOST, host.latin1() );
            return 0;
         };
         hostInfo->ports[i]=result;
      };
      hostInfo->created=time(0);
      m_hostInfoCache.insert(hostUpper,hostInfo);
   };
   //here hostInfo is always != 0
   if (hostInfo==0)
   {
      error( ERR_INTERNAL, "hostInfo==0" );
      return 0;
   };

   UDSEntry entry;
   for (int i=0; i<KIOLAN_MAX; i++)
   {
      if (hostInfo->ports[i]==1)
      {
         UDSAtom atom;
         atom.m_uds = KIO::UDS_NAME;
         atom.m_str = m_protocolInfo[i].name;
         entry.append( atom );
         atom.m_uds = KIO::UDS_SIZE;
         atom.m_long = 1024;
         entry.append(atom);
         atom.m_uds = KIO::UDS_ACCESS;
         atom.m_long = S_IRUSR | S_IRGRP | S_IROTH ;
         //atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
         entry.append(atom);
         atom.m_uds = KIO::UDS_FILE_TYPE;
         if (strcmp(m_protocolInfo[i].name,"HTTP")==0)
         {
            atom.m_long=S_IFREG;
            entry.append(atom);

            atom.m_uds = KIO::UDS_MIME_TYPE;
            atom.m_str="text/html";
            entry.append( atom );
         }
         else
         {
            atom.m_long = S_IFDIR; // it is always a directory
            entry.append(atom);
            atom.m_uds = KIO::UDS_MIME_TYPE;
            atom.m_str="inode/directory";
            entry.append( atom );
         }
         listEntry(entry,false);
      };
   };
   listEntry( entry, true ); // ready
   return 1;
};

//checks the port using a nonblocking connect
int LANProtocol::checkPort( int _port, in_addr ip )
{
   kdDebug(7101)<<"LANProtocol::checkPort: "<<_port<<endl;
   struct sockaddr_in to_scan;

   to_scan.sin_family = AF_INET;
   to_scan.sin_addr = ip;
   to_scan.sin_port = htons(_port);

   // open a TCP socket
   int mysocket = ::socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (mysocket< 0 )
   {
      cerr << "Node::scanPort -> Error while opening Socket" << endl;
      ::close( mysocket );
      return 0;
   }

   //make the socket non blocking
   long int options = O_NONBLOCK | ::fcntl(mysocket, F_GETFL);
   if (::fcntl( mysocket, F_SETFL, options )!=0)
   {
      cerr << "Node::scanPort -> Error making it nonblocking"<< endl;
      ::close( mysocket );
      return 0;
   };

   int result=connect( mysocket, (struct sockaddr *) &to_scan, sizeof( to_scan ));
   //it succeeded immediately
   if (result==0)
   {
      cerr<<"LANProtocol::checkPort("<<_port<<") connect succeeded immediatly"<<endl;
      ::shutdown( mysocket, SHUT_RDWR );
      return 1;
   };
   //it failed
   if ((result<0) && (errno!=EINPROGRESS))
   {
      ::shutdown( mysocket, SHUT_RDWR );
      return 0;
   };
   timeval tv;
   tv.tv_sec=5;
   tv.tv_usec=0;
   fd_set rSet, wSet;
   FD_ZERO(&rSet);
   FD_SET(mysocket,&rSet);
   wSet=rSet;
   //timeout or error
   result=select(mysocket+1,&rSet,&wSet,0,&tv);
   if (result==1)
   {
      ::shutdown( mysocket, SHUT_RDWR );
      return 1;
   }
   else if (result==0)
   {
      //timeout, host not reachable
      ::shutdown( mysocket, SHUT_RDWR );
      return -1;
   };

   /*if (FD_ISSET(mysocket,&rSet) || FD_ISSET(mysocket,&wSet))
   {
      int error(0);
      int length(sizeof(error));
      if (getsockopt(mysocket,SOL_SOCKET,SO_ERROR, &error, &length)<0)
      {
         ::shutdown( mysocket, SHUT_RDWR );
         return 0;
      };
      ::shutdown( mysocket, SHUT_RDWR );
      return 1;
   };*/
   ::shutdown( mysocket, SHUT_RDWR );
   return 0;

   /*if (result<1)
   {
      ::shutdown( mysocket, SHUT_RDWR );
      return 0;
   };
   cerr<<"LANProtocol::checkPort("<<_port<<") select succeeded "<<result<<endl;
   ::shutdown( mysocket, SHUT_RDWR );
   return 1;*/
}

void LANProtocol::setHost( const QString& host, int port, const QString& s1, const QString& s2)
{
   if (m_isLanIoslave)
   {
      m_currentHost=host;
      if (port==0)
         m_port=7741;
      else
         m_port=port;
      kdDebug(7101)<<"LANProtocol::setHost: "<<m_currentHost<<endl;
   }
   else
   {
      if (!host.isEmpty())
         error(ERR_MALFORMED_URL,i18n("No hosts allowed in rlan:/ url"));
   };
};

void LANProtocol::mimetype( const KURL& url)
{
   kdDebug(7101)<<"LANProtocol::mimetype -"<<url.prettyURL()<<"-"<<endl;
   QString path( QFile::encodeName(url.path()));
   QStringList pathList=QStringList::split( "/",path);
   if ((pathList.count()==2) && (pathList[1].upper()=="HTTP"))
   {
      //kdDebug(7101)<<"LANProtocol::mimeType text/html"<<endl;
      mimeType("text/html");
   }
   else
   {
      mimeType("inode/directory");
      //kdDebug(7101)<<"LANProtocol::mimeType inode/directory"<<endl;
   };
   finished();
};

void LANProtocol::listDir( const KURL& _url)
{
   KURL url(_url);
   QString path( QFile::encodeName(url.path()));
   if (path.isEmpty())
   {
      url.setPath("/");
      redirection(url);
      finished();
      return;
   };
   if ((m_currentHost.isEmpty()) && (m_isLanIoslave))
   {
      url.setHost("localhost");
      redirection(url);
      finished();
      return;
   };
   kdDebug(7101)<<"LANProtocol::listDir: host: "<<m_currentHost<<" port: "<<m_port<<" path: "<<path<<endl;
   QStringList pathList=QStringList::split("/", path);
   kdDebug(7101)<<"parts are: "<<endl;
   for (QStringList::Iterator it=pathList.begin(); it !=pathList.end(); it++)
      kdDebug(7101)<<"-"<<(*it)<<"-"<<endl;
   if (pathList.count()>2)
   {
      kdDebug(7101)<<"LANProtocol::listDir: to deep path: "<<pathList.count()<<endl;
      error(ERR_DOES_NOT_EXIST,_url.prettyURL());
      return;
   };
   int succeeded(0);
   if (path=="/")
   {
      //get the stuff from the netland server
      succeeded=readDataFromServer();
   }
   else if (pathList.count()==1)
   {
      //check the ports and stuff
      succeeded=checkHost(pathList[0]);
   }
   else
   {
      kdDebug(7101)<<"LANProtocol::listDir: trying to redirect"<<endl;
      int isSupportedProtocol(0);
      for (int i=0; i<KIOLAN_MAX; i++)
      {
         if (pathList[1].upper()==m_protocolInfo[i].name)
         {
            isSupportedProtocol=m_protocolInfo[i].enabled;
            break;
         };
      };
      if (isSupportedProtocol==PORTSETTINGS_DISABLE)
      {
         kdDebug(7101)<<"LANProtocol::listDir: protocol not enabled "<<endl;
         error(ERR_DOES_NOT_EXIST,_url.prettyURL());
         return;
      };
      //redirect it
      KURL newUrl(pathList[1]+"://"+pathList[0]);
      redirection(newUrl);
      succeeded=1;
   };
   if (succeeded) finished();
};

void LANProtocol::stat( const KURL & url)
{
   kdDebug(7101)<<"LANProtocol::stat: "<<endl;
   UDSEntry entry;
   UDSAtom atom;

   atom.m_uds = KIO::UDS_NAME;
   atom.m_str = url.path();
   entry.append( atom );
   atom.m_uds = KIO::UDS_SIZE;
   atom.m_long = 1024;
   entry.append(atom);

   atom.m_uds = KIO::UDS_ACCESS;
   atom.m_long = S_IRUSR | S_IRGRP | S_IROTH ;
   //atom.m_long = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
   entry.append(atom);


   QString path( QFile::encodeName(url.path()));
   QStringList pathList=QStringList::split( "/",path);
   if ((pathList.count()==2) && (pathList[1].upper()=="HTTP"))
   {
      atom.m_uds = KIO::UDS_FILE_TYPE;
      atom.m_long=S_IFREG;
      entry.append(atom);
      atom.m_uds = KIO::UDS_MIME_TYPE;
      atom.m_str="text/html";
      //kdDebug(7101)<<"LANProtocol::stat: http is reg file"<<endl;
      entry.append( atom );
   }
   else
   {
      atom.m_uds = KIO::UDS_FILE_TYPE;
      atom.m_long = S_IFDIR; // it is always a directory
      entry.append(atom);
      atom.m_uds = KIO::UDS_MIME_TYPE;
      atom.m_str="inode/directory";
      //kdDebug(7101)<<"LANProtocol::stat: is dir"<<endl;
      entry.append( atom );
   };

   statEntry( entry );
   finished();
};

void LANProtocol::get(const KURL& url )
{
   QString path(QFile::encodeName(url.path()));

   kdDebug(7101)<<"LANProtocol::get: "<<path<<endl;
   QStringList pathList=QStringList::split("/", path);
   kdDebug(7101)<<"parts are: "<<endl;
   for (QStringList::Iterator it=pathList.begin(); it !=pathList.end(); it++)
      kdDebug(7101)<<"-"<<(*it)<<"-"<<endl;
   if ((pathList.count()!=2) || (pathList[1].upper()!="HTTP"))
   {
      kdDebug(7101)<<"LANProtocol::get: invalid url: "<<pathList.count()<<endl;
      error(ERR_DOES_NOT_EXIST,url.prettyURL());
      return;
   };
   KURL newUrl("http://"+pathList[0]);
   redirection(newUrl);
   finished();
   return;
};
