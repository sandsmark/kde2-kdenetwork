/**********************************************************************

 Server Controller

 $$Id: servercontroller.cpp 192453 2002-12-06 16:12:51Z lunakl $$

 Main Server Controller.  Displays server connection window, and makes
 new server connection on demand.

 Signals: NONE

 Slots:

   new_connection(): Creates popup asking for new connection

   new_ksircporcess(QString):
      Args:
         QString: new server name or IP to connect to.
      Action:
	 Creates a new sirc process and window !default connected to the
	 server.  Does nothing if a server connection already exists.

   add_toplevel(QString parent, QString child):
      Args:
	   parent: the server name that the new channel is being joined on
	   child: the new channel name
      Action:
         Adds "child" to the list of joined channles in the main
	 window.  Always call this on new window creation!

   delete_toplevel(QString parent, QString child):
      Args:
         parent: the server name of which channel is closing
	         child: the channle that is closing. IFF Emtpy, parent is
		 deleted.
      Action:
	 Deletes the "child" window from the list of connections.  If
	 the child is Empty the whole tree is removed since it is assumed
         the parent has disconnected and is closing.

   new_channel:  Creates popup asking for new channel name

   new_toplevel(QString str):
      Args:
         str: name of the new channel to be created
      Action:
         Sends a signal to the currently selected server in the tree
         list and join the requested channel.  Does nothing if nothing
         is selected in the tree list.

   recvChangeChanel(QString parent, QString old, QString new):
      Args:
         parent: parent server connection
         old: the old name for the window
         new: the new name for the window
      Action:
          Changes the old window name to the new window name in the tree
          list box.  Call for all name change!

 *********************************************************************/

#include <stdio.h>
#include <unistd.h>

#include "servercontroller.h"
#include "KSOpenkSirc/open_ksirc.h"
#include "NewWindowDialog.h"
#include "KSircColour.h"
#include "config.h"
#include "control_message.h"
#include "FilterRuleEditor.h"
#include "../config.h"
#include "version.h"
#include "KSPrefs/ksprefs.h"
#include "toplevel.h"
#include <iostream>
#include <stdlib.h>

#include "objFinder.h"

#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kfontdialog.h>
#include <kiconloader.h>
#include <kwin.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kstdaccel.h>
#include <kstddirs.h>

#include <qfile.h>
#include <qkeycode.h>

#include "displayMgrSDI.h"
#include "displayMgrMDI.h"

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#include <klocale.h>

extern KConfig *kConfig;
extern KApplication *kApp;
extern global_config *kSircConfig;
DisplayMgr *displayMgr;

