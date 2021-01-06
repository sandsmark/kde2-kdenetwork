/*
 * portsettingsbar.cpp
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

#include "portsettingsbar.h"
#include <klocale.h>

PortSettingsBar::PortSettingsBar(const QString& title, QWidget *parent)
:QHButtonGroup(title, parent)
{
   m_checkButton=new QRadioButton(i18n("if available"), this);
   m_provideButton=new QRadioButton(i18n("always"), this);
   m_disableButton=new QRadioButton(i18n("never"), this);

   connect(m_checkButton,SIGNAL(clicked()),this,SIGNAL(changed()));
   connect(m_provideButton,SIGNAL(clicked()),this,SIGNAL(changed()));
   connect(m_disableButton,SIGNAL(clicked()),this,SIGNAL(changed()));
};

int PortSettingsBar::selected()
{
   if (m_checkButton->isChecked())
      return PORTSETTINGS_CHECK;
   else if (m_provideButton->isChecked())
      return PORTSETTINGS_PROVIDE;
   return PORTSETTINGS_DISABLE;
};

void PortSettingsBar::setChecked(int what)
{
   if (what==PORTSETTINGS_CHECK)
      m_checkButton->setChecked(TRUE);
   else if (what==PORTSETTINGS_PROVIDE)
      m_provideButton->setChecked(TRUE);
   else 
      m_disableButton->setChecked(TRUE);
};

#include "portsettingsbar.moc"

