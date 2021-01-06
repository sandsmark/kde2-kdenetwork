/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PAGE_COLORS_H
#define PAGE_COLORS_H

#include "page_colorsbase.h"

#include <kconfig.h>

class PageColors : public PageColorsBase
{

Q_OBJECT

public:
	PageColors( QWidget *parent = 0, const char *name = 0 );
	~PageColors();

  void saveConfig();
  void defaultConfig();
  void readConfig();

private:
     KConfig *config;

};

#endif
