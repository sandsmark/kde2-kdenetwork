/*    addressvalidator.cpp
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

#include "addressvalidator.h"
#include "mystring.h"
#include "getdebug.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

AddressValidator::AddressValidator(const MyString& addressSpecs)
//this is 127.0.0.0
:localhostNet(htonl(0x7f000000))
//with mask 255.255.255.0
,localhostMask(htonl(0xffffff00))
{
   clearSpecs();
   MyString tmp=addressSpecs;
   setValidAddresses(tmp);
};

AddressValidator::AddressValidator()
   //this is 127.0.0.0
   :localhostNet(htonl(0x7f000000))
   //with mask 255.255.255.0
   ,localhostMask(htonl(0xffffff00))
{
   clearSpecs();
};

AddressValidator::~AddressValidator()
{};

void AddressValidator::configure(Config& config)
{
   MyString tmp=stripWhiteSpace(config.getEntry("AllowedAddresses",""));
   tmp=tmp+";";
   setValidAddresses(tmp);
   getDebug()<<"AddressValidator::configure(): "<<tmp<<endl;
};


void AddressValidator::setValidAddresses(MyString addressSpecs)
{
   getDebug()<<"AddressValidator::setValidAddresses"<<endl;
   allowedHosts=addressSpecs;
   MyString nextPart;
   while (!addressSpecs.isEmpty())
   {
      getDebug()<<"AddressValidator::setValidAddresses: "<<addressSpecs<<endl;
      int pos=addressSpecs.find(";");
      nextPart=addressSpecs.left(pos);
      addressSpecs=addressSpecs.mid(pos+1);
      getDebug()<<"AddressValidator::setValidAddresses: nextPart: "<<nextPart<<endl;
      if (nextPart.contains('.')==3)
      {
         addSpec(EXACTADDR_SPEC,inet_addr(nextPart.data()));
         getDebug()<<"AddressValidator::setValidAddresses: exact addr: "
	 <<ios::hex<<inet_addr(nextPart.data())<<ios::dec<<endl;
      }
      else
      {
         pos=nextPart.find('/');
         MyString netStr=nextPart.left(pos);
         MyString maskStr=nextPart.mid(pos+1);
         int mask=inet_addr(maskStr.data());
         int net= (inet_addr(netStr.data()) & mask);
         getDebug()<<"AddressValidator::setValidAddresses: net/mask: "
	 <<ios::hex<<net<<"/"<<mask<<ios::dec<<endl;
         addSpec(NETMASK_SPEC,net,mask);
      };
   };
};

void AddressValidator::clearSpecs()
{
   allowedHosts="";
   for (int i=0; i<MAX_SPECS; i++)
   {
      specs[i].address=0;
      specs[i].mask=0;
      specs[i].typeOfSpec=NO_SPEC;
   };
};

void AddressValidator::addSpec(int type, int address, int mask)
{
   for (int i=0; i<MAX_SPECS; i++)
   {
      if (specs[i].typeOfSpec==NO_SPEC)
      {
         specs[i].address=address;
         specs[i].mask=mask;
         specs[i].typeOfSpec=type;
         return;
      };
   };
};

int AddressValidator::isValid(int addressNBO)
{
   getDebug()<<"AddressValidator::isValid: "<<ios::hex<<addressNBO<<ios::dec<<endl;
   //localhost is always allowed
   getDebug()<<"local net: "<<
   ios::hex<<localhostNet<<" mask: "<<localhostMask<<" AND: "<<(addressNBO &
   localhostMask)<<ios::dec<<endl;
   if ((addressNBO & localhostMask) == localhostNet)
      return 1;
      
   for (int i=0; i<MAX_SPECS; i++)
   {
      if (specs[i].typeOfSpec==NO_SPEC)
      {
         //since the specifications are always entered from the beginning
         //of the array, we already passed the last one if we get here
         //so we can return now "it is invalid !" ;-)
         return 0;
         //continue;
      }
      else if (specs[i].typeOfSpec==EXACTADDR_SPEC)
      {
         getDebug()<<"AddressValidator::isValid: comparing "
	 <<ios::hex<<specs[i].address<<ios::dec<<endl;
         if (addressNBO==specs[i].address)
         {
            getDebug()<<"AddressValidator::isValid: exact address"<<endl;
            return 1; // this one is allowed to :-)
         };
      }
      else if (specs[i].typeOfSpec==NETMASK_SPEC)
      {
         getDebug()<<"AddressValidator::isValid: ANDing "<<
	 ios::hex<<(addressNBO & specs[i].mask)<<" "<<specs[i].address<<ios::dec<<endl;
         if ((addressNBO & specs[i].mask) == specs[i].address)
         {
            getDebug()<<"AddressValidator::isValid: net/mask"<<endl;
            return 1;
         };
      };
   };
   //if ((addressNBO==htonl(0x0a040801)) || (addressNBO==htonl(0xc0a80001))) return 0;
   getDebug()<<"AddressValidator::isValid: invalid address"<<endl;
   return 0;
};

