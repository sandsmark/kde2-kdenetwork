/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PAGE_GENERAL_H
#define PAGE_GENERAL_H

#include "page_generalbase.h"
#include <kapp.h>
#include <kconfig.h>

class PageGeneral : public PageGeneralBase
{

Q_OBJECT

public:
    PageGeneral( QWidget *parent = 0, const char *name = 0 );
    ~PageGeneral();

  void saveConfig();
  void defaultConfig();
  void readConfig();

public slots:
    void setPreviewPixmap( bool isSDI );
    void handleWallpaper();
    void showWallpaperPixmap( const QString &url );

private:
	KConfig *config;
};

#endif
