/*
    knfoldermanager.cpp

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

#include <qlistview.h>
#include <qdir.h>
#include <qhbox.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kaction.h>
#include <kglobal.h>
#include <kapp.h>
#include <kurl.h>
#include <kdebug.h>
#include <klineedit.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "knfolder.h"
#include "kncollectionviewitem.h"
#include "utilities.h"
#include "knfoldermanager.h"
#include "knarticlemanager.h"
#include "kncleanup.h"
#include "knmemorymanager.h"
#include "knode.h"


KNFolderManager::KNFolderManager(KNListView *v, KNArticleManager *a) : v_iew(v), a_rtManager(a)
{
  f_List.setAutoDelete(true);

  //standard folders
  QString dir(KGlobal::dirs()->saveLocation("appdata","folders/"));
  if (dir==QString::null) {
    KNHelper::displayInternalFileError();
    return;
  }

  KNFolder *f;

  f=new KNFolder(0, i18n("Local Folders"), "root");
  f_List.append(f);
  f->readInfo();

  f=new KNFolder(1, i18n("Drafts"), "drafts", root());
  f_List.append(f);
  f->readInfo();

  f=new KNFolder(2, i18n("Outbox"), "outbox", root());
  f_List.append(f);
  f->readInfo();

  f=new KNFolder(3, i18n("Sent"), "sent", root());
  f_List.append(f);
  f->readInfo();

  l_astId=3;

  //custom folders
  loadCustomFolders();

  showListItems();
  setCurrentFolder(0);
}


KNFolderManager::~KNFolderManager()
{
}


void KNFolderManager::setCurrentFolder(KNFolder *f)
{
  c_urrentFolder=f;
  a_rtManager->setFolder(f);

  kdDebug(5003) << "KNFolderManager::setCurrentFolder() : folder changed" << endl;

  if(f && !f->isRootFolder()) {
    if( loadHeaders(f) )
      a_rtManager->showHdrs();
    else
      KMessageBox::error(knGlobals.topWidget, i18n("Cannot load index-file!"));
  }
}


bool KNFolderManager::loadHeaders(KNFolder *f)
{
  if( !f || f->isRootFolder() )
    return false;

  if (f->isLoaded())
    return true;

  // we want to delete old stuff first => reduce vm fragmentation
  knGlobals.memManager->prepareLoad(f);

  if (f->loadHdrs()) {
    knGlobals.memManager->updateCacheEntry( f );
    return true;
  }

  return false;
}


bool KNFolderManager::unloadHeaders(KNFolder *f, bool force)
{
  if(!f || !f->isLoaded())
    return false;

  if (!force && (c_urrentFolder == f))
    return false;

  if (f->unloadHdrs(force))
    knGlobals.memManager->removeCacheEntry(f);
  else
    return false;

  return true;
}


KNFolder* KNFolderManager::folder(int i)
{
  KNFolder *ret=0;
  for(ret=f_List.first(); ret; ret=f_List.next())
    if(ret->id()==i) break;
  return ret;
}


KNFolder* KNFolderManager::newFolder(KNFolder *p)
{
  if (!p)
    p = root();
  KNFolder *f=new KNFolder(++l_astId, i18n("New folder"), p);
  f_List.append(f);
  createListItem(f);
  return f;
}


bool KNFolderManager::deleteFolder(KNFolder *f)
{
  if(!f || f->isRootFolder() || f->isStandardFolder() || f->lockedArticles()>0)
    return false;

  QList<KNFolder> del;
  del.setAutoDelete(false);
  KNFolder *fol;
  KNCollection *p;

  for(fol=f_List.first(); fol; fol=f_List.next()) {
    p=fol->parent();
    while(p) {
      if(p==f) {
        if(fol->lockedArticles()>0)
          return false;
        del.append(fol);
        break;
      }
      p=p->parent();
    }
  }

  del.append(f);
  for(fol=del.first(); fol; fol=del.next()) {
    if(c_urrentFolder==fol)
      c_urrentFolder=0;

    if (unloadHeaders(fol, true)) {
      fol->deleteFiles();
      f_List.removeRef(fol); // deletes fol
    } else
      return false;
  }

  return true;
}


void KNFolderManager::emptyFolder(KNFolder *f)
{
  if(!f || f->isRootFolder())
    return;
  knGlobals.memManager->removeCacheEntry(f);
  f->deleteAll();
}


bool KNFolderManager::moveFolder(KNFolder *f, KNFolder *p)
{
  if(!f || p==f->parent()) // nothing to be done
    return true;

  // is "p" a child of "f" ?
  KNCollection *p2=p?p->parent():0;
  while(p2) {
    if(p2==f)
      break;
    p2=p2->parent();
  }

  if( (p2 && p2==f) || f==p || f->isStandardFolder() || f->isRootFolder()) //  no way ;-)
    return false;

  // reparent
  f->setParent(p);
  f->saveInfo();

  // recreate list-item
  delete f->listItem();
  showListItems();
  if(c_urrentFolder==f)
    v_iew->setActive(f->listItem(), true);

  return true;
}


int KNFolderManager::unsentForAccount(int accId)
{
  int cnt=0;

  for(KNFolder *f=f_List.first(); f; f=f_List.next()) {
    for(int idx=0; idx < f->length(); idx++) {
      KNLocalArticle *a=f->at(idx);
      if(a->serverId()==accId && a->doPost() && !a->posted())
        cnt++;
    }
  }

  return cnt;
}


void KNFolderManager::compactFolder(KNFolder *f)
{
  if(!f || f->isRootFolder())
    return;

  KNCleanUp cup(knGlobals.cfgManager->cleanup());
  cup.compactFolder(f);
}


void KNFolderManager::compactAll(KNCleanUp *cup)
{
  for(KNFolder *f=f_List.first(); f; f=f_List.next()) {
    if (!f->isRootFolder() && f->lockedArticles()==0)
      cup->appendCollection(f);
  }
}


void KNFolderManager::compactAll()
{
  KNCleanUp *cup = new KNCleanUp(knGlobals.cfgManager->cleanup());

  for(KNFolder *f=f_List.first(); f; f=f_List.next()) {
    if (!f->isRootFolder() && f->lockedArticles()==0)
      cup->appendCollection(f);
  }

  cup->start();

  knGlobals.cfgManager->cleanup()->setLastCompactDate();
  delete cup;
}


void KNFolderManager::importFromMBox(KNFolder *f)
{
  if(!f || f->isRootFolder())
    return;

  f->setNotUnloadable(true);

  if (!f->isLoaded() && !loadHeaders(f)) {
    f->setNotUnloadable(false);
    return;
  }

  KNLoadHelper helper(knGlobals.topWidget);
  KNFile *file = helper.getFile(i18n("Import MBox Folder"));
  KNLocalArticle::List list;
  KNLocalArticle *art;
  QString s;
  int artStart=0, artEnd=0;
  bool done=true;

  if (file) {
    knGlobals.top->setCursorBusy(true);
    knGlobals.top->setStatusMsg(i18n(" Importing articles..."));
    knGlobals.top->secureProcessEvents();

    if (!file->atEnd()) {                // search for the first article...
      s = file->readLine();
      if (s.left(5) == "From ") {
        artStart = file->at();
        done = false;
      } else {
        artStart = file->findString("\n\nFrom ");
        if (artStart != -1) {
          file->at(artStart+1);
          s = file->readLine();
          artStart = file->at();
          done = false;
        }
      }
    }

    knGlobals.top->secureProcessEvents();

    if (!done) {
      while (!file->atEnd()) {
        artEnd = file->findString("\n\nFrom ");

        if (artEnd != -1) {
          file->at(artStart);    // seek the first character of the article
          int size=artEnd-artStart;
          QCString buff(size+10);
          int readBytes=file->readBlock(buff.data(), size);

          if (readBytes != -1) {
            buff.at(readBytes)='\0'; //terminate string
            art = new KNLocalArticle(0);
            art->setEditDisabled(true);
            art->setContent(buff);
            art->parse();
            list.append(art);
          }

          file->at(artEnd+1);
          s = file->readLine();
          artStart = file->at();
        } else {
          if ((int)file->size() > artStart) {
            file->at(artStart);    // seek the first character of the article
            int size=file->size()-artStart;
            QCString buff(size+10);
            int readBytes=file->readBlock(buff.data(), size);

            if (readBytes != -1) {
              buff.at(readBytes)='\0'; //terminate string
              art = new KNLocalArticle(0);
              art->setEditDisabled(true);
              art->setContent(buff);
              art->parse();
              list.append(art);
            }
          }
        }

        if (list.count()%75 == 0)
          knGlobals.top->secureProcessEvents();
      }
    }

    knGlobals.top->setStatusMsg(i18n(" Storing articles..."));
    knGlobals.top->secureProcessEvents();

    if (!list.isEmpty())
      knGlobals.artManager->moveIntoFolder(list, f);

    knGlobals.top->setStatusMsg(QString::null);
    knGlobals.top->setCursorBusy(false);
  }

  f->setNotUnloadable(false);
}


void KNFolderManager::exportToMBox(KNFolder *f)
{
  if(!f || f->isRootFolder())
    return;

  f->setNotUnloadable(true);

  if (!f->isLoaded() && !loadHeaders(f)) {
    f->setNotUnloadable(false);
    return;
  }

  KNSaveHelper helper(f->name()+".mbox", knGlobals.topWidget);
  QFile *file = helper.getFile(i18n("Export Folder"));

  if (file) {
    knGlobals.top->setCursorBusy(true);
    knGlobals.top->setStatusMsg(i18n(" Exporting articles..."));
    knGlobals.top->secureProcessEvents();

    QTextStream ts(file);
    ts.setEncoding(QTextStream::Latin1);
    KNLocalArticle *a;

    for(int idx=0; idx<f->length(); idx++) {
      a=f->at(idx);

      a->setNotUnloadable(true);

      if (a->hasContent() || knGlobals.artManager->loadArticle(a)) {
        ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";
        a->toStream(ts, true);
        ts << "\n";
      }

      a->setNotUnloadable(false);

      if (idx%75 == 0)
        knGlobals.top->secureProcessEvents();
    }

    knGlobals.top->setStatusMsg(QString::null);
    knGlobals.top->setCursorBusy(false);
  }
}


void KNFolderManager::syncFolders()
{
  QString dir(KGlobal::dirs()->saveLocation("appdata","folders/"));
  if (dir==QString::null) {
    KNHelper::displayInternalFileError();
    return;
  }

  //sync
  for(KNFolder *f=f_List.first(); f; f=f_List.next()) {
    if (!f->isRootFolder())
      f->syncIndex();
    f->saveInfo();
  }
}


int KNFolderManager::loadCustomFolders()
{
  int cnt=0;
  QString dir(KGlobal::dirs()->saveLocation("appdata","folders/"));
  KNFolder *f;

  if (dir == QString::null) {
    KNHelper::displayInternalFileError();
    return 0;
  }

  QDir d(dir);
  QStringList entries(d.entryList("custom_*.info"));  // ignore info files of standard folders
  for(QStringList::Iterator it=entries.begin(); it != entries.end(); it++) {
    f=new KNFolder();
    if(f->readInfo(d.absFilePath(*it))) {
      if(f->id()>l_astId)
        l_astId=f->id();
      f_List.append(f);
      cnt++;
    }
    else
      delete f;
  }

  // set parents
  if(cnt>0) {
    QList<KNFolder> l(f_List);
    l.setAutoDelete(false);
    for(f=l.first(); f; f=l.next()) {
      if (!f->isRootFolder()) {   // the root folder has no parent
        KNFolder *par = folder(f->parentId());
        if (!par)
          par = root();
        f->setParent(par);
      }
    }
  }

  return cnt;
}


void KNFolderManager::showListItems()
{
  for(KNFolder *f=f_List.first(); f; f=f_List.next())
    if(!f->listItem()) createListItem(f);
  // now open the folders if they were open in the last session
  for(KNFolder *f=f_List.first(); f; f=f_List.next())
    if (f->listItem()) f->listItem()->setOpen(f->wasOpen());
}


void KNFolderManager::createListItem(KNFolder *f)
{
  KNCollectionViewItem *it;
  if(f->parent()==0) {
    it=new KNCollectionViewItem(v_iew);
  }
  else {
    if(!f->parent()->listItem())
      createListItem(static_cast<KNFolder*>(f->parent()));
    it=new KNCollectionViewItem(f->parent()->listItem());
  }
  f->setListItem(it);
  QPixmap pix;
  if (f->isRootFolder())
    pix = knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::rootFolder);
  else
    if (f->isStandardFolder())
      pix = knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::customFolder);
    else
      pix = knGlobals.cfgManager->appearance()->icon(KNConfig::Appearance::folder);
  it->setPixmap(0, pix);
  f->updateListItem();
}

