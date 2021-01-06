/* -------------------------------------------------------------

   toplevel.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>
   (C) by Matthias H�lzer 1998

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   TopLevel   The toplevel widget of Kdict.

 ------------------------------------------------------------- */

#include <qcolor.h>
#include <qregexp.h>
#include <qclipboard.h>
#include <qsplitter.h>
#include <qpushbutton.h>

#include <kaction.h>
#include <kstdaction.h>
#include <kapp.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kwin.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <dcopclient.h>

#include "actions.h"
#include "dict.h"
#include "options.h"
#include "queryview.h"
#include "matchview.h"
#include "sets.h"
#include "toplevel.h"


// cut a QString and add "..."
QString getShortString(QString str,unsigned int length)
{
  if (str.length()>length) {
    str.truncate(length-3);
    str.append("...");
  }
  return str;
}


DictInterface *interface;
GlobalData *global;


TopLevel::TopLevel()
: KMainWindow(), DCOPObject("KDictIface"), optDlg(0L), setsDlg(0L), stopRef(0)
{
  kapp->dcopClient()->setDefaultObject(objId());
  kapp->setMainWidget(this);
  setCaption(i18n("The KDE Dict(ionary) Client"));

  global = new GlobalData();
  global->topLevel = this;
  global->read();

  interface = new DictInterface();
  connect(interface,SIGNAL(infoReady()),SLOT(stratDbChanged()));
  connect(interface,SIGNAL(started(const QString&)),SLOT(clientStarted(const QString&)));
  connect(interface,SIGNAL(stopped(const QString&)),SLOT(clientStopped(const QString&)));

  queryView = new QueryView(this);
  connect(queryView,SIGNAL(defineRequested(const QString&)),SLOT(define(const QString&)));
  connect(queryView,SIGNAL(matchRequested(const QString&)),SLOT(match(const QString&)));
  connect(queryView,SIGNAL(clipboardRequested()),SLOT(defineClipboard()));
  connect(queryView,SIGNAL(enableCopy(bool)),SLOT(enableCopy(bool)));
  connect(queryView,SIGNAL(enablePrintSave()),SLOT(enablePrintSave()));
  connect(queryView,SIGNAL(renderingStarted()),SLOT(renderingStarted()));
  connect(queryView,SIGNAL(renderingStopped()),SLOT(renderingStopped()));
  connect(queryView,SIGNAL(newCaption(const QString&)),SLOT(newCaption(const QString&)));

  matchView = new MatchView();
  connect(matchView,SIGNAL(defineRequested(const QString&)),SLOT(define(const QString&)));
  connect(matchView,SIGNAL(matchRequested(const QString&)),SLOT(match(const QString&)));
  connect(matchView,SIGNAL(clipboardRequested()),SLOT(matchClipboard()));
  connect(matchView,SIGNAL(windowClosed()),SLOT(toggleMatchListShow()));

  connect(&resetStatusbarTimer,SIGNAL(timeout()),SLOT(resetStatusbar()));

  setupStatusBar();
  setupActions();
  recreateGUI();
  buildHistMenu();

  if (global->showMatchList) {    // show splitter, html view & match list
    if (global->swallowMatchList) {
      splitter = new QSplitter(QSplitter::Horizontal,this);
      splitter->setOpaqueResize(true);
      queryView->reparent(splitter,0,queryView->pos(),true);
      matchView->reparent(splitter,0,matchView->pos(),true);
      setCentralWidget(splitter);
      splitter->setResizeMode(matchView,QSplitter::KeepSize);
      adjustMatchViewSize();
    } else {
      setCentralWidget(queryView);
      matchView->show();
      adjustMatchViewSize();
    }
  } else {                       // show only html view
      setCentralWidget(queryView);
      matchView->hide();
  }

  //apply settings
  resize(600,390);
  applyMainWindowSettings(KGlobal::config(),"toplevel_options");
  actToggleToolbar->setChecked(!toolBar()->isHidden());
  actToggleStatusbar->setChecked(!statusBar()->isHidden());

  stratDbChanged();              // fill combos, build menus

  actQueryCombo->setFocus();        // place cursor in combobox
}


TopLevel::~TopLevel()
{
}


void TopLevel::normalStartup()
{
  if (global->defineClipboard)
    defineClipboard();
}

