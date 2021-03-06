/*
* shell.cpp -- Implementation of class KornShell.
* Author:	Sirtaj Singh Kang
* Version:	$Id: shell.cpp 93384 2001-04-22 08:26:04Z taj $
* Generated:	Sun May  3 10:30:24 EST 1998
*/

#include<assert.h>
#include<qpopupmenu.h>
#include<qlayout.h>
#include<qcursor.h>

#include<kmessagebox.h>
#include<kglobal.h>
#include<kapp.h>
#include<kconfig.h>
#include<kwin.h>
#include<klocale.h>
#include<kaction.h>
#include<kstdaction.h>
#include<kaboutapplication.h>
#include<kbugreport.h>
#include<kdebug.h>

#include"utils.h"
#include"shell.h"

#include"kornbutt.h"
#include"kornset.h"
#include"dropman.h"

#include"optdlg.h"
#include"dropdlg.h"

KornShell::KornShell( QWidget *parent )
	: QWidget( parent ),
	_configDirty( false ),
	_toWrite( false ),
	_layout( 0 ),
	_optDlg( 0 )
{
        _buttons = new QList<KornButton>;
	_buttons->setAutoDelete( false );
        _menu = initMenu();

        _manager = new KDropManager;

	connect( _manager, SIGNAL(monitorCreated()),
			this, SLOT(configDirty()) );
	connect( _manager, SIGNAL(monitorDeleted()),
			this, SLOT(configDirty()) );
	connect( _manager, SIGNAL(configChanged()),
			this, SLOT(configDirty()) );

        _settings= new KornSettings( kapp->config() );

#if defined(test_headerbutton)
	_headbutton = new HeadButton(this);
#endif
}

KornShell::~KornShell()
{
	if ( _toWrite ) {
		// write config if changed
		_manager->writeConfig( *(kapp->config()), fu("Korn"));
		kapp->config()->sync();
	}
			
	delete _buttons;
	delete _menu;
	delete _manager;
	delete _settings;
	delete _optDlg;
	delete _layout;
#if defined(test_headerbutton)
	delete _headbutton;
#endif
}

bool KornShell::init()
{
	_settings->readConfig();

	bool ok = _manager->readConfig( *(kapp->config()),
			fu("Korn"));

	if( !ok && firstTimeInit() == false ) {
		qWarning( "KornShell: DropManager configuration not valid." );
		return false;
	}

	createButtons( _settings->layout() );

	return true;
}





/* lays out the monitor buttons.
 * On top is a "korn" string,
 * below the buttons in their layout
 */
void KornShell::createButtons( KornSettings::Layout layout )
{
	// create layout manager if buttons are
	// not to be docked

	if( _layout != 0 ) {
	  kdDebug() << "deleting layout" << endl;
		delete _layout;
		_layout = 0;
	}

	_buttons->setAutoDelete( true );
	_buttons->clear();
	
#if 0
 	if( layout == KornSettings::Dock
 			&& !KWM::isKWMInitialized() ) {
 		// No KWM, go to fallback style
 		_settings->setLayout( KornSettings::Horizontal );
		layout = _settings->layout();
 	}
#endif

	switch ( layout ) {
		case KornSettings::Vertical:
		  //kdDebug() << "setting vertical layout" << endl;
			_layout = new QBoxLayout( this,
					QBoxLayout::TopToBottom, 2 );
			break;

		case KornSettings::Horizontal:
		  //kdDebug() << "setting horizontal layout" << endl;
			_layout = new QBoxLayout( this,
					QBoxLayout::LeftToRight, 2 );
			break;

		default:
		  //kdDebug() << "setting no layout" << endl;
			_layout = 0;
			break;
	}

	// create buttons

	QListIterator<KMailDrop> list = _manager->monitors();
	QWidget *parent = ( _layout != 0 ) ? this : 0;
	//kdDebug() << "set parent " << parent << endl;

#if defined(test_headerbutton)
	if ( _layout ) {
	  _layout->addWidget(_headerbutton);
	  size++;
	}
	else {
	  KWin::setType( _headbutton->winId(), NET::Dock );
	  KWin::setSystemTrayWindowFor( _headbutton->winId(), 0 );
	  QApplication::syncX();
	  kapp->setMainWidget( 0 );
	}
	_headbutton->show();
#endif

	_buttons->setAutoDelete( (_layout == 0) );
	int size = 0;

	for( ; list.current(); ++list ) {
		KornButton *butt = new KornButton( parent, list.current() );

		connect( butt, SIGNAL (rightClick()), this, SLOT(popupMenu()));
//		connect( butt, SIGNAL (dying(KornButton *)),
//			this, SLOT(disconnectButton(KornButton *)) );

		_buttons->append( butt );

		if ( _layout ) {
			_layout->addWidget( butt );
			size++;
			kapp->setMainWidget( this );
		}
		else {
			KWin::setType( butt->winId(), NET::Dock );
			KWin::setSystemTrayWindowFor( butt->winId(), 0 );
			QApplication::syncX();
			kapp->setMainWidget( 0 );
		}

		butt->show();
	}

	// set size of top level widget

	if ( size ) {
		int vert;
		int horiz;

		if( layout == KornSettings::Vertical ) {
			vert = size * 25;
			horiz = 25;
		}
		else {
			vert = 25;
			horiz = size * 25;
		}
		resize( horiz, vert );
	}

  if (0 == _buttons->count()) {
    resize(30,30);
  }
}	