servercontroller::servercontroller( QWidget*, const char* name )
    : KMainWindow( 0, name ), kc(0L)
{
  we_are_exiting = false;
  MenuBar = menuBar();
  KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );


  if(kSircConfig->DisplayMode == 0){
  SDI:
    displayMgr = new DisplayMgrSDI();
  }
  else if ( kSircConfig->DisplayMode == 1 )
      displayMgr = new DisplayMgrMDI;
  else{
      kdWarning() << "Game mode Indeterminate, defaulting to SDI" << endl;
      goto SDI;
  }

  sci = new scInside(this, QString(name) + "_mainview");
  setCentralWidget(sci);

  sci->setFrameStyle(QFrame::Box | QFrame::Raised);
  ConnectionTree = sci->ConnectionTree;

  connect(ConnectionTree, SIGNAL(clicked( QListViewItem * )),
	  this, SLOT(WindowSelected(QListViewItem *)));

  setFrameBorderWidth(5);

  KAPopupMenu *file = new KAPopupMenu(0, QString(name) + "_menu_file");
  file->insertItem(i18n("&Quit"), this, SLOT(endksirc()), KStdAccel::quit());
  MenuBar->insertItem(i18n("&File"), file);

  connections = new KAPopupMenu(0, QString(name) + "_menu_connections");

  server_id = connections->insertItem(i18n("&New Server..."), this, SLOT(new_connection()), Key_F2 );
  join_id = connections->insertItem(i18n("&Join Channel..."), this, SLOT(new_channel()), Key_F3 );
  connections->setItemEnabled(join_id, FALSE);
  MenuBar->insertItem(i18n("&Connections"), connections);

  kConfig->setGroup("General");
  kSircConfig->AutoCreateWin = kConfig->readBoolEntry("AutoCreateWin", FALSE);
  kSircConfig->BeepNotify = kConfig->readBoolEntry("BeepNotify", TRUE);
  kSircConfig->NickCompletion = kConfig->readBoolEntry("NickCompletion", TRUE);
  kSircConfig->ColourPicker = kConfig->readBoolEntry("ColourPicker", TRUE);
  kSircConfig->AutoRejoin = kConfig->readBoolEntry("AutoRejoin", TRUE);
  kSircConfig->BackgroundPix = kConfig->readBoolEntry("BackgroundPix", FALSE);
  kSircConfig->BackgroundFile = kConfig->readEntry("BackgroundFile");
  kSircConfig->DisplayTopic = kConfig->readBoolEntry("DisplayTopic", TRUE);

  kConfig->setGroup("GlobalOptions");
  options = new KAPopupMenu(0, QString(name) + "_menu_options");
  //insertChild(options);
  options->setCheckable(TRUE);

  options->insertItem(i18n("&Color Preferences..."),
		      this, SLOT(colour_prefs()), CTRL + Key_P);
  options->insertItem(i18n("&Global Fonts..."),
		      this, SLOT(font_prefs()));
  options->insertItem(i18n("&Filter Rule Editor..."),
		      this, SLOT(filter_rule_editor()));
  options->insertSeparator();
  options->insertItem(i18n("&Preferences..."),
		      this, SLOT(general_prefs()));

  MenuBar->insertItem(i18n("&Options"), options);


  KAPopupMenu *help = new KAPopupMenu(0, QString(name) + "_menu_help");
  //insertChild(help);
  //  help->insertItem("Help...",
  //		   this, SLOT(help_general()));
  help->insertItem(i18n("Help on Colors..."),
		   this, SLOT(help_colours()));
  help->insertItem(i18n("Help on Filters..."),
		   this, SLOT(help_filters()));
  help->insertItem(i18n("Help on Keys..."),
		   this, SLOT(help_keys()));
  help->insertSeparator();
  help->insertItem(i18n("About kSirc..."),
		   this, SLOT(about_ksirc()));
  MenuBar->insertItem(i18n("&Help"), help);

  open_toplevels = 0;

  pic_server = new QPixmap(UserIcon("server"));
  pic_gf = new QPixmap(UserIcon("ksirc_a"));
  pic_run = new QPixmap(UserIcon("mini-run"));
  pic_ppl = new QPixmap(UserIcon("channels"));
  pic_icon = new QPixmap(UserIcon("ksirc_b"));
  pic_dock = new QPixmap(UserIcon("ksirc_dock"));

  setCaption( i18n("Server Control") );
  KWin::setIcons(winId(), *pic_icon, *pic_server);

  if(kSircConfig->DisplayMode == 0)
    resize( 450,200 );

  // Server Controller is done setting up, create Puke interface.

  kSircConfig->pukeSocket = locateLocal("socket", QString("ksirc.%1").arg(getpid()));

  PukeC = new PukeController(kSircConfig->pukeSocket, this, "pukecontroller");
  if(PukeC->running == TRUE){
    connect(PukeC, SIGNAL(PukeMessages(QString, int, QString)),
	    this, SLOT(ProcMessage(QString, int, QString)));
    connect(this, SIGNAL(ServMessage(QString, int, QString)),
	    PukeC, SLOT(ServMessage(QString, int, QString)));
  }
  else{
    kdDebug() << "Puke failed" << endl;
  }

  dockWidget = new dockServerController(this, "servercontroller_dock");
  KWin::setSystemTrayWindowFor( dockWidget->winId(), winId() );
  dockWidget->show();
  MenuBar->show(); // dunno why, but we have to show() the menubar for the mac mode.
  hide();
}


servercontroller::~servercontroller()
{
  delete pic_icon;
  if(PukeC != 0x0){
     delete PukeC;
     PukeC = 0x0;
  }
}