// ******* DCOP Interface ********************************************************

void TopLevel::quit()
{
  kdDebug(5004) << "*DCOP call* TopLevel::quit()" << endl;
  kapp->closeAllWindows();
}


void TopLevel::makeActiveWindow()
{
  kdDebug(5004) << "*DCOP call* TopLevel::makeActiveWindow()" << endl;
  KWin::setOnDesktop(winId(),KWin::currentDesktop());
  KWin::setActiveWindow(winId());
}


void TopLevel::definePhrase(QString phrase)
{
  kdDebug(5004) << "*DCOP call* TopLevel::definePhrase()" << endl;
  define(phrase);
  KWin::setOnDesktop(winId(),KWin::currentDesktop());
  KWin::setActiveWindow(winId());
}


void TopLevel::matchPhrase(QString phrase)
{
  kdDebug(5004) << "*DCOP call* TopLevel::matchPhrase()" << endl;
  match(phrase);
  KWin::setOnDesktop(winId(),KWin::currentDesktop());
  KWin::setActiveWindow(winId());
}


void TopLevel::defineClipboardContent()
{
  kdDebug(5004) << "*DCOP call* TopLevel::defineClipboardContent()" << endl;
  defineClipboard();
  KWin::setOnDesktop(winId(),KWin::currentDesktop());
  KWin::setActiveWindow(winId());
}


void TopLevel::matchClipboardContent()
{
  kdDebug(5004) << "*DCOP call* TopLevel::matchClipboardContent()" << endl;
  matchClipboard();
  KWin::setOnDesktop(winId(),KWin::currentDesktop());
  KWin::setActiveWindow(winId());
}


QStringList TopLevel::getDatabases()
{
  kdDebug(5004) << "*DCOP call* TopLevel::getDatabases()" << endl;
  return global->databases;
}


QString TopLevel::currentDatabase()
{
  kdDebug(5004) << "*DCOP call* TopLevel::currentDatabase()" << endl;
  return global->databases[global->currentDatabase];
}


QStringList TopLevel::getStrategies()
{
  kdDebug(5004) << "*DCOP call* TopLevel::getStrategies()" << endl;
  return global->strategies;
}


QString TopLevel::currentStrategy()
{
  kdDebug(5004) << "*DCOP call* TopLevel::currentStrategy()" << endl;
  return global->strategies[global->currentStrategy];
}


bool TopLevel::setDatabase(QString db)
{
  kdDebug(5004) << "*DCOP call* TopLevel::setDatabase()" << endl;

  int newCurrent = global->databases.findIndex(db);
  if (newCurrent == -1)
    return false;
  else {
    global->currentDatabase = newCurrent;
    actDbCombo->setCurrentItem(global->currentDatabase);
    return true;
  }
}


bool TopLevel::setStrategy(QString strategy)
{
  kdDebug(5004) << "*DCOP call* TopLevel::setStrategy()" << endl;

  return matchView->selectStrategy(strategy);
}


bool TopLevel::historyGoBack()
{
  kdDebug(5004) << "*DCOP call* TopLevel::historyGoBack()" << endl;

  if (!queryView->browseBackPossible())
    return false;
  else {
    queryView->browseBack();
    return true;
  }
}


bool TopLevel::historyGoForward()
{
  kdDebug(5004) << "*DCOP call* TopLevel::historyGoForward()" << endl;

  if (!queryView->browseForwardPossible())
    return false;
  else {
    queryView->browseForward();
    return true;
  }
}

// *******************************************************************************

void TopLevel::define(const QString &query)
{
  kdDebug(5004) << "TopLevel::define()" << endl;
  actQueryCombo->setEditText(query);
  doDefine();
}


void TopLevel::defineClipboard()
{
  kdDebug(5004) << "TopLevel::defineClipboard()" << endl;
  define(kapp->clipboard()->text());
}


void TopLevel::match(const QString &query)
{
  kdDebug(5004) << "TopLevel::match()" << endl;
  actQueryCombo->setEditText(query);
  doMatch();
}


void TopLevel::matchClipboard()
{
  kdDebug(5004) << "TopLevel::matchClipboard()" << endl;
  match(kapp->clipboard()->text());
}


