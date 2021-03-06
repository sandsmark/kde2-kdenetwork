/***************************************************************************
                          filter_pmail.hxx  -  Pegasus-Mail import
                             -------------------
    begin                : Sat Jan 6 2001
    copyright            : (C) 2001 by Holger Schurig
    email                : holgerschurig@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILTER_PMAIL_HXX
#define FILTER_PMAIL_HXX

#include "filters.hxx"

/**imports Pegasus-Mail 3.xx messages into KMail
  *@author root
  */

class filter_pmail : public filter  {
private:
   QString CAP;
  /** the working directory */
  char dir[1024];
  /**  */
  filterInfo * inf;
  /** which file (of totalFiles) is now in the work? */
  int currentFile;
  /** total number of files that get imported */
  int totalFiles;
  /** Our parent widget */
  QWidget * par;
public:
	filter_pmail();
	~filter_pmail();
public:
  void import(filterInfo *info);	
private: // Private methods
  /** updates currentFile and the overall progress bar */
  void nextFile();
  /** this looks for all files with the filemask 'mask' and calls the 'workFunc' on each of them */
  void processFiles(const char *mask,  void(filter_pmail::* workFunc)(const char*) );
  /** counts all files with mask (e.g. '*.cnm') in in a directory */
  int countFiles(const char *mask);
  /** this function imports one *.CNM message */
  void importNewMessage(const char *file);
  /** this function imports one mail folder file (*.PMM) */
  void importMailFolder(const char *file);
  /** imports a 'unix' format mail folder (*.MBX) */
  void importUnixMailFolder(const char *file);
};

#endif
