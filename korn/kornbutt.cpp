
#include<assert.h>

#include<qwidget.h>
#include<qstring.h>
#include<qtooltip.h>
#include<qpainter.h>
#include<qcolor.h>
#include<qpalette.h>

#include<kiconloader.h>
#include<kglobal.h>
#include<krun.h>
#include<kdebug.h>
#include<kprocess.h>

#include"kornbutt.h"
#include"maildrop.h"
#include"btnstyle.h"

KornButton::KornButton( QWidget *parent, KMailDrop *box )
	: QToolButton(parent, "kornbutton"),
	_box(box),
	_lastNum(0),
	_style( 0 )
{
	assert(box);

	connect(_box, SIGNAL(changed(int)),       this, SLOT(setNumber(int)));
	connect(_box, SIGNAL(notifyDisconnect()), this, SLOT(disconnectMonitor()));
	connect(_box, SIGNAL(configChanged()),    this, SLOT(monitorUpdated()));

	switch( _box->displayStyle() ){
		case KMailDrop::Colour: 
			_style = new KornBtnColourTextStyle( this, _box );
			break;
		case KMailDrop::Icon: 
			_style = new KornBtnIconStyle( this, _box );
			break;
		case KMailDrop::Plain: 
		default:
			_style = new KornBtnColourTextStyle( this, _box );
			break;
	}
	_style->enable();

	monitorUpdated();

	_box->startMonitor();
}

void KornButton::setNumber(int num)
{
	if (0 == _box)
		return;

	if (_box->count() != _lastNum)
		setText(QString::number(_box->count()));

	bool newMessages = _box->count() > _lastNum;
	_lastNum = num;

	_style->update( _box->count(), newMessages );

	if (newMessages && !_box->newMailCmd().isEmpty())
	{
		KShellProcess * proc = new KShellProcess;
		*proc << _box->newMailCmd();
		proc->start();
	}
}

void KornButton::runCommand(bool onlyIfUnread)
{
  kdDebug() << "KornButton::runCommand(" << (onlyIfUnread ? "true" : "false") << ")" << endl;
  if (0 == _box) {
	  kdDebug() << "KornButton::runCommand: button not connected to a box?"
		  	<< endl;
    return;
  }

  if (onlyIfUnread) {

	  kdDebug( ) << "calling box check" << endl;
    _box->recheck();

    if (0 == _box->count())
      return;
  }

  kdDebug() << "KornButton::runCommand: clickCmd == `" << _box->clickCmd() << "'" << endl;

  if (!_box->clickCmd().isEmpty())
	{
		KShellProcess * proc = new KShellProcess;
		*proc << _box->clickCmd();
		proc->start();
	}
}

void KornButton::disconnectMonitor()
{
	delete _style; _style = 0;
  _box->stopMonitor();
  _box = 0;

  emit dying(this);
}

void KornButton::monitorUpdated()
{
  kdDebug() << "KornButton::monitorUpdated()" << endl;
  kdDebug() << "count = " << _box->count() << endl;

  if (0 == _box)
    return;

  bool newMessages = _box->count() > _lastNum;
  _lastNum = _box->count();

  QToolTip::remove(this);
  QToolTip::add(this, _box->caption());

  delete _style; _style = 0;

  switch( _box->displayStyle() ){
	  case KMailDrop::Colour: 
		  _style = new KornBtnColourTextStyle( this, _box );
		  break;
	  case KMailDrop::Icon: 
		  _style = new KornBtnIconStyle( this, _box );
		  break;
	  case KMailDrop::Plain: 
	  default:
		  _style = new KornBtnColourTextStyle( this, _box );
		  break;
  }
  _style->enable();
  _style->update( _box->count(), newMessages );
}

  void
KornButton::mousePressEvent(QMouseEvent * e)
{
  if (e->button() == RightButton) {
    emit(rightClick());
  }
}
  void
KornButton::mouseReleaseEvent(QMouseEvent * e)
{
  switch (e->button()) {
    case LeftButton:  runCommand( true );	break;
    case MidButton:   runCommand( false );	break;
    default:	break;
  }
}

HeadButton::HeadButton( QWidget *parent )
  :
  QToolButton(parent, "headbutton")
{
  setText("Korn");
}

void
HeadButton::mousePressEvent(QMouseEvent * e)
{
  if (e->button() == RightButton) {
    emit(rightClick());
  }
}


#include "kornbutt.moc"