void servercontroller::new_connection()
{
  open_ksirc *w = new open_ksirc(); // Create new ksirc popup
  connect(w, SIGNAL(open_ksircprocess(QString)), // connected ok to process
          this, SLOT(new_ksircprocess(QString))); // start
  w->exec();                                       // show the sucker!
  delete w;
}

void servercontroller::new_ksircprocess(QString str)
{
  if(str.isEmpty())  // nothing entered, nothing done
    return;
  if(proc_list[str])   // if it already exists, quit
    return;

  // Insert new base
  QListViewItem *rootItem = new QListViewItem( ConnectionTree, str );
  rootItem->setPixmap( 0, *pic_server );
  rootItem->setOpen( true );

  // We do no_channel here since proc emits the signal in the
  // constructor, and we can't connect to before then, so we have to
  // do the dirty work here.
  ProcMessage(str, ProcCommand::addTopLevel, QString("no_channel"));

  KSircProcess *proc = new KSircProcess((char *) str.ascii(), 0, (QString(name()) + "_" + str + "_ksp").ascii() ); // Create proc
  //this->insertChild(proc);                           // Add it to out inheritance tree so we can retreive child widgets from it.
  objFinder::insert(proc);
  proc_list.insert(str, proc);                      // Add proc to hash
  connect(proc, SIGNAL(ProcMessage(QString, int, QString)),
	  this, SLOT(ProcMessage(QString, int, QString)));
  connect(this, SIGNAL(ServMessage(QString, int, QString)),
	  proc, SLOT(ServMessage(QString, int, QString)));

  if(!ConnectionTree->currentItem()){   // If nothing's highlighted
    ConnectionTree->setCurrentItem(rootItem);     // highlight it.
  }

  connections->setItemEnabled(join_id, TRUE);
}

void servercontroller::new_channel()
{
  NewWindowDialog w;
  connect(&w, SIGNAL(openTopLevel(QString)), SLOT(new_toplevel(QString)));
  w.exec();
}

void servercontroller::new_toplevel(QString str)
{
  QListViewItem *citem = ConnectionTree->currentItem(); // get item
  if(citem){ // if it exist, ie something is highlighted, continue
    if(proc_list[citem->text(0)]){ // If it's a match with a server, ok
      proc_list[citem->text(0)]->new_toplevel(str);
    }
    // Otherwise, check the parent to see it's perhaps a server.
    else if ( citem->parent() ) {
	if(proc_list[citem->parent()->text(0)]){
	    proc_list[citem->parent()->text(0)]->new_toplevel(str);
	}
    }
  }
}

void servercontroller::ToggleAutoCreate()
{
  kConfig->setGroup("General");
  if(kConfig->readBoolEntry("AutoCreateWin", FALSE) == FALSE){
    kConfig->writeEntry("AutoCreateWin", TRUE);
    kSircConfig->AutoCreateWin = TRUE;
  }
  else{
    kConfig->writeEntry("AutoCreateWin", FALSE);
    kSircConfig->AutoCreateWin = FALSE;
  }
  kConfig->sync();
}

void servercontroller::colour_prefs()
{
  if (!kc)
  {
     kc = new KSircColour();
     connect(kc, SIGNAL(update()),
	  this, SLOT(configChange()));
     connect(kc, SIGNAL(finished()),
	  this, SLOT(colour_prefsClosed()));
  }
  kc->show();
}

void servercontroller::colour_prefsClosed()
{
  if (kc)
  {
     kc->hide();
     kc->delayedDestruct();
     kc = 0;
  }
}

void servercontroller::general_prefs()
{
  KSPrefs *kp = new KSPrefs();
  connect(kp, SIGNAL(update()),
          this, SLOT(configChange()));
  kp->resize(550, 450);
  kp->show();
}

void servercontroller::filter_rule_editor()
{
  FilterRuleEditor *fe = new FilterRuleEditor();
  connect(fe, SIGNAL(destroyed()),
	  this, SLOT(slot_filters_update()));
  fe->show();
}

void servercontroller::font_prefs()
{
    QFont font = kSircConfig->defaultfont;
    if ( KFontDialog::getFont( font ) == KFontDialog::Accepted )
	font_update( font );
}

void servercontroller::font_update(const QFont &font)
{
  kSircConfig->defaultfont = font;
  configChange();
  kConfig->setGroup("GlobalOptions");
  kConfig->writeEntry("MainFont", kSircConfig->defaultfont);
  kConfig->sync();
}

