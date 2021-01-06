/*
 * findnic.cpp
 *
 * Copyright (c) 2001 Alexander Neundorf <neundorf@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef USE_SOLARIS
#define BSD_COMP
#endif

#include "findnic.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*struct MyNIC
{
   struct sockaddr_in addr;
   struct sockaddr_in netmask;
   bool bcast;
   bool loopback;
};

typedef QList<MyNIC> NICList;*/

NICList* findNICs()
{
   NICList* nl=new NICList;
   nl->setAutoDelete(true);

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

   char buf[8*1024];
   struct ifconf ifc;
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_req = (struct ifreq *) buf;
	int result=ioctl(sockfd, SIOCGIFCONF, &ifc);

   for (char* ptr = buf; ptr < buf + ifc.ifc_len; )
   {
      struct ifreq *ifr =(struct ifreq *) ptr;
      int len = sizeof(struct sockaddr);
#ifdef	HAVE_SOCKADDR_SA_LEN
      if (ifr->ifr_addr.sa_len > len)
         len = ifr->ifr_addr.sa_len;		/* length > 16 */
#endif
      ptr += sizeof(ifr->ifr_name) + len;	/* for next one in buffer */

      int flags;
      struct sockaddr_in *sinptr;
      switch (ifr->ifr_addr.sa_family)
      {
      case AF_INET:
         sinptr = (struct sockaddr_in *) &ifr->ifr_addr;
         flags=0;
         //printf("%s\t%s \n", ifr->ifr_name,inet_ntoa(sinptr->sin_addr));

         struct ifreq ifcopy;
         ifcopy=*ifr;
         result=ioctl(sockfd,SIOCGIFFLAGS,&ifcopy);
         //cout<<"result from ioctl: "<<result<<endl;
         flags=ifcopy.ifr_flags;
         if (((flags & IFF_UP) == IFF_UP)
            && ((flags & IFF_BROADCAST) == IFF_BROADCAST)
            && ((flags & IFF_LOOPBACK) == 0))
         {
            //yes, we are interested in this one :-)
            ifcopy=*ifr;
            result=ioctl(sockfd,SIOCGIFNETMASK,&ifcopy);
            //cout<<"result from ioctl: "<<result<<endl;
            MyNIC *tmp=new MyNIC;
            tmp->name=ifr->ifr_name;
            tmp->addr=*sinptr;
            sinptr = (struct sockaddr_in *) &ifcopy.ifr_addr;
            //flags=0;
            //printf("%s\t%s \n", ifcopy.ifr_name,inet_ntoa(sinptr->sin_addr));
            tmp->netmask=*sinptr;
            nl->append(tmp);
         };


         break;

      default:
         //printf("%s\n", ifr->ifr_name);
         break;
      }
   }
   return nl;
};

