/*    netscanner.cpp
 *
 *    Copyright (c) 2000, Alexander Neundorf,
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

#include "config.h"
#include "netscanner.h"
#include "ipaddress.h"
#include "lisadefines.h"
#include "getdebug.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#ifdef __osf__
#undef BYTE_ORDER
#define _OSF_SOURCE
#undef _MACHINE_ENDIAN_H_
#undef __STDC__
#define __STDC__ 0
#include <netinet/ip.h>
#undef __STDC__
#define __STDC__ 1
#endif
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define getDebug() getDebug()<<procId

struct ICMPEchoRequest
{
   unsigned char type;
   unsigned char code;
   unsigned short int checkSum;
   unsigned short id;
   unsigned short seqNumber;
};

unsigned short in_cksum(unsigned short *addr, int len)
{
   int nleft = len;
	int sum(0);
	unsigned short	*w = addr;
	unsigned short	answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
   while (nleft > 1)
   {
      sum += *w++;
      nleft -= 2;
	}

   /* 4mop up an odd byte, if necessary */
   if (nleft == 1)
   {
		*(unsigned char *)(&answer) = *(unsigned char *)w ;
      sum += answer;
   }

   /* 4add back carry outs from top 16 bits to low 16 bits */
   sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
   sum += (sum >> 16);			/* add carry */
   answer = ~sum;				/* truncate to 16 bits */
   return(answer);
}


NetScanner::NetScanner(int& rawSocketFD, int strictMode)
:procId("")
,m_firstWait(5)
,m_secondWait(15)
,m_strictMode(strictMode)
,m_rawSocketFD(rawSocketFD)
,validator()
,ipRangeStr(";")
,m_maxPings(256)
,m_deliverUnnamedHosts(0)
,m_useNmblookup(0)
,hostList(0)
,tmpIPRange("")
{};

NetScanner::~NetScanner()
{
   delete hostList;
   ::close(m_rawSocketFD);
};

void addMissingSemicolon(MyString& text)
{
   if (text.isEmpty()) return;
   if (text[text.length()-1]!=';')
      text+=';';
};

void NetScanner::configure(Config& config)
{
   //ranges are not allowed in strict mode
   if (!m_strictMode)
   {
      ipRangeStr=stripWhiteSpace(config.getEntry("PingAddresses",""));
      addMissingSemicolon(ipRangeStr);
   };
   MyString pingNames=stripWhiteSpace(config.getEntry("PingNames",""));
   addMissingSemicolon(pingNames);
   MyString nextName;
   int semicolonPos=pingNames.find(';');
   int hostsAdded(0);
   while (semicolonPos!=-1)
   {
      nextName=pingNames.left(semicolonPos);
      getDebug()<<"NetScanner::configure(): looking up -"<<nextName<<"-"<<endl;
      //now the name lookup
      in_addr server_addr;
      hostent *hp=gethostbyname(nextName.data());
      if (hp!=0)
      {
         if ((m_strictMode) && (hostsAdded>=STRICTMODEMAXHOSTS))
            break;
         memcpy(&server_addr, hp->h_addr, hp->h_length);
         char *ip=inet_ntoa(server_addr);
         getDebug()<<"NetScanner::configure(): looking up "<<nextName<<" gives -"<<ip<<"-"<<endl;
         ipRangeStr=ipRangeStr+ip+';';
         hostsAdded++;
      }

      pingNames=pingNames.mid(semicolonPos+1);
      semicolonPos=pingNames.find(';');
   };
   if ((!ipRangeStr.isEmpty()) && (ipRangeStr[0]==';'))
      ipRangeStr=ipRangeStr.mid(1);
   m_deliverUnnamedHosts=config.getEntry("DeliverUnnamedHosts",0);
   m_useNmblookup=config.getEntry("SearchUsingNmblookup",0);
   m_maxPings=config.getEntry("MaxPingsAtOnce",256);
   m_firstWait=config.getEntry("FirstWait",5);
   m_secondWait=config.getEntry("SecondWait",15);
   if (m_firstWait<1) m_firstWait=1;
   if (m_maxPings<8) m_maxPings=8;
   if (m_maxPings>1024) m_maxPings=1024;
   //on some systems (Solaris ?) select() doesn't work correctly
   // if the microseconds are more than 1.000.000
   if (m_firstWait>99) m_firstWait=99;
   if (m_secondWait>99) m_secondWait=99;
   getDebug()<<"NetScanner::configure(): "<<ipRangeStr<<endl;
};