bool TopLevel::queryClose()
{
  kdDebug(5004) << "TopLevel::queryClose()" << endl;

  saveMainWindowSettings(KGlobal::config(),"toplevel_options");

  saveMatchViewSize();
  global->write();

  return true;
}


void TopLevel::setupActions()
{
  // file menu...
  actSave = KStdAction::save(queryView, SLOT(saveQuery()), actionCollection());
  actSave->setText("&Save...");
  actSave->setEnabled(false);
  actPrint = KStdAction::print(queryView, SLOT(printQuery()), actionCollection());
  actPrint->setEnabled(false);
  actStartQuery = new KAction(i18n("St&art Query"),"redo", 0 , this,
                  SLOT(doDefine()), actionCollection(), "start_query");
  actStopQuery = new KAction(i18n("St&op Query"),"stop", 0 , this,
                 SLOT(stopClients()), actionCollection(), "stop_query");
  actStopQuery->setEnabled(false);
  KStdAction::quit(kapp, SLOT(closeAllWindows()), actionCollection());

  // edit menu...
  actCopy = KStdAction::copy(queryView, SLOT(copySelection()), actionCollection());
  actCopy->setEnabled(false);
  KStdAction::selectAll(queryView, SLOT(selectAll()), actionCollection());
  new KAction(i18n("&Define Clipboard Content"), "define_clip", 0 , this,
              SLOT(defineClipboard()), actionCollection(), "define_clipboard");
  new KAction(i18n("&Match Clipboard Content"), 0 , this,
              SLOT(matchClipboard()), actionCollection(), "match_clipboard");
  KStdAction::find(queryView, SLOT(showFindDialog()), actionCollection());

  // history menu...
  actBack = new KToolBarPopupAction(i18n("&Back"), "back", KStdAccel::key(KStdAccel::Back),
                                    queryView, SLOT(browseBack()), actionCollection(),"browse_back");
  actBack->setDelayed(true);
  actBack->setStickyMenu(false);
  actBack->setEnabled(false);
  actForward = new KToolBarPopupAction(i18n("&Forward"), "forward", KStdAccel::key(KStdAccel::Forward),
                                       queryView, SLOT(browseForward()), actionCollection(),"browse_forward");
  actForward->setDelayed(true);
  actForward->setStickyMenu(false);
  actForward->setEnabled(false);
  new KAction(i18n("&Clear History"), 0 , this,
              SLOT(clearQueryHistory()), actionCollection(), "clear_history");

  // server menu...
  new KAction(i18n("&Get Capabilities"), 0 , interface,
              SLOT(updateServer()), actionCollection(), "get_capabilities");
  new KAction(i18n("Edit &Database Sets..."), "edit", 0 , this,
              SLOT(showSetsDialog()), actionCollection(), "edit_sets");
  new KAction(i18n("&Summary"), 0 , interface,
              SLOT(showDatabases()), actionCollection(), "db_summary");
  new KAction(i18n("S&trategy Information"), 0 , interface,
              SLOT(showStrategies()), actionCollection(), "strategy_info");
  new KAction(i18n("&Server Information"), 0 , interface,
              SLOT(showInfo()), actionCollection(), "server_info");

  // settings menu...
  actToggleToolbar = KStdAction::showToolbar(this, SLOT(toggleToolbar()), actionCollection());
  actToggleStatusbar = KStdAction::showStatusbar(this, SLOT(toggleStatusbar()), actionCollection());
  actShowMatchList = new KToggleAction(i18n("Show &Match List"), 0 , this,
                                 SLOT(toggleMatchListShow()), actionCollection(), "show_match");
  actSwallowMatchList = new KToggleAction(i18n("Swallow Match &List"), 0 , this,
                                    SLOT(toggleMatchListSwallow()), actionCollection(), "swallow_match");
  actShowMatchList->setChecked(global->showMatchList);
  actSwallowMatchList->setChecked(global->swallowMatchList);
  KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());
  KStdAction::preferences(this, SLOT(showOptionsDialog()), actionCollection());

  // toolbar...
  new KAction(i18n("Clear Input Field"), "query_erase", 0 , this,
              SLOT(clearInput()), actionCollection(), "clear_query");

  actQueryLabel = new DictLabelAction(i18n("&Look for"), actionCollection(), "look_label");
  actQueryCombo = new DictComboAction(i18n("Query"), actionCollection(), "query_combo",true,true);
  connect(actQueryCombo,SIGNAL(activated(const QString &)), SLOT(define(const QString&)));
  actDbLabel = new DictLabelAction(i18n("&in"), actionCollection(), "in_label");
  actDbCombo = new DictComboAction(i18n("Databases"), actionCollection(), "db_combo",false,false);
  connect(actDbCombo,SIGNAL(activated(int)),SLOT(databaseSelected(int)));
  actDefineBtn = new DictButtonAction(i18n("&Define"), this, SLOT(doDefine()), actionCollection(), "define_btn");
  actMatchBtn = new DictButtonAction(i18n("&Match"), this, SLOT(doMatch()), actionCollection(), "match_btn");

  queryView->setActions(actBack,actForward,actQueryCombo);
}


