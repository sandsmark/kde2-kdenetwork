/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qspinbox.h>

#include <kapp.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <klocale.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kconfig.h>

#include "page_general.h"

PageGeneral::PageGeneral( QWidget *parent, const char *name ) : PageGeneralBase( parent, name)
{

    modePreview->setPixmap(QPixmap(locate("data","ksirc/pics/sdi.png")));
    wallpaperPathLE->fileDialog()->setFilter( I18N_NOOP( "*.jpg *.png *.gif" ) );
    connect( sdiCB, SIGNAL( toggled( bool ) ), SLOT( setPreviewPixmap( bool ) ) );

    connect( wallpaperPathLE, SIGNAL( textChanged(  const QString& ) ),
					 this, SLOT( showWallpaperPixmap( const QString& ) ) );

    config = kapp->config();
}

PageGeneral::~PageGeneral()
{
  config->sync();
  delete config;
}

void PageGeneral::saveConfig()
{
    config->setGroup( "General" );
    config->writeEntry( "AutoCreateWin", autoCreateWindowCB->isChecked()  );
    config->writeEntry( "NickCompletion", nickCompletionCB->isChecked() );
    config->writeEntry( "DisplayTopic", displayTopicCB->isChecked() );
    config->writeEntry( "TimeStamp", timeStampCB->isChecked() );
    config->writeEntry( "BeepNotify", beepCB->isChecked() );
    config->writeEntry( "ColourPicker", colorPickerPopupCB->isChecked() );
    config->writeEntry( "AutoRejoin", autoRejoinCB->isChecked() );

    config->writeEntry( "WindowLength", historySB->value() );

    config->writeEntry( "BackgroundFile", wallpaperPathLE->url() );

    if ( mdiCB->isChecked() ) config->writeEntry( "DisplayMode", "1" );
    if ( sdiCB->isChecked() ) config->writeEntry( "DisplayMode", "0" );

    config->sync();

}

void PageGeneral::readConfig()
{
    config->setGroup( "General" );
    autoCreateWindowCB->setChecked( config->readBoolEntry( "AutoCreateWin", true ) );
    nickCompletionCB->setChecked( config->readBoolEntry( "NickCompletion", true ) );
    displayTopicCB->setChecked( config->readBoolEntry( "DisplayTopic", true ) );
    timeStampCB->setChecked( config->readBoolEntry( "TimeStamp", true ) );
    beepCB->setChecked( config->readBoolEntry( "BeepNotify", true ) );
    colorPickerPopupCB->setChecked( config->readBoolEntry( "ColourPicker", true ) );
    autoRejoinCB->setChecked( config->readBoolEntry( "AutoRejoin", true ) );

    historySB->setValue( config->readNumEntry( "WindowLength", true ) );

    wallpaperPathLE->setURL( config->readEntry("BackgroundFile") );

    if (config->readNumEntry( "DisplayMode", 1 ) == 1 )
        mdiCB->setChecked( true );
    else
        sdiCB->setChecked( true );

    config->sync();

}

void PageGeneral::defaultConfig()
{
}

void PageGeneral::setPreviewPixmap( bool isSDI )
{
    if (isSDI == true)
        modePreview->setPixmap( QPixmap( locate("data", "ksirc/pics/sdi.png" ) ) );
    else
        modePreview->setPixmap( QPixmap( locate("data", "ksirc/pics/mdi.png" ) ) );
}

void PageGeneral::showWallpaperPixmap( const QString &url )
{
    wallpaperPreview->setPixmap( QPixmap( url ) );
}

void PageGeneral::handleWallpaper()
{

    QString path = config->readEntry("Background", QString::null);

    wallpaperPathLE->setURL( path );

    if ( path != QString::null ) wallpaperPreview->setPixmap( QPixmap( path ) );
}
