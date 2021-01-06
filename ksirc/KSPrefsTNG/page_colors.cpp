/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "page_colors.h"

#include <qcheckbox.h>

#include <kapp.h>
#include <kcolorbutton.h>

#include <kdebug.h>

PageColors::PageColors( QWidget *parent, const char *name ) : PageColorsBase( parent, name)
{
    config = kapp->config();
}

PageColors::~PageColors()
{
    config->sync();
    delete config;
}

void PageColors::saveConfig()
{
}

void PageColors::readConfig()
{
   config->setGroup( "Colours" );
   backCBtn->setColor(config->readColorEntry("Background"));
   errorCBtn->setColor(config->readColorEntry("error"));
   infoCBtn->setColor(config->readColorEntry("info"));
   genericTextCBtn->setColor(config->readColorEntry("text"));
   ownNickCBtn->setColor(config->readColorEntry("uscolor"));
   nickFGCBtn->setColor(config->readColorEntry("nickfcolour"));
   nickBGCBtn->setColor(config->readColorEntry("nickbcolour"));
   nickFGColorCB->setEnabled(config->readBoolEntry("usebcolour"));

   allowKSircColorsCB->setChecked(config->readBoolEntry("kcolour", true));
   allowMIRCColorsCB->setChecked(config->readBoolEntry("mcolour", true));

   config->sync();
}

void PageColors::defaultConfig()
{
}