void servercontroller::configChange()
{
  QDictIterator<KSircProcess> it( proc_list );
  while(it.current()){
    it.current()->filters_update();
    it.current()->getWindowList()["!all"]->control_message(REREAD_CONFIG, "");
    ++it;
  }
}

void servercontroller::about_ksirc()
{
  QString caption = PACKAGE;
  caption += "-";
  caption += KSIRC_VERSION;
  caption += i18n("\n\n(c) Copyright 1997,1998,2001 Andrew Stanley-Jones (asj@ksirc.org)\n\nkSirc Irc Client");
  KMessageBox::about(this, caption, i18n("About kSirc"));
}

void servercontroller::help_general()
{
  kApp->invokeHelp(QString::null, "ksirc");
}

void servercontroller::help_colours()
{
  kApp->invokeHelp("sectcolors", "ksirc");
}

void servercontroller::help_filters()
{
  kApp->invokeHelp("filters", "ksirc");
}

void servercontroller::help_keys()
{
  kApp->invokeHelp("keys", "ksirc");
}

void servercontroller::ProcMessage(QString server, int command, QString args)
{
  QListViewItem *serverItem = 0L;
  QListViewItem *item = ConnectionTree->firstChild();
  while ( item ) {
      if ( !item->parent() && item->text(0) == server ) {
	  serverItem = item;
	  break;
      }
      item = item->nextSibling();
  }

  if ( !serverItem ) {
      kdDebug() << "* ProcMessage for non-existant server?!" << endl;
      return;
  }


  switch(command){

#if 0
    // fix this... /notify ... add the "Online" item again?

    // Nick offline and online both remove the nick first.
    // We remove the nick in case of an online so that we don't get
    // duplicates.
    // Args == nick comming on/offline.
  case ProcCommand::nickOffline:
    // Add new channel, first add the parent to the path
    path.push(&server);
    path.push(&online);
    path.push(&args);

    item = serverItem;

    // add a new child item with parent as it's parent
    ConnectionTree->removeItem(&path); // Remove the item
    break;
  case ProcCommand::nickOnline:
    // Add new channel, first add the parent to the path
    path.push(&server);
    path.push(&online);
    path.push(&args);
    // Remove old one if it's there
    ConnectionTree->removeItem(&path); // Remove the item
    path.pop();
    // add a new child item with parent as its parent
    ConnectionTree->addChildItem(args, pic_run, &path);
    if (kSircConfig->BeepNotify) {
      KNotifyClient::beep();
    }
    break;
#endif

    /**
      *  Args:
      *	   parent: the server name that the new channel is being joined on
      *    child:  the new channel name
      *  Action:
      *    Adds "child" to the list of joined channles in the main
      *    window.  Always call this on new window creation!
      */
  case ProcCommand::addTopLevel:
    // Add new channel
    if(args[0] == '!')
      args.remove(0, 1); // If the first char is !, it's control, remove it
    // add a new child item with parent as it's parent
    item = new QListViewItem( serverItem, args );
    item->setPixmap( 0, *pic_ppl );

    open_toplevels++;
    break;
    /**
      *  Args:
      *    parent: the server name of which channel is closing
      *	   child: the channle that is closing. IFF Emtpy, parent is
      *	   deleted.
      *  Action:
      *	   Deletes the "child" window from the list of connections.  If
      *	   the child is Empty the whole tree is removed since it is assumed
      *    the parent has disconnected and is closing.
      */
  case ProcCommand::deleteTopLevel:
    // If the child is emtpy, delete the whole tree, otherwise just the child
    if(args[0] == '!')
      args.remove(0, 1); // If the first char is !, it's control, remove it

    item = findChild( serverItem, args );
    delete item;
    if ( serverItem->childCount() == 0 )
	delete serverItem;

    open_toplevels--;
    break;

  /**
      *  Args:
      *    parent: parent server connection
      *    old: the old name for the window
      *    new: the new name for the window
      *  Action:
      *    Changes the old window name to the new window name in the tree
      *    list box.  Call for all name change!
      */
  case ProcCommand::changeChannel:
    {
      char *new_s, *old_s;
      new_s = new char[args.length()+1];
      old_s = new char[args.length()+1];
      sscanf(args.ascii(), "%s %s", old_s, new_s);
      //  If the channel has a !, it's a control channel, remove the !
      if(old_s[0] == '!')
        // Even though, we want strlen() -1 characters, strlen doesn't
        // include the \0, so we need to copy one more. -1 + 1 = 0.
	memmove(old_s, old_s+1, strlen(old_s));
      if(new_s[0] == '!')
	memmove(new_s, new_s, strlen(new_s)); // See above for strlen()

      item = findChild( serverItem, old_s );
      delete item;
      item = new QListViewItem( serverItem, new_s );
      item->setPixmap( 0, *pic_ppl );

      delete[] new_s;
      delete[] old_s;
    }
    break;
  case ProcCommand::procClose:
    delete serverItem;
    proc_list.remove(server); // Remove process entry while we are at it
    if(proc_list.count() == 0){
      ConnectionTree->clear();
      connections->setItemEnabled(join_id, FALSE);
    }
    break;
  case ProcCommand::turnOffAutoCreate:
    if (kSircConfig->AutoCreateWin) {
      ToggleAutoCreate();
    }
    break;
  case ProcCommand::turnOnAutoCreate:
    if (!kSircConfig->AutoCreateWin) {
      ToggleAutoCreate();
    }
    break;
  default:
    kdDebug() << "Unkown command: [" << command << "] from "
              << server
              << " " << args << endl;
  }
}

