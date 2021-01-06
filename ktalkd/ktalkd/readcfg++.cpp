/* This file is part of ktalkd

    Copyright (C) 1997 David Faure (faure@kde.org)

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
    Boston, MA 02111-1307, USA. */

/*
 * Routines for reading configuration from KDE configuration
 *  for ktalkd.
 */

/* Unix includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>

/* KDE & Qt includes */
#include <qstring.h>
#include <kapp.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <qfile.h>

/* Ktalkd includes */
#include "readconf.h"
#include "defs.h"
#include <config.h>

#include "print.h"

/** Converts a string s into a boolean. Handles 0/1, on/off, true/false. 
 * (case insensitive) */
int booleanresult(const char * s)
{
     if (strlen(s)==1)
	  { return atoi(s); }
     else if ((!strncasecmp(s,"on",2))||(!strncasecmp(s,"true",4))) {return 1;}
     else if ((!strncasecmp(s,"off",3))||(!strncasecmp(s,"false",5))){return 0;}
     else {
	  syslog(LOG_ERR,"Wrong boolean value %s in ktalkdrc",s);
	  return 0;
     }
}

/*  User configuration file, ktalkdrc in localconfigdir(). */
KConfig * ktalkdcfg = 0;
/*  User config file for talk announces, ktalkannouncerc in localconfigdir(). */
KConfig * ktkanncfg = 0;

/*  Initiate user-config-file reading. */
int init_user_config(const char * l_name)
{
  struct passwd * pw = getpwnam(l_name);

  if (!pw) return 0;
  else {
      struct stat buf;
      QString cfgFileName, tkannFileName;

      /* Set $HOME, because KInstance uses it */
      setenv("HOME",pw->pw_dir,1 /* overwrite */); 
      unsetenv("KDEHOME");
      unsetenv("XAUTHORITY");
message("%s",pw->pw_dir);
      KInstance tmpInstance("tmpInstance");
      cfgFileName = locateLocal("config", "ktalkdrc", &tmpInstance );
      tkannFileName = locateLocal("config", "ktalkannouncerc", &tmpInstance);
      endpwent();
message("%s","endpwent");
 
//WABA: New KConfig should be able to handle this gracefully:
     if (stat(QFile::encodeName(tkannFileName),&buf)!=-1) {
          // check if it exists, 'cause otherwise it would be created empty with
          // root as owner !
          ktkanncfg = new KConfig(tkannFileName);
          ktkanncfg -> setGroup("ktalkannounce");
          ktkanncfg -> setDollarExpansion(true);
      } else ktkanncfg = 0L;
      if (stat(QFile::encodeName(cfgFileName),&buf)!=-1) {
          // check if it exists, 'cause otherwise it would be created empty with
          // root as owner !
          ktalkdcfg = new KConfig(cfgFileName);
          ktalkdcfg -> setGroup("ktalkd");
          ktalkdcfg -> setDollarExpansion(true);
          message("User config file ok");
      } else {
          ktalkdcfg = 0L;
          message("No user config file %s !",cfgFileName.ascii());
      }
message("%s","done");
      return ((ktkanncfg != 0L) || (ktalkdcfg != 0L));
      /* Return true if at least one file exists */
  }
}

/*
 * Read one entry in user-config-file
 */

int read_user_config(const char * key, char * result, int max)
{
    KConfig * cfg;
    if (!strncmp(key,"Sound",5))
        // Any key starting with Sound is in ktalkannouncerc
        // talkprg is there too, but we don't care about it here
        cfg = ktkanncfg;
    else
        cfg = ktalkdcfg;

    if (!cfg) return 0; // file doesn't exist
    QString Qresult;
    if ((Qresult = cfg -> readEntry(key, "unset")) != "unset")
    {
        qstrncpy( result, Qresult.ascii(), max);

        if (Options.debug_mode) syslog(LOG_DEBUG,"User option %s : %s", key, result);
        return 1;
    }
    else 
    {
        message("User option %s NOT found", key);
        return 0;
    }
}

