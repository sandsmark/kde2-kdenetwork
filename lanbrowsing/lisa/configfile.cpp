/*    configfile.cpp
 *
 *    Copyright (c) 1998, 2000, Alexander Neundorf
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

#include "configfile.h"
#include "getdebug.h"

#include <iostream>
#include <fstream.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Config::Config(const MyString& name/*,String path*/)
{
   char buff[16*1024],c;
/*   String s,empty="#############################################################################################################################################################";
   String home=getenv("HOME");

   if (home!="") home+=String("/")+name;
   if (fexists(home)==0)
   {
      home=path+"/"+name;
      if (fexists(home)==0) 
      {
         home=name;
         if (fexists(home)==0) return;
      };
   };*/
   ifstream inf(name.data());
   if (!inf)
   {
      cout<<"could not open file "<<name<<endl;
      return;
   };
   getDebug()<<"Config::Config(): opened file "<<name<<endl;
   //read the file
   char key[256], value[16*1024];
   do
	{
      char* buffStart=buff;
		//inf.getline(buff,16*1024,'\n');
      int bufSize(16*1024);
      int lineBroken(0);
      do
      {
         lineBroken=0;
         inf.get(buffStart,bufSize,'\n');
         inf.get(c);
         int l=strlen(buffStart);
         if (buffStart[l-1]=='\\')
         {
            buffStart=buffStart+l-1;
            bufSize=bufSize+1-l;
            lineBroken=1;
         };
      } while ((lineBroken) && (!inf.eof()));
      //make it ignore comments
      char *theChar=strchr(buff,'#');
      if (theChar!=0)
         *theChar='\0';
      //now divide the line into key and value
      theChar=strchr(buff,'=');
      if (theChar!=0)
      {
         *theChar='\0';
         key[0]='\0';
         sscanf(buff,"%s",key);
         //do we have something valid ?
         if (key[0]!='\0')
         {
            //the char behind the = should be at least the terminating \0
            // so I can be sure to access valid memory here, IMO
            value[0]='\0';

            strcpy(value,theChar+1);
            //sscanf(theChar+1,"%s",value);
            if (value[0]!='\0')
            {
               //here we can be sure that the list will only contain
               //strings which are at least one char long
               getDebug()<<"Config::Config(): adding "<<key<<endl;
               MapEntry entry;
               entry.key=key;
               entry.value=value;
               liste.append(entry);
            };
         };
      };
	}
   while (!inf.eof());
};

MyString Config::getEntry(const char *key, const char* defaultValue)
{
   //this should be faster than strlen()
   if ((key==0) || (key[0]=='\0'))
      return defaultValue;

   getDebug()<<"Config::getEntry(): searching "<<key<<endl;
   for (MapEntry *entry=liste.first(); entry!=0; entry=liste.next())
   {
      //well, I don't have hashes, but maybe this already
      //helps a bit to make it faster, since we only compare the whole
      //string if the first char is already the same
      
      getDebug()<<"Config::getEntry(): comparing "<<key[0]<<" and "<<entry->key[0]<<endl;
      if (entry->key[0]==key[0])
      {
         getDebug()<<"Config::getEntry(): the same :-), now comparing "<<key<<" with "<<entry->key<<endl;
         if (entry->key==key)
         {
            getDebug()<<"Config::getEntry(): found :-)"<<key<<endl;
            return entry->value;
         };
      };
   };
   return defaultValue;
};

int Config::getEntry(const char *key, int defaultValue)
{
   char def[100];
   sprintf(def,"%d",defaultValue);
   MyString tmp=stripWhiteSpace(getEntry(key,def));
   int i(0);
   int ok=sscanf(tmp.c_str(),"%d",&i);
   if (ok==1) return i;
   return defaultValue;
};

