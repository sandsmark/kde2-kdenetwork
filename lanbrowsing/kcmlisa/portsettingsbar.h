/*
 * portsettingsbar.h
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

#ifndef PORTSETTINGSBAR_H
#define PORTSETTINGSBAR_H

#include <qradiobutton.h>
#include <qhbuttongroup.h>

#define PORTSETTINGS_CHECK 0
#define PORTSETTINGS_PROVIDE 1
#define PORTSETTINGS_DISABLE 2

class PortSettingsBar:public QHButtonGroup
{
   Q_OBJECT
   public:
      PortSettingsBar(const QString& title, QWidget *parent=0);
      virtual ~PortSettingsBar() {};
      int selected();
      void setChecked(int what);
   signals:
      void changed();
   protected:
      QRadioButton *m_checkButton;
      QRadioButton *m_provideButton;
      QRadioButton *m_disableButton;
};

#endif