int read_bool_user_config(const char * key, int * result)
{
    char msgtmpl[S_CFGLINE];
    int ret = read_user_config(key, msgtmpl, S_CFGLINE); // ret=1 if found
    if (ret!=0) *result = booleanresult(msgtmpl);
    return ret;
}

// Close user-config-file and destroys objects used.

void end_user_config()
{
  if (ktalkdcfg) delete ktalkdcfg;
  if (ktkanncfg) delete ktkanncfg;
  ktalkdcfg = 0L;
  ktkanncfg = 0L;
}

// System configuration file

int process_config_file(void)
{ 
  // Where is ktalkdlg installed ?
  QString ktalkdlg_dir = locate("exe", "ktalkdlg");
  ktalkdlg_dir.truncate( ktalkdlg_dir.findRev('/') );
  // Has to be done, for any $KDEBINDIR in ktalkdrc.
  setenv("KDEBINDIR", QFile::encodeName(ktalkdlg_dir), 0/*don't overwrite*/);

  KConfig * syscfg = new KConfig( "ktalkdrc" );

  syscfg -> setGroup("ktalkd");
  syscfg -> setDollarExpansion(true);

  QString result;
    
#define found(k) (!(result = syscfg -> readEntry(k)).isEmpty())
  //    QString cfgStr = cfgStr0.stripWhiteSpace();
  
  if (found("AnswMach")) {
    Options.answmach=booleanresult(result.ascii()); 
    message("AnswMach : %d",Options.answmach);}
  
  if (found("XAnnounce")) {
    Options.XAnnounce=booleanresult(result.ascii()); 
    message("XAnnounce : %d",Options.XAnnounce); }
  
  if (found("Time")) { 
    Options.time_before_answmach=result.toInt(); 
    message("Time : %d",Options.time_before_answmach); }
  
  if (found("Sound")) { 
    Options.sound=booleanresult(result.ascii());
    message("Sound : %d",Options.sound); }
  
  if (found("SoundFile")) { 
    qstrncpy(Options.soundfile,QFile::encodeName(result),S_CFGLINE);
    message("SoundFile = %s",Options.soundfile); }
  
  if (found("SoundPlayer")) { 
    qstrncpy(Options.soundplayer,QFile::encodeName(result),S_CFGLINE); 
    message("SoundPlayer = %s",Options.soundplayer); }
  
  if (found("SoundPlayerOpt")) { 
    qstrncpy(Options.soundplayeropt,QFile::encodeName(result),S_CFGLINE);
    message("SoundPlayerOpt = %s",Options.soundplayeropt); }
  
  if (found("MailProg")) { 
    qstrncpy(Options.mailprog,QFile::encodeName(result),S_CFGLINE);
    message("Mail prog = %s",Options.mailprog); }
  
  /* text based announcement */
  if (found("Announce1")) { qstrncpy(Options.announce1,result.local8Bit(),S_CFGLINE); }
  if (found("Announce2")) { qstrncpy(Options.announce2,result.local8Bit(),S_CFGLINE); }
  if (found("Announce3")) { qstrncpy(Options.announce3,result.local8Bit(),S_CFGLINE); }

  if (found("NEUUser"))   { 
      qstrncpy(Options.NEU_user,result.local8Bit(),S_CFGLINE); 
      message("NEUUser = %s", Options.NEU_user); 
  }
  if (found("NEUBehaviour")) {
      Options.NEU_behaviour=result.toInt(); 
      message("NEUBehaviour : %d",Options.NEU_behaviour); 
  }
  if (found("NEUForwardMethod"))   { 
      qstrncpy(Options.NEU_forwardmethod,result.ascii(),5); 
      message("NEUForwardMethod = %s", Options.NEU_forwardmethod); 
  }
  
  if (found("ExtPrg")) { 
    qstrncpy(Options.extprg,QFile::encodeName(result),S_CFGLINE);
    message("Ext prg = %s",Options.extprg); }
  else {   /* has to work even without config file at all */
    KStandardDirs stddirs;
    qstrncpy(Options.extprg, QFile::encodeName(stddirs.findResource("exe","ktalkdlg")), S_CFGLINE-1);
  }

  delete syscfg;
  message("End of global configuration");
  return 1;
}


