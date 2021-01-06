/*
 * main.cpp for lisa,reslisa,kio_lan and kio_rlan kcm module
 *
 *  Copyright (C) 2000,2001 Alexander Neundorf <neundorf@kde.org>
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

#include "main.h"

#include "kcmlisa.h"
#include "kcmreslisa.h"
#include "kcmkiolan.h"

#include <klocale.h>
#include <qtabwidget.h>
#include <kglobal.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qdir.h>

#include <kcmodule.h>

#include <unistd.h>
#include <sys/types.h>

LisaContainer::LisaContainer(QWidget *parent, const char* name)
:KCModule(parent,name)
,layout(this)
,tabs(this)
,lisaPage(0)
,resLisaPage(QDir::homeDirPath()+"/.reslisarc",&tabs)
,kioLanPage("kio_lanrc", &tabs)
,lisaPageChanged(false)
{
   layout.addWidget(&tabs);
   lisaPage=new LisaSettings("/etc/lisarc",&tabs);
   tabs.addTab(lisaPage,i18n("LISa daemon"));
   connect(lisaPage,SIGNAL(changed()),this,SLOT(slotLisaEmitChanged()));
   tabs.addTab(&resLisaPage,i18n("ResLISa daemon"));
   tabs.addTab(&kioLanPage,i18n("lan:/ and rlan:/ configuration"));
   connect(&resLisaPage,SIGNAL(changed()),this,SLOT(slotEmitChanged()));
   connect(&kioLanPage,SIGNAL(changed()),this,SLOT(slotEmitChanged()));
   setButtons(Apply|Help);
   load();
};

void LisaContainer::slotEmitChanged()
{
   emit changed(true);
};

void LisaContainer::slotLisaEmitChanged()
{
   lisaPageChanged=true;
   emit changed(true);
};

void LisaContainer::load()
{
   lisaPage->load();
   lisaPageChanged=false;
   resLisaPage.load();
   kioLanPage.load();
//   kioRLanPage.load();
}

void LisaContainer::save()
{
   resLisaPage.save();
   kioLanPage.save();
   if (lisaPageChanged)
      lisaPage->save();
   lisaPageChanged=false;
}


QString LisaContainer::quickHelp() const
{
   return i18n("<h1>LAN Browsing</h1>Here you setup your "
		"<b>\"Network Neighbourhood\"</b>, you "
		"can use either the LISa daemon and the lan:/ ioslave or the "
		"ResLISa daemon and the rlan:/ ioslave.<br><br>"
		"About the <b>LAN ioslave</b> configuration:<br> If you select "
		"<i>if available</i> the ioslave will check whether the host "
		"supports this service when you open this host. Please note "
		"that paranoia people might consider even this an attack.<br>"
		"<i>Always</i> means that you will always see the links for the "
		"services no matter whether they are actually offered by the host. "
		"<i>Never</i> means that you will never have the links to the services. "
		"In both cases you won't contact the host, so that nobody might ever "
		"consider you an attacker.<br><br>More information about <b>LISa</b> "
		"can be found at  <a href=\"http://lisa-home.sourceforge.net\">"
		"the LISa Homepage</a> or contact Alexander Neundorf "
		"&lt;<a href=\"mailto:neundorf@kde.org\">neundorf@kde.org</a>&gt;.");
}


extern "C"
{

  KCModule *create_lanbrowser(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmlanbrowser");
    return new LisaContainer(parent, name);
  }

}

#include "main.moc"

