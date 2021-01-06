/*    client.cpp
 *
 *    Copyright (c) 2000, Alexander Neundorf
 *    neundorf@kde.org
 *
 *    You may distribute under the terms of the GNU General Public
 *    License as specified in the COPYING file.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 */

#include "netmanager.h"
#include "client.h"
#include "lisadefines.h"
#include "getdebug.h"

#include <iostream>
#include <unistd.h>

#include <strings.h>

Client::Client(NetManager* parent, int socketFD, int closeOnDelete)
:parentServer(parent)
,m_fd(socketFD)
,m_done(0)
,m_closeOnDelete(closeOnDelete)
{
   getDebug()<<"created new client"<<endl;
   if (m_fd==-1) m_done=1;
   m_creationTime=time(0);
};

Client::Client()
:parentServer(0)
,m_fd(-1)
,m_done(1)
,m_closeOnDelete(1)
{
   m_creationTime=time(0);
};

Client::Client(const Client& c)
:parentServer(c.parentServer)
,m_fd(c.m_fd)
,m_creationTime(c.m_creationTime)
,m_done(c.m_done)
,m_closeOnDelete(1)
{
   getDebug()<<"Client::Client copy ctor"<<endl;
};

int Client::tryToGetInfo()
{
   getDebug()<<"Client::tryToGetInfo: m_done: "<<m_done<<endl;
   if (done()) return 1;
   if (m_fd==-1)
   {
      close();
      return 1;
   };
   getDebug()<<"Client::tryToGetInfo: calling data.getFile()"<<endl;
   if (!parentServer->isInformed()) return 0;
   //we fork now, so that writing to the client can't block the server process
   /*      int pid=fork();
    if (pid==-1)
    {
    //parent
    getDebug()<<"NetScanner::scan: error occured"<<endl;
    return 1;
    }
    else if (pid!=0)
    {
    //parent
    return 1;
    };*/
   //child
   //this one does it all :-)
   getDebug()<<"Client::tryToGetInfo: sending data to client"<<endl;
   parentServer->writeDataToFD(fd(),0);
   close();
   //exit(0);
   return 1;
};

void Client::close()
{
   if (m_fd!=-1) ::close(m_fd);
   m_fd=-1;
   m_done=1;
};

void Client::read()
{
   getDebug()<<"Client::read()"<<endl;
   if (m_fd==-1) return;
   char buf[1024];
   //the clients should not send us something !
   int result=::read(m_fd,buf,1024);
   if (result>=0) close();
};

Client::~Client()
{
   if (m_closeOnDelete)
      close();
};

