/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qframe.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qlineedit.h>

#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcolorbutton.h>

#include <kdebug.h>

#include "ksprefs.moc"

KSPrefs::KSPrefs(QWidget * parent, const char * name):
    KDialogBase(KDialogBase::IconList, i18n("Configure KSirc"),
                KDialogBase::Help | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel | KDialogBase::Default,
                KDialogBase::Ok, parent, name)
{

    QFrame *itemGeneral  = addPage( i18n( "General" ), i18n( "General KSirc settings" ), BarIcon( "ksirc", KIcon::SizeMedium ) );
    QFrame *itemStartup  = addPage( i18n( "Startup" ), i18n( "KSirc startup settings" ), BarIcon( "gear", KIcon::SizeMedium ) );
    QFrame *itemColors   = addPage( i18n( "Colors" ), i18n( "KSirc color settings" ), BarIcon( "colors", KIcon::SizeMedium ) );
    QFrame *itemRMBMenu  = addPage( i18n( "User Menu" ), i18n( "User menu configuration" ), BarIcon( "", KIcon::SizeMedium ) );
    QFrame *itemServChan = addPage( i18n( "Server/Channel" ), i18n( "Server/channel configuration" ), BarIcon( "", KIcon::SizeMedium ) );

    QVBoxLayout *generalTopLayout  = new QVBoxLayout( itemGeneral, 0, 6 );
    QVBoxLayout *startupTopLayout  = new QVBoxLayout( itemStartup, 0, 6 );
    QVBoxLayout *colorsTopLayout   = new QVBoxLayout( itemColors, 0, 6 );
    QVBoxLayout *rmbMenuTopLayout  = new QVBoxLayout( itemRMBMenu, 0, 6 );
    QVBoxLayout *servChanTopLayout = new QVBoxLayout( itemServChan, 0, 6 );

    pageGeneral  = new PageGeneral( itemGeneral );
    pageStartup  = new PageStartup( itemStartup );
    pageColors   = new PageColors( itemColors );
    pageRMBMenu  = new PageRMBMenu( itemRMBMenu );
    pageServChan = new PageServChan( itemServChan );

    connect(this, SIGNAL( applyClicked() ), this, SLOT( saveConfig() ) );
    connect(this, SIGNAL( okClicked() ), this, SLOT(closeDialog() ) );
    connect(this, SIGNAL( defaultClicked() ), this, SLOT(defaultConfig() ) );

    generalTopLayout->addWidget( pageGeneral );
    startupTopLayout->addWidget( pageStartup );
    colorsTopLayout->addWidget( pageColors );
    rmbMenuTopLayout->addWidget( pageRMBMenu );
    servChanTopLayout->addWidget( pageServChan );

	//enableButtonSeperator( true );

    readConfig();

}

KSPrefs::~KSPrefs()
{
}

void KSPrefs::closeDialog()
{
    saveConfig();
    this->close();
}

void KSPrefs::readConfig()
{
	// apply by calling readConfig in each page

	pageGeneral->readConfig();
	pageColors->readConfig();
	pageStartup->readConfig();
	pageRMBMenu->readConfig();
	pageServChan->readConfig();

}

void KSPrefs::saveConfig()
{
	// apply by calling saveConfig in each page
	// use setDirty flag for each page and
	// emit update() appropriate

	pageGeneral->saveConfig();
	pageColors->saveConfig();
	pageStartup->saveConfig();
	pageRMBMenu->saveConfig();
	pageServChan->saveConfig();
	emit update();
}


void KSPrefs::defaultConfig()
{
	// apply by calling defaultConfig in each page

	// ### TODO: Only restore current page

	pageGeneral->defaultConfig();
	pageColors->defaultConfig();
	pageStartup->defaultConfig();
	pageRMBMenu->defaultConfig();
	pageServChan->defaultConfig();
}