void servercontroller::slot_filters_update()
{
  emit ServMessage(QString(), ServCommand::updateFilters, QString());
}

void servercontroller::saveGlobalProperties(KConfig *ksc)
{
  // ksc hos the K Session config
  // ksp == current KSircProcess
  // ksm == current KSircMessageReceiver

  // Ignore all !<name> windows

  QString group = ksc->group();

  ksc->setGroup( "KSircSession" );
  QMap<QString,QStringList>::ConstIterator it = m_sessionConfig.begin();
  for (; it != m_sessionConfig.end(); ++it )
    ksc->writeEntry( it.key(), *it );

  ksc->setGroup("ServerController");
  ksc->writeEntry("Docked", !isVisible());
  ksc->writeEntry("Size", geometry());
  ksc->setGroup(group);
}

void servercontroller::readGlobalProperties(KConfig *ksc)
{
    QString group = ksc->group();

    // ksc == K Session Config

    // KMainWindow silently disables our menubar, when we quit in a docked
    // state, so we have to force showing it here.
    menuBar()->show();

    // commented in for testing...
    ksc->setGroup( "KSircSession" );
    QMap<QString,QString> keyMap = ksc->entryMap( ksc->group() );
    QMap<QString,QString>::Iterator it = keyMap.begin();

    while(it != keyMap.end()) {
	//	debug("%s", it.key().latin1());
	new_ksircprocess(it.key()); // sets up proc_list
	QStringList channels = ksc->readListEntry(it.key());
	for(uint i = 0; i < channels.count(); i++){
	    proc_list[it.key()]->new_toplevel(*(channels.at(i)));
	}
	++it;
    }

    QRect geom;

    ksc->setGroup("ServerController");
    bool docked = ksc->readBoolEntry("Docked", FALSE);
    if ( !docked )
      show();

    geom = ksc->readRectEntry("Size");
    if(! geom.isEmpty())
    	setGeometry(geom);

    ksc->setGroup(group);
}

void servercontroller::saveSessionConfig()
{  
	QDictIterator<KSircProcess> ksp(proc_list);
	while(ksp.current()){
		QStringList channels;
		QDictIterator<KSircMessageReceiver> ksm(ksp.current()->getWindowList());
		while(ksm.current()){
			if(ksm.currentKey()[0] != '!') // Ignore !ksm's (system created)
			channels.append(ksm.currentKey());
			++ksm;
		}
        m_sessionConfig[ ksp.currentKey() ] = channels;
		++ksp;
	}
}

void servercontroller::showEvent( QShowEvent *e )
{
  QWidget::showEvent( e );
  if ( !e->spontaneous() )
    saveDockingStatus();
}

