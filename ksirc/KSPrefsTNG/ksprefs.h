/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSPREFS_H
#define KSPREFS_H

#include "page_colors.h"
#include "page_general.h"
#include "page_rmbmenu.h"
#include "page_servchan.h"
#include "page_startup.h"

#include <kdialogbase.h>
#include <kconfig.h>


class KSPrefs : public KDialogBase
{

Q_OBJECT
public:
    KSPrefs(QWidget * parent = 0, const char * name = 0);
    ~KSPrefs();

public slots:
    void closeDialog();
    void saveConfig();
    void defaultConfig();
    void readConfig();

signals:
    void update();

private:
    PageColors   *pageColors;
    PageGeneral  *pageGeneral;
    PageRMBMenu  *pageRMBMenu;
    PageStartup  *pageStartup;
    PageServChan *pageServChan;

//  KConfig *config;
//  QTabDialog *pTab;
};

#endif