void TopLevel::setupStatusBar()
{
  statusBar()->insertItem(i18n(" Ready "),0,2);
  statusBar()->setItemAlignment(0,AlignLeft | AlignVCenter);

  QString serverInfo;
  if (global->authEnabled)
    serverInfo = QString(" %1@%2:%3 ").arg(getShortString(global->user,50))
                                    .arg(getShortString(global->server,50))
                                    .arg(global->port);
  else
    serverInfo = QString(" %1:%3 ").arg(getShortString(global->server,50))
                                 .arg(global->port);
  statusBar()->insertItem(serverInfo, 1,3);
  statusBar()->setItemAlignment(1,AlignLeft | AlignVCenter);
}


void TopLevel::recreateGUI()
{
  kdDebug(5004) << "TopLevel::recreateGUI()" << endl;
  createGUI("kdictui.rc");

  actQueryCombo->setList(global->queryHistory);
  actQueryCombo->clearEdit();
  actQueryLabel->setBuddy(actQueryCombo->widget());

  actDbCombo->setList(global->databases);
  actDbCombo->setCurrentItem(global->currentDatabase);
  actDbLabel->setBuddy(actDbCombo->widget());

  int bwidth;
  if (actDefineBtn->widthHint() > actMatchBtn->widthHint())
    bwidth = actDefineBtn->widthHint();
  else
    bwidth = actMatchBtn->widthHint();
  actDefineBtn->setWidth(bwidth-15);
  actMatchBtn->setWidth(bwidth-15);
}


// add text in the query-combobox to the history
void TopLevel::addCurrentInputToHistory()
{
  QString text(actQueryCombo->currentText());
  
  // maintain queryHistory
  global->queryHistory.remove(text);                     // no double entrys
  global->queryHistory.prepend(text);                   // prepend new item
  while (global->queryHistory.count()>global->maxHistEntrys)    // shorten list
    global->queryHistory.remove(global->queryHistory.fromLast());

  actQueryCombo->setList(global->queryHistory);
  actQueryCombo->setCurrentItem(0);
  buildHistMenu();
}



// erase text in query-combobox
void TopLevel::clearInput()
{
  actQueryCombo->clearEdit();
  actQueryCombo->setFocus();      // place cursor in combobox
}


// define text in the combobox
void TopLevel::doDefine()
{
  QString text(actQueryCombo->currentText());

  if (!text.isEmpty()) {
    addCurrentInputToHistory();
    interface->define(text.utf8());
  }
}


void TopLevel::doMatch()
{
  QString text(actQueryCombo->currentText());

  if (!text.isEmpty()) {
    addCurrentInputToHistory();

    if (!global->showMatchList)
      toggleMatchListShow();
    else
      if (!global->swallowMatchList)
        KWin::setActiveWindow(matchView->winId());
    matchView->match(text);
    setCaption(getShortString(text.simplifyWhiteSpace(),70));
  }
}


void TopLevel::stopClients()
{
  interface->stop();
  queryView->stop();
}


// rebuild history menu on demand
void TopLevel::buildHistMenu()
{
  unplugActionList("history_items");

  historyActionList.setAutoDelete(true);
  historyActionList.clear();

  unsigned int i = 0;
  while ((i<10)&&(i<global->queryHistory.count())) {
    historyActionList.append( new KAction(getShortString(global->queryHistory[i],70), 0, this, SLOT(queryHistMenu()),
                                          0, global->queryHistory[i].utf8().data()) );
    i++;
  }

  plugActionList("history_items", historyActionList);
}


