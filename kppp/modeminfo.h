/*
 *
 *            kPPP: A front end for pppd for the KDE project
 *
 * $Id: modeminfo.h 27659 1999-08-15 17:39:35Z porten $
 * 
 * Copyright (C) 1997 Bernd Johannes Wuebben 
 * wuebben@math.cornell.edu
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef _MODEMINFO_H_
#define _MODEMINFO_H_

#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qevent.h>
#include <qtimer.h>
#include <kprogress.h>

const int NUM_OF_ATI = 8;


class ModemTransfer : public QDialog {
  Q_OBJECT
public:
  ModemTransfer(QWidget *parent=0, const char *name=0);
  
public slots:
  void init();
  void readtty();
  void do_script();
  void time_out_slot();
  void cancelbutton();
  void readChar(unsigned char);

private:
  void ati_done();

protected:
  void closeEvent(QCloseEvent *e);

private:
  int 	step;
  QString readbuffer;

  QPushButton *cancel;
  KProgress *progressBar;
  QLabel *statusBar;

  QTimer *timeout_timer;
  QTimer *scripttimer;
  QString ati_query_strings[NUM_OF_ATI];
};


class ModemInfo : public QDialog {
  Q_OBJECT
public:
  ModemInfo(QWidget *parent=0, const char *name=0);

public:
  void setAtiString(int num, QString s);

private:
  QLabel *ati_label[NUM_OF_ATI];
  QLineEdit *ati_label_result[NUM_OF_ATI];
};

#endif
