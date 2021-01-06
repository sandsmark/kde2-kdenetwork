/*
 * kcmkiolan.cpp
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

#include "kcmkiolan.h"

#include <qlabel.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kdebug.h>

IOSlaveSettings::IOSlaveSettings(const QString& config, QWidget *parent)
:QVBox(parent)
,m_config(config,false,true)
{
/*   QLabel *desc=new QLabel(i18n("\nIf you select \"Check port\" the ioslave will check whether the host supports this service when\n"\
                                "you open this host. Please note that paranoia people might consider even this an attack.\n"\
                                "\"Provide without checking\" means that you will always see the links for the services no matter whether\n"\
                                "they are actually offered by the host. \"Never provide\" means that you will never have the links to the\n"\
                                "services. In these both cases you won't contact the host, so that nobody might ever consider you an attacker.\n"),this);*/
   //m_fingerSettings=new PortSettingsBar(i18n("Finger service (port 79)"),this);
   m_ftpSettings=new PortSettingsBar(i18n("Provide FTP service links (TCP, port 21)"),this);
   m_httpSettings=new PortSettingsBar(i18n("Provide HTTP service links (TCP, port 80)"),this);
   m_nfsSettings=new PortSettingsBar(i18n("Provide NFS service links (TCP, port 2049)"),this);
   m_smbSettings=new PortSettingsBar(i18n("Provide SMB service links (TCP, port 139)"),this);
   m_shortHostnames = new QCheckBox(i18n("Show only the hostname (without the domain suffix)"),this);

   QWidget *w=new QWidget(this);
   setMargin(10);
   setSpacing(15);
//   setStretchFactor(desc,0);
   //setStretchFactor(m_fingerSettings,0);
   setStretchFactor(m_ftpSettings,0);
   setStretchFactor(m_httpSettings,0);
   setStretchFactor(m_nfsSettings,0);
   setStretchFactor(m_smbSettings,0);
   setStretchFactor(m_shortHostnames,0);
   setStretchFactor(w,1);

   connect(m_ftpSettings,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_httpSettings,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_nfsSettings,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_smbSettings,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_shortHostnames,SIGNAL(clicked()),this,SIGNAL(changed()));
};

void IOSlaveSettings::load()
{
   kdDebug()<<"IOSlaveSettings::load()"<<endl;
   //m_fingerSettings->setChecked(m_config.readNumEntry("Support_Finger", PORTSETTINGS_CHECK));
   m_ftpSettings->setChecked(m_config.readNumEntry("Support_FTP", PORTSETTINGS_CHECK));
   m_httpSettings->setChecked(m_config.readNumEntry("Support_HTTP", PORTSETTINGS_CHECK));
   m_nfsSettings->setChecked(m_config.readNumEntry("Support_NFS", PORTSETTINGS_CHECK));
   m_smbSettings->setChecked(m_config.readNumEntry("Support_SMB", PORTSETTINGS_CHECK));
	m_shortHostnames->setChecked(m_config.readBoolEntry("ShowShortHostnames",false));

};

void IOSlaveSettings::save()
{
   kdDebug()<<"IOSlaveSettings::save()"<<endl;
   //m_config.writeEntry("Support_Finger", m_fingerSettings->selected());
   m_config.writeEntry("AlreadyConfigured",true);
   m_config.writeEntry("Support_FTP", m_ftpSettings->selected());
   m_config.writeEntry("Support_HTTP", m_httpSettings->selected());
   m_config.writeEntry("Support_NFS", m_nfsSettings->selected());
   m_config.writeEntry("Support_SMB", m_smbSettings->selected());
   m_config.writeEntry("ShowShortHostnames",m_shortHostnames->isChecked());

   m_config.sync();
};

#include "kcmkiolan.moc"