// process a query via the history menu
void TopLevel::queryHistMenu()
{
  QCString name = sender()->name();
  if (!name.isEmpty())
    define(QString::fromUtf8(name));
}


void TopLevel::clearQueryHistory()
{
  global->queryHistory.clear();
  actQueryCombo->clear();
  buildHistMenu();
}


// fill combos, rebuild menus
void TopLevel::stratDbChanged()
{
  actDbCombo->setList(global->databases);
  actDbCombo->setCurrentItem(global->currentDatabase);
  matchView->updateStrategyCombo();

  unplugActionList("db_detail");

  dbActionList.setAutoDelete(true);
  dbActionList.clear();

  for (unsigned int i=0;i<global->serverDatabases.count();i++)
    dbActionList.append( new KAction(global->serverDatabases[i], 0, this, SLOT(dbInfoMenuClicked()),
                                     0, global->serverDatabases[i].utf8().data()) );

  plugActionList("db_detail", dbActionList);
}


void TopLevel::dbInfoMenuClicked()
{
  QCString name = sender()->name();
  if (!name.isEmpty())
    interface->showDbInfo(name);
}


void TopLevel::databaseSelected(int num)
{
  global->currentDatabase = num;
}


void TopLevel::enableCopy(bool selected)
{
  actCopy->setEnabled(selected);
}


void TopLevel::enablePrintSave()
{
  actSave->setEnabled(true);
  actPrint->setEnabled(true);
}


void TopLevel::clientStarted(const QString &message)
{
  statusBar()->changeItem(message,0);
  resetStatusbarTimer.stop();
  stopRef++;
  actStopQuery->setEnabled(stopRef>0);       // enable stop-icon
  kapp->setOverrideCursor(waitCursor);
}


void TopLevel::clientStopped(const QString &message)
{
  statusBar()->changeItem(message,0);
  resetStatusbarTimer.start(4000);
  if (stopRef > 0)
    stopRef--;
  actStopQuery->setEnabled(stopRef>0);      // disable stop-icon
  kapp->restoreOverrideCursor();
}


void TopLevel::resetStatusbar()
{
  resetStatusbarTimer.stop();
  statusBar()->changeItem(i18n(" Ready "),0);
}


void TopLevel::renderingStarted()
{
  stopRef++;
  actStopQuery->setEnabled(stopRef>0);      // disable stop-icon
  kapp->setOverrideCursor(waitCursor);
}


void TopLevel::renderingStopped()
{
  if (stopRef > 0)
    stopRef--;
  actStopQuery->setEnabled(stopRef>0);      // disable stop-icon
  kapp->restoreOverrideCursor();
}


void TopLevel::newCaption(const QString &s)
{
  setCaption(s);
}


void TopLevel::toggleStatusbar()
{
  if (statusBar()->isVisible())
    statusBar()->hide();
  else
    statusBar()->show();
}


void TopLevel::toggleToolbar()
{
  if(toolBar()->isVisible())
    toolBar()->hide();
  else
    toolBar()->show();
}


void TopLevel::toggleMatchListShow()
{
  saveMatchViewSize();
  if (global->showMatchList) {   // list is visible -> hide it
    global->showMatchList = false;
    if (global->swallowMatchList) {
      queryView->reparent(this,0,queryView->pos(),true);
      matchView->reparent(this,0,matchView->pos(),true);
      matchView->hide();
      delete splitter;
      setCentralWidget(queryView);
    } else {
      matchView->hide();
    }
  } else {                      // list is not visible -> show it
    global->showMatchList = true;
    if (global->swallowMatchList) {
      splitter = new QSplitter(QSplitter::Horizontal,this);
      splitter->setOpaqueResize(true);
      setCentralWidget(splitter);
      splitter->show();
      queryView->reparent(splitter,0,queryView->pos(),true);
      matchView->reparent(splitter,0,matchView->pos(),true);
      splitter->setResizeMode(matchView,QSplitter::KeepSize);
      adjustMatchViewSize();
    } else {
      matchView->reparent(0,0,matchView->pos(),true);
      matchView->show();
      adjustMatchViewSize();
    }
  }
  actShowMatchList->setChecked(global->showMatchList);
}


