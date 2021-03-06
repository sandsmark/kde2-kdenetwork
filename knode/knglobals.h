/*
    knglobals.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNGLOBALS_H
#define KNGLOBALS_H

class KNConfigManager;
class KNNetAccess;
class KNProgress;
class KNAccountManager;
class KNGroupManager;
class KNArticleManager;
class KNArticleFactory;
class KNFolderManager;
class QWidget;
class KNFilterManager;
class KNMainWindow;
class KNScoringManager;
class KNMemoryManager;
class KNpgp;
class KNArticleWidget;


// idea: Previously the manager classes were available
//       via KNodeApp. Now they can be accessed directly,
//       this removes many header dependencies.
//       (knode.h isn't include everywhere)
class KNGlobals {

  public:

    QWidget               *topWidget;    // topWidget == top, used for message boxes,
    KNMainWindow          *top;          // no need to include knode.h everywhere
    KNArticleWidget       *artWidget;
    KNConfigManager       *cfgManager;
    KNNetAccess           *netAccess;
    KNProgress            *progressBar;
    KNAccountManager      *accManager;
    KNGroupManager        *grpManager;
    KNArticleManager      *artManager;
    KNArticleFactory      *artFactory;
    KNFolderManager       *folManager;
    KNFilterManager       *filManager;
    KNScoringManager      *scoreManager;
    KNMemoryManager       *memManager;
    KNpgp                 *pgp;
};


extern KNGlobals knGlobals;

#endif