void KornShell::show()
{
	// If docked, there's no real toplevel widget
	if( ( _settings->layout() != KornSettings::Dock ) ) {
		QWidget::show();
	}
}

void KornShell::popupMenu()
{
	_menu->popup( QCursor::pos() );
}

void KornShell::optionDlg()
{
	if( _optDlg != 0 ) {
		_optDlg->show();
		return;
	}
	
	_optDlg = new KornOptDlg( _manager, 0 );

	_optDlg->setKornLayout( _settings->layout() );

	connect( _optDlg, SIGNAL(finished()), this, SLOT(dlgClosed()) );

	_optDlg->show();
}

void KornShell::dlgClosed()
{
	if( _optDlg == 0 ) {
		qWarning( "KornShell:: dlgClosed() called without a dialog." );
		return;
	}

	bool needsCreate = false;

	// ok, read and update settings

	if ( _settings->layout() != _optDlg->kornLayout() ) {

		_settings->setLayout( _optDlg->kornLayout() );
		_settings->writeConfig();

		needsCreate = true;
	}

	if ( _configDirty ) {
		needsCreate = true;
		_configDirty = false;
	}

	if ( needsCreate ) {
		// change only if not the same layout
		hide();
		createButtons( _settings->layout() );
		show();
	}

	_optDlg->delayedDestruct();
	_optDlg = 0;
}

void KornShell::help()
{
	kapp->invokeHelp();
}

void KornShell::reportBug()
{
  KBugReport br(this);
  br.show();
}

void KornShell::about()
{
  KAboutApplication about(this);
  about.show();
}

QPopupMenu *KornShell::initMenu()
{
  QPopupMenu *menu = new QPopupMenu;

  KStdAction::preferences(this, SLOT(optionDlg()))->plug(menu);
  menu->insertSeparator();
  KStdAction::help(this, SLOT(help()))->plug(menu);
  KStdAction::reportBug(this, SLOT(reportBug()))->plug(menu);
  KStdAction::aboutApp(this, SLOT(about()))->plug(menu);
  menu->insertSeparator();
  KStdAction::quit(qApp, SLOT(quit()))->plug(menu);

  return menu;
}

bool KornShell::firstTimeInit()
{
	// ask user whether a sample config is wanted
 
  // XXX Do we really need to ask ?

#if 0
	int status = KMessageBox::warningYesNo(0, 
		i18n( "You do not appear to have used KOrn before.\n"
		"Would you like a basic configuration created for you?" ),
		i18n("Welcome to KOrn"),
		i18n("Yes"), i18n( "No, Exit" ));

	if( status != 0 ) {
		return false;
	}
#endif

	// get manager to create default config

	return _manager->createBasicConfig();
}

void KornShell::disconnectButton( KornButton *button )
{
	assert( button != 0 );

	if( _buttons->remove( button )  ) {
		// button was in list

		if( !_buttons->autoDelete() ) {
			delete button;
		}
	}
}

void KornShell::configDirty()
{
	_configDirty = true;
	_toWrite = true;
}

void KornShell::saveSession()
{
	// No Session management saving required... should
	// just restore itself with last config.
}
#include "shell.moc"