struct in_addr NetScanner::getIPfromArray(unsigned int index)
{
   //getDebug()<<endl<<"*** start ***"<<endl;
   unsigned int tmpIndex(0),indexLeft(index);
   resetIPRange();
   MyString tmp(getNextIPRange());
//   getDebug()<<"NetScanner::getIPFromArray: -"<<tmp<<"-"<<endl;
   while (tmp!="")
   {
      if (tmp.contains('/'))
      {
         //getDebug()<<"net/mask combination detected"<<endl;
         MyString netStr(tmp.left(tmp.find("/")));
         MyString maskStr(tmp.mid(tmp.find("/")+1));
         unsigned int net(IPAddress(netStr).asInt());
         unsigned int mask(IPAddress(maskStr).asInt());
         if ((~mask)<indexLeft)
         {
            indexLeft=indexLeft-(~mask+1);
            tmpIndex+=(~mask)+1;
            //getDebug()<<"i: "<<tmpIndex<<" left: "<<indexLeft<<endl;
         }
         else
         {
            net+=indexLeft;
            return IPAddress(net).asStruct();
            //return string2Struct(ipInt2String(net));
         };
      }
      else if (tmp.contains('-')==1)
      {
         //getDebug()<<"single range detected"<<endl;
         MyString fromIPStr(tmp.left(tmp.find("-")));
         MyString toIPStr(tmp.mid(tmp.find("-")+1));
         //getDebug()<<"fromIPStr: "<<fromIPStr<<endl;
         //getDebug()<<"toIPStr: "<<toIPStr<<endl;
         unsigned int fromIP(IPAddress(fromIPStr).asInt());
         unsigned int toIP(IPAddress(toIPStr).asInt());
         //unsigned int fromIP(ipString2Int(fromIPStr)), toIP(ipString2Int(toIPStr));
         //index hinter diesem bereich
         if ((fromIP+indexLeft)>toIP)
         {
            tmpIndex+=1+toIP-fromIP;
            indexLeft=indexLeft-(1+toIP-fromIP);
            //getDebug()<<"i: "<<tmpIndex<<" left: "<<indexLeft<<endl;
         }
         //index in diesem bereich
         else
         {
            fromIP+=indexLeft;
            return IPAddress(fromIP).asStruct();
            //return string2Struct(ipInt2String(fromIP));
         };
         
      }
      else if (tmp.contains('-')==4)
      {
         //getDebug()<<"multiple range detected"<<endl;
         int cp(tmp.find('-'));
         int from1(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('.');
         int to1(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('-');
         int from2(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('.');
         int to2(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('-');
         int from3(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('.');
         int to3(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         cp=tmp.find('-');
         int from4(atoi(tmp.left(cp).data()));
         tmp=tmp.mid(cp+1);

         int to4(atoi(tmp.data()));
         
         unsigned int count((1+to4-from4)*(1+to3-from3)*(1+to2-from2)*(1+to1-from1));
         if (count<indexLeft)
         {
            tmpIndex+=count;
            indexLeft-=count;
            //getDebug()<<"i: "<<tmpIndex<<" left: "<<indexLeft<<endl;
         }
         else
         {
            for (int b1=from1; b1<=to1; b1++)
               for (int b2=from2; b2<=to2; b2++)
                  for (int b3=from3; b3<=to3; b3++)
                     for (int b4=from4; b4<=to4; b4++)
                     {
                        if (tmpIndex==index)
                        {
                           return IPAddress(b1,b2,b3,b4).asStruct();
                        };
                        tmpIndex++;
                        indexLeft--;
                        //getDebug()<<"i: "<<tmpIndex<<" left: "<<indexLeft<<endl;
                     };
         };
      }
      //single IP address
      else if (tmp.contains('.')==3)
      {
         //getDebug()<<"single IP address detected"<<endl;
         //if (tmpIndex==index) return string2Struct(tmp);
         if (tmpIndex==index) return IPAddress(tmp).asStruct();
         else
         {
            tmpIndex++;
            indexLeft--;
            //getDebug()<<"i: "<<tmpIndex<<" left: "<<indexLeft<<endl;
         };
      };
      //getDebug()<<"nextIPRange: *"<<tmp<<"*"<<endl;
      tmp=getNextIPRange();
   };
   return IPAddress("0.0.0.0").asStruct();
};

void NetScanner::resetIPRange()
{
   tmpIPRange=ipRangeStr;
};

MyString NetScanner::getNextIPRange()
{
   if (tmpIPRange.contains(';')<1) return "";
   int cp(tmpIPRange.find(';'));
   MyString tmp(tmpIPRange.left(cp));
   tmpIPRange=tmpIPRange.mid(cp+1);
   return tmp;
};

char* NetScanner::ip2Name(struct in_addr ip)
{
   struct hostent *hostname=0;
   // Set the hostname of node
   if ( ( hostname = gethostbyaddr( (char *) &ip.s_addr, 4, AF_INET  ) ) == 0 )
   {
      getDebug() << "TCPNode::TCPNode->gethostbyname* error" << endl;
      return inet_ntoa( ip );
   }
   getDebug()<<"NetScanner::ip2name -"<<hostname->h_name<<endl;
   return hostname->h_name;
};

void NetScanner::nmblookupScan(SimpleList<Node>* newList)
{
   getDebug()<<"NetScanner::nmblookupScan()"<<endl;
   //newList->clear();
   FILE * nmblookupFile=popen("nmblookup \"*\"","r");
   //no success
   if (nmblookupFile==0)
   {
      getDebug()<<"NetScanner::nmblookupScan(): could not start nmblookup"<<endl;
      return;
   };
   MyString dummy("");
   char *receiveBuffer=0;
   int bufferSize(0);
   int nmblookupFd=fileno(nmblookupFile);
   struct timeval tv;
   fd_set fds;
   int done(0);
   int timeOuts(0);
   char *tmpBuf=new char[16*1024];
   do
   {
      FD_ZERO(&fds);
      FD_SET(nmblookupFd,&fds);
      tv.tv_sec=10;
      tv.tv_usec=0;
      int result=select(nmblookupFd+1,&fds,0,0,&tv);
      //error
      if (result<0)
         done=1;
      //timeout
      else if (result==0)
      {
         timeOuts++;
         //3 time outs make 30 seconds, this should be *much* more than enough
         if (timeOuts>=3)
            done=1;
      }
      else if (result>0)
      {
         //read stuff
         int bytesRead=::read(nmblookupFd,tmpBuf,16*1024);
         //pipe closed
         if (bytesRead==0)
            done=1;
         else
         {
            char *newBuf=new char[bufferSize+bytesRead];
            if (receiveBuffer!=0)
            {
               memcpy(newBuf,receiveBuffer,bufferSize);
               delete [] receiveBuffer;
            };
            memcpy(newBuf+bufferSize,tmpBuf,bytesRead);
            receiveBuffer=newBuf;
            bufferSize+=bytesRead;
         };
      };
   } while (!done);
   pclose(nmblookupFile);

   delete [] tmpBuf;
   //we received nothing
   if (receiveBuffer==0)
      return;

   //check for a terrminating '\0'
   if (receiveBuffer[bufferSize-1]=='\0')
      receiveBuffer[bufferSize-1]='\0';

   //cerr<<receiveBuffer<<endl;

   tmpBuf=receiveBuffer;
   int bytesLeft=bufferSize;

   int bufferState=0;
   while (bytesLeft>0)
   {
      //getDebug()<<"bytesLeft: "<<bytesLeft<<" received: "<<bufferSize<<endl;
      //since we added a terminating \0 we can be sure here
      char *endOfLine=(char*)memchr(tmpBuf,'\n',bytesLeft);
      //point to the last character
      if (endOfLine==0)
         endOfLine=receiveBuffer+bufferSize-1;

      //0-terminate the string
      *endOfLine='\0';
      //now tmpBuf to endOfLine is a 0-terminated string
      int strLength=strlen(tmpBuf);
      //hmm, if this happens, there must be something really wrong
      if (strLength>1000)
         break;

      bytesLeft=bytesLeft-strLength-1;

      if (bufferState==0)
      {
         if (isdigit(tmpBuf[0]))
            bufferState=1;
      };
      //yes, no else !
      if (bufferState==1)
      {
         char tmpIP[1024];
         //cerr<<"tmpBuf: -"<<tmpBuf<<"-"<<endl;
         sscanf(tmpBuf,"%s *<00>\n",tmpIP);
         getDebug()<<"NetScanner::nmblookupScan: tmpIP: -"<<tmpIP<<"-"<<endl;
         struct sockaddr_in addr;
         //if (inet_aton(tmpIP,&addr.sin_addr))
         if ((addr.sin_addr.s_addr = inet_addr(tmpIP)) != INADDR_NONE)
            if  (!hostAlreadyInList(addr.sin_addr.s_addr,newList))
            {
               if (validator.isValid(addr.sin_addr.s_addr))
               {
                  getDebug()<<"NetScanner::nmblookupScan: adding "<<inet_ntoa(addr.sin_addr)<<endl;
                  newList->append(Node(dummy,addr.sin_addr.s_addr));
               };
            };
      };
      tmpBuf=endOfLine+1;
   };
   getDebug()<<"NetScanner::nmblookupScan: finished"<<endl;
   delete [] receiveBuffer;
};

void NetScanner::pingScan(SimpleList<Node>* newList)   //the ping-version
{
   getDebug()<<"NetScanner::pingScan()"<<endl;
   //newList->clear();
   MyString dummy("");
   getDebug()<<"NetScanner::pingScan: m_maxPings: "<<m_maxPings<<endl;

/*   int bufferSize(60*1024);
   getDebug()<<"NetScanner::pingScan: regaining root privileges for setting up the icmp socket"<<endl;
   seteuid(0);
   int sockFD=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
   getDebug()<<"NetScanner::pingScan: setsockopt returns "<<setsockopt(sockFD,SOL_SOCKET,SO_RCVBUF,&bufferSize,sizeof(bufferSize))<<endl;
   //make the socket non-blocking
   //long int options = O_NONBLOCK | ::fcntl(sockFD, F_GETFL);
   //getDebug()<<"NetScanner::pingScan:: made it non blocking: "<<::fcntl( sockFD, F_SETFL, options )<<endl;
   seteuid(getuid());
   getDebug()<<"NetScanner::pingScan: dropped root privileges again"<<endl;*/

   pid_t pid=getpid();
   ICMPEchoRequest echo;
   echo.type=ICMP_ECHO;
   echo.code=0;
   echo.id=pid;
   echo.seqNumber=0;
   echo.checkSum=0;
   echo.checkSum=in_cksum((unsigned short *)&echo,8);

   char receiveBuf[16*1024];
   //before we start first read everything what might be in the receive buffer
   //of the raw socket

   timeval tv1;
   //wait a moment for answers
   tv1.tv_sec = 0;
   tv1.tv_usec = 0;
   fd_set clearSet;
   FD_ZERO(&clearSet);
   FD_SET(m_rawSocketFD,&clearSet);
   while(select(m_rawSocketFD,&clearSet,0,0,&tv1)>0)
   {
      ::recvfrom(m_rawSocketFD,(char*)&receiveBuf,16*1024,0,0,0);
      tv1.tv_sec = 0;
      tv1.tv_usec = 0;
      FD_ZERO(&clearSet);
      FD_SET(m_rawSocketFD,&clearSet);
   };
   //now the buffer should be empty
   
   //wait a moment for answers
   tv1.tv_sec = 0;
   tv1.tv_usec = m_firstWait*10*1000;//0.5 sec

   int loopCount(2);
   if (m_secondWait<0) loopCount=1;
   for (int repeatOnce=0; repeatOnce<loopCount; repeatOnce++)
   {
      getDebug()<<"******************** starting loop *****************"<<endl;
      unsigned int current(0);
      int finished(0);
      while (!finished)
      {
         for (int con=0; con<m_maxPings; con++)
         {
            struct in_addr tmpIP;
            do
            {
               tmpIP=getIPfromArray(current);
               current++;
               getDebug()<<"NetScanner::pingScan(): trying "<<inet_ntoa(tmpIP)<<endl;
               if (hostAlreadyInList(tmpIP.s_addr,newList))
                  getDebug()<<"already in list :-)"<<endl;
               if (!validator.isValid(tmpIP.s_addr))
                  getDebug()<<"NetScanner::pingScan(): invalid IP :-("<<endl;
               //ping only hosts which are allowed to receive the results
               //and which are not in the list
            } while ( (tmpIP.s_addr!=0)
                      && ((!validator.isValid(tmpIP.s_addr))
                      || (hostAlreadyInList(tmpIP.s_addr,newList))));

            finished=(tmpIP.s_addr==0);
            if (!finished)
            {
               //send the icmp echo request
               struct sockaddr_in toAddr;
               toAddr.sin_family = AF_INET;
               toAddr.sin_addr = tmpIP;
               toAddr.sin_port = 0;
               int sb=sendto(m_rawSocketFD,(char*)&echo,sizeof(echo),0,(sockaddr*)&toAddr,sizeof(toAddr));
               //int sb=sendto(sockFD,(char*)&echo,sizeof(echo),0,(sockaddr*)&toAddr,sizeof(toAddr));
               getDebug()<<"NetScanner::pingScan: pinging "<<inet_ntoa(toAddr.sin_addr)<<endl;
            }
            else break;
         };
         select(0,0,0,0,&tv1);
         //now read the answers, hopefully
         struct sockaddr_in fromAddr;
         socklen_t length(sizeof(fromAddr));
         int received(0);

         fd_set sockFDset;
         FD_ZERO(&sockFDset);
         FD_SET(m_rawSocketFD,&sockFDset);
         tv1.tv_sec=0;
         tv1.tv_usec=0;
         while(select(m_rawSocketFD+1,&sockFDset,0,0,&tv1)>0)
         {
            received=recvfrom(m_rawSocketFD, (char*)&receiveBuf, 16*1024, 0,
			      (sockaddr*)&fromAddr, &length);
            if (received!=-1)
            {
               getDebug()<<"NetScanner::pingScan: received from "<<inet_ntoa(fromAddr.sin_addr)<<" "<<received<<" b, ";
               struct ip *ipFrame=(ip*)&receiveBuf;
               int icmpOffset=(ipFrame->ip_hl)*4;
               icmp *recIcmpFrame=(icmp*)(receiveBuf+icmpOffset);
               int iType=recIcmpFrame->icmp_type;
               //if its a ICMP echo reply
               if ((iType==ICMP_ECHOREPLY)
                   //to an echo request we sent
                   && (recIcmpFrame->icmp_id==pid)
                   //and the host is not yet in the list
                   && (!hostAlreadyInList(fromAddr.sin_addr.s_addr,newList)))
               {
                  //this is an answer to our request :-)
                  getDebug()<<"NetScanner::pingScan: adding "<<inet_ntoa(fromAddr.sin_addr)<<endl;
                  newList->append(Node(dummy,fromAddr.sin_addr.s_addr));
               };
            };
            tv1.tv_sec=0;
            tv1.tv_usec=0;
            FD_ZERO(&sockFDset);
            FD_SET(m_rawSocketFD,&sockFDset);
            //FD_SET(sockFD,&sockFDset);
         };
      };
      tv1.tv_sec = 0;
      tv1.tv_usec = m_secondWait*10*1000;//0.5 sec
   };
   getDebug()<<"NetScanner::pingScan() ends"<<endl;
};

void NetScanner::doScan()
{
   getDebug()<<" NetScanner::doScan"<<endl;
   //child
   SimpleList<Node>* tmpPingList=new SimpleList<Node>;
   getDebug()<<" NetScanner::doScan: created list"<<endl;
   if (m_useNmblookup)
      nmblookupScan(tmpPingList);
   pingScan(tmpPingList);
   // get the names from cache or lookup
   completeNames(tmpPingList);
   getDebug()<<"NetScanner::doScan: completed names"<<endl;
   if (m_deliverUnnamedHosts==0)
      removeUnnamedHosts(tmpPingList);

   getDebug()<<"NetScanner::doScan: added hosts"<<endl;
   
   delete hostList;
   hostList=tmpPingList;
};

int NetScanner::hostAlreadyInList(int ip, SimpleList<Node>* nodes)
{
   for (Node* node=nodes->first(); node!=0; node=nodes->next())
   {
      if (node->ip==ip)
         return 1;
   };
   return 0;
};

void NetScanner::removeUnnamedHosts(SimpleList<Node>* nodes)
{
   struct in_addr tmpAddr;
   Node* pre(0);
   for (Node* node=nodes->first(); node!=0; node=nodes->next())
   {
      //in node this is already in NBO
      tmpAddr.s_addr=node->ip;
      const char* ip=inet_ntoa(tmpAddr);
      //this way we only compare the whole string via strcmp()
      //if at least the first char is already the same
      if (ip[0]==node->name[0])
      {
         //this host has no name
         if (strcmp(ip,node->name.data())==0)
         {
            nodes->remove(node);
            //set the pointer to the previous item
            if (pre==0) node=nodes->first();
            else node=pre;
         };
      };
      pre=node;
   };
};

void NetScanner::completeNames(SimpleList<Node>* nodes)
{
   struct sockaddr_in tmpAddr;
   //for every host
   for (Node* node=nodes->first(); node!=0; node=nodes->next())
   {
      tmpAddr.sin_addr.s_addr=node->ip;
      getDebug()<<"\n NetScanner::completeNames: looking up "<<inet_ntoa(tmpAddr.sin_addr)<<endl;
      int done(0);
      //first look wether we have the name already
      if (hostList!=0) for (Node* oldNode=hostList->first(); oldNode!=0; oldNode=hostList->next())
      {
         if (node->ip==oldNode->ip)
         {
            getDebug()<<"NetScanner::completeNames: cached: "<<oldNode->name<<" :-)"<<endl;
            node->name=oldNode->name;
            done=1;
            break;
         };
      };
      //otherwise do a name lookup
      if (!done)
      {
         //IPAddress tmpAddress(node->ip);
         //getDebug()<<"NetScanner::completeNames: doing actual lookup"<<endl;
         node->name=ip2Name(tmpAddr.sin_addr);
         getDebug()<<"NetScanner::completeNames: resolved: "<<node->name<<endl;
      };
   };
};

#undef getDebug

