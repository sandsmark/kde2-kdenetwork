/*
 * kcmreslisa.h
 *
 * Copyright (c) 2000 Alexander Neundorf <neundorf@kde.org>
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

#ifndef KCMRESLISA_H
#define KCMRESLISA_H

#include <krestrictedline.h>
#include <kconfig.h>

#include <keditlistbox.h>

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qpushbutton.h>

class ResLisaSettings:public QVBox
{
   Q_OBJECT
   public:
      ResLisaSettings(const QString& config, QWidget *parent=0);
      virtual ~ResLisaSettings() {};
      void load();
      void save();
   signals:
      void changed();
   protected slots:
      void autoSetup();
   protected:
      KConfig m_config;
      QPushButton *m_autoSetup;
      QCheckBox *m_useNmblookup;
      KEditListBox *m_pingNames;
      KRestrictedLine *m_allowedAddresses;
      //KRestrictedLine *m_broadcastNetwork;
      QSpinBox *m_firstWait;
      QCheckBox *m_secondScan;
      QSpinBox *m_secondWait;
      QSpinBox *m_updatePeriod;
      QCheckBox *m_deliverUnnamedHosts;
      QSpinBox *m_maxPingsAtOnce;
};

#endif

