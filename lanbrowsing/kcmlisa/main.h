/*
 * main.cpp for lisa,reslisa,kio_lan and kio_rlan kcm module
 *
 *  Copyright (C) 2000 Alexander Neundorf <neundorf@kde.org>
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

#ifndef MYMAIN_H
#define MYMAIN_H

#include "kcmlisa.h"
#include "kcmreslisa.h"
#include "kcmkiolan.h"

#include <qtabwidget.h>
#include <kglobal.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kcmodule.h>

class LisaContainer:public KCModule
{
   Q_OBJECT
   public:
      LisaContainer(QWidget *parent=0, const char * name=0);
      virtual ~LisaContainer() {};
      virtual void load();
      virtual void save();
      virtual void defaults() {};
      QString quickHelp() const;

   protected slots:
      void slotEmitChanged();
      void slotLisaEmitChanged();
   private:
      QVBoxLayout layout;
      QTabWidget tabs;
      LisaSettings *lisaPage;
      ResLisaSettings resLisaPage;
      IOSlaveSettings kioLanPage;
      bool lisaPageChanged;
};
#endif

