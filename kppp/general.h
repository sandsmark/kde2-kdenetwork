/*
 *
 *            kPPP: A pppd front end for the KDE project
 *
 * $Id: general.h 91080 2001-04-08 21:23:05Z porten $
 *
 *            Copyright (C) 1997 Bernd Johannes Wuebben
 *                   wuebben@math.cornell.edu
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _GENERAL_H_
#define _GENERAL_H_

#include <qwidget.h>
#include <qpainter.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <kcolorbtn.h>

#include "groupbox.h"

class QSlider;

class ModemTransfer;
class KIntNumInput;

class GeneralWidget : public KGroupBox {
  Q_OBJECT
public:
  GeneralWidget( QWidget *parent=0, const char *name=0 );

private slots:
  void 	pppdtimeoutchanged(int);
  void 	caption_toggled(bool);
  void  iconify_toggled(bool on);
  void 	redial_toggled(bool on);
  void 	xserver_toggled(bool on);
  void  quit_toggled(bool);
  void  docking_toggled(bool on);

private:
  QGroupBox 	*box;

  QLabel        *label1;
  QLabel 	*label3;
  QLabel 	*label4;
  QLabel 	*label5;
  QLabel 	*label6;
  QLabel 	*labeltmp;

  QCheckBox 	*chkbox1;
  QCheckBox 	*chkbox2;
  QCheckBox 	*chkbox3;
  QCheckBox 	*chkbox4;
  QCheckBox 	*chkbox5;
  QCheckBox 	*chkbox6;
  QCheckBox 	*chkbox7;

  QLabel        *versionlabel;
  KIntNumInput 	*pppdtimeout;
};


class ModemWidget : public KGroupBox {
  Q_OBJECT
public:
  ModemWidget( QWidget *parent=0, const char *name=0 );

private slots:
  void 	setmodemdc(int);
  void 	setflowcontrol(int);
  void 	modemtimeoutchanged(int);
  void 	modemlockfilechanged(bool);
  void 	setenter(int);
  void  speed_selection(int);

private:
  QComboBox 	*enter;
  QLabel 	*label1;
  QLabel 	*label2;
  QLabel 	*labeltmp;
  QLabel 	*labelenter;
  QComboBox 	*modemdevice;
  QComboBox 	*flowcontrol;

  QComboBox *baud_c;
  QLabel *baud_label;

  KIntNumInput 	*modemtimeout;
  QCheckBox     *modemlockfile;
};


class ModemWidget2 : public KGroupBox {
  Q_OBJECT
public:
  ModemWidget2( QWidget *parent=0, const char *name=0 );

private slots:
  void  waitfordtchanged(bool);
  void 	busywaitchanged(int);
//  void 	use_cdline_toggled(bool);
  void 	modemcmdsbutton();
  void 	terminal();
  void 	query_modem();
  void  volumeChanged(int);

private:
  ModemTransfer *modemtrans;
  QLabel 	*labeltmp;
  QPushButton 	*modemcmds;
  QPushButton 	*modeminfo_button;
  QPushButton 	*terminal_button;
  QFrame 	*fline;
  QCheckBox     *waitfordt;
  KIntNumInput 	*busywait;
  QCheckBox 	*chkbox1;
  QSlider       *volume;
};

class GraphSetup : public KCheckGroupBox {
  Q_OBJECT
public:
  GraphSetup(QWidget *parent = 0, const char *name = 0);

private slots:
  void enableToggled(bool);
  void colorChanged(const QColor &);

private:
  void save();

  KColorButton *bg_color;
  KColorButton *text_color;
  KColorButton *in_color;
  KColorButton *out_color;

  QLabel *bg_text;
  QLabel *text_text;
  QLabel *in_text;
  QLabel *out_text;
};

class AboutWidget : public KGroupBox {
  Q_OBJECT
public:
  AboutWidget( QWidget *parent=0, const char *name=0 );
};

#endif