void TopLevel::toggleMatchListSwallow()
{
  saveMatchViewSize();
  if (global->swallowMatchList) {    // list is swallowed -> unswallow
    global->swallowMatchList = false;
    if (global->showMatchList) {
      queryView->reparent(this,0,queryView->pos(),true);
      matchView->reparent(0,0,matchView->pos(),true);
      matchView->show();
      delete splitter;
      setCentralWidget(queryView);
      adjustMatchViewSize();
    }
  } else {                         // list is not swallowed -> swallow
    global->swallowMatchList = true;
    if (global->showMatchList) {
      splitter = new QSplitter(QSplitter::Horizontal,this);
      splitter->setOpaqueResize(true);
      setCentralWidget(splitter);
      splitter->show();
      queryView->reparent(splitter,0,queryView->pos(),true);
      matchView->reparent(splitter,0,matchView->pos(),true);
      splitter->setResizeMode(matchView,QSplitter::KeepSize);
      adjustMatchViewSize();
    }
  }
}


void TopLevel::saveMatchViewSize()
{
  if (global->showMatchList) {
    if (global->swallowMatchList)
      global->splitterSizes = splitter->sizes();
    else
      global->matchSize = matchView->size();
  }
}


void TopLevel::adjustMatchViewSize()
{
  if (global->swallowMatchList) {
    if (global->splitterSizes.count()==2)
      splitter->setSizes(global->splitterSizes);
  } else {
    if (global->matchSize.isValid())
      matchView->resize(global->matchSize);
  }

}


void TopLevel::slotConfKeys()
{
  KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}


void TopLevel::slotConfToolbar()
{
  saveMainWindowSettings(KGlobal::config(),"toplevel_options");
  KEditToolbar *dlg = new KEditToolbar(actionCollection(), "kdictui.rc");

  if (dlg->exec()) {
    recreateGUI();
    applyMainWindowSettings(KGlobal::config(),"toplevel_options");
    buildHistMenu();   // actionlists must be inserted
    stratDbChanged();
  }

  delete dlg;
}


void TopLevel::showSetsDialog()
{
  if (!setsDlg) {
    setsDlg = new DbSetsDialog(this);
    connect(setsDlg,SIGNAL(setsChanged()),this,SLOT(setsChanged()));
    connect(setsDlg,SIGNAL(dialogClosed()),this,SLOT(hideSetsDialog()));
    setsDlg->show();
  } else {
    KWin::setActiveWindow(setsDlg->winId());
  }
}


void TopLevel::hideSetsDialog()
{
  if (setsDlg) {
    disconnect(setsDlg,SIGNAL(setsChanged()),this,SLOT(setsChanged()));
    disconnect(setsDlg,SIGNAL(dialogClosed()),this,SLOT(hideSetsDialog()));
    delete setsDlg;
    setsDlg = 0L;
  }
}


void TopLevel::setsChanged()
{
  actDbCombo->setList(global->databases);
  actDbCombo->setCurrentItem(global->currentDatabase);
}


void TopLevel::showOptionsDialog()
{
  if (!optDlg) {
    optDlg = new OptionsDialog(this);
    connect(optDlg,SIGNAL(optionsChanged()),this,SLOT(optionsChanged()));
    connect(optDlg,SIGNAL(finished()),this,SLOT(hideOptionsDialog()));
    optDlg->show();
  } else {
    KWin::setActiveWindow(optDlg->winId());
  }
}


void TopLevel::hideOptionsDialog()
{
  if (optDlg) {
    optDlg->delayedDestruct();
    optDlg=0;
  }
}


void TopLevel::optionsChanged()
{
  QString serverInfo;
  if (global->authEnabled)
    serverInfo = QString(" %1@%2:%3 ").arg(getShortString(global->user,50))
                                    .arg(getShortString(global->server,50))
                                    .arg(global->port);
  else
    serverInfo = QString(" %1:%3 ").arg(getShortString(global->server,50))
                                    .arg(global->port);
  statusBar()->changeItem(serverInfo,1);
  interface->serverChanged();                  // inform client
  queryView->optionsChanged();              // inform html-view
}


//--------------------------------

#include "toplevel.moc"