void servercontroller::hideEvent( QHideEvent *e )
{
  QWidget::hideEvent( e );
  if ( !e->spontaneous() )
    saveDockingStatus();
  if(QWidget::isMinimized()){
    hide();
    KWin::setState(winId(), NET::SkipTaskbar);
  }
}

void servercontroller::saveDockingStatus()
{
    if ( we_are_exiting ) // we are hidden by closeEvent
	return;

    KConfigGroupSaver s( kConfig, "ServerController" );
    kConfig->writeEntry("Docked", !isVisible());
    kConfig->sync();
}

void servercontroller::endksirc(){
    kConfig->sync();
    unlink( QFile::encodeName( kSircConfig->pukeSocket ));
    exit(0);
}

void servercontroller::closeEvent( QCloseEvent *e )
{
    we_are_exiting = true;
    saveSessionConfig();
    KMainWindow::closeEvent( e );
}

void servercontroller::WindowSelected(QListViewItem *item)
{
    if ( !item )
	return;

  QListViewItem *parent_server = item->parent();
  if(!parent_server)
    return;

  QString txt = QString(parent_server->text(0)) + "_" + item->text(0) + "_toplevel";
  KSircTopLevel *obj = reinterpret_cast<KSircTopLevel *>( objFinder::find(txt, "KSircTopLevel"));
  if(obj == 0x0){
    txt =QString(parent_server->text(0)) + "_!" + item->text(0) + "_toplevel";
    obj = reinterpret_cast<KSircTopLevel *>( objFinder::find(txt, "KSircTopLevel"));
  }

  if(obj != 0x0){
    displayMgr->raise(obj);
  }
  else {
    kdWarning() << "Did not find widget ptr to raise it" << endl;
  }
}


QListViewItem * servercontroller::findChild( QListViewItem *parent,
					     const QString& text )
{
    if ( !parent || parent->childCount() == 0 ) {
	return 0L;
    }

    QListViewItem *item = parent->firstChild();
    while ( item ) {
	if ( item->text(0) == text ) {
	    return item;
	}
	item = item->nextSibling();
    }

    return 0L;
}

scInside::scInside ( QWidget * parent, const char * name, WFlags
		     f, bool allowLines )
  : QFrame(parent, name, f, allowLines)
{
  ASConn = new QLabel(i18n("Active Server Connections"), this, "servercontroller_label");
  QFont asfont = ASConn->font();
  asfont.setBold(TRUE);
  ASConn->setFont(asfont);

  ConnectionTree = new KListView(this, "connectiontree");
  ConnectionTree->addColumn(QString::null);
  ConnectionTree->setRootIsDecorated( true );
  ConnectionTree->setSorting( 0 );
  ConnectionTree->header()->hide();
}

scInside::~scInside()
{
  delete ASConn;
  delete ConnectionTree;
}

void scInside::resizeEvent ( QResizeEvent *e )
{
  QFrame::resizeEvent(e);
  ASConn->setGeometry(10,10, width() - 20,
		      ASConn->fontMetrics().height()+5);
  ConnectionTree->setGeometry(10, 10 + ASConn->height(),
                              width() - 20, height() - 20 - ASConn->height());
}

dockServerController::dockServerController(servercontroller *_sc, const char *_name)
  : KSystemTray(_sc, _name)
{
  sc = _sc;

  KPopupMenu *pop = contextMenu();
  pop->setName("dockServerController_menu_pop");

  pop->insertItem(i18n("&Color Preferences..."),
                  sc, SLOT(colour_prefs()));
  pop->insertItem(i18n("&Global Fonts..."),
                  sc, SLOT(font_prefs()));
  pop->insertItem(i18n("&Filter Rule Editor..."),
                  sc, SLOT(filter_rule_editor()));
  pop->insertItem(SmallIcon("configure"), i18n("&Preferences..."),
                  sc, SLOT(general_prefs()));
  pop->insertSeparator();
  pop->insertItem(i18n("New Server..."),
                  sc, SLOT(new_connection()));

  if ( _sc->pic_dock )
    setPixmap( *_sc->pic_dock );

}

dockServerController::~dockServerController()
{
  sc = 0x0;
}

#include "servercontroller.moc"
