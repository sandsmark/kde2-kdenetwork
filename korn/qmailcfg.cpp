/*
* qmailcfg.cpp -- Implementation of class KQMailCfg.
* Author:	Sirtaj Singh Kang
* Version:	$Id: qmailcfg.cpp 136311 2002-02-13 13:15:52Z mlaurent $
* Generated:	Mon Aug  3 13:21:18 EST 1998
*/

#include "qmailcfg.h"

#include <assert.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <qlabel.h>
#include <qlayout.h>

#include "qmail.h"
//#include "typolayout.h"
#include "kfbrowsebtn.h"

KQMailCfg::KQMailCfg( KQMailDrop *drop )
	: KMonitorCfg( drop ),
	_pathEdit( 0 )
{
}

QString KQMailCfg::name() const
{
	return i18n( "&Maildir" );
}

QWidget *KQMailCfg::makeWidget( QWidget *parent )
{
	KQMailDrop *d = dynamic_cast<KQMailDrop *>(drop());
  assert(0 != d);

	QWidget *dlg = new QWidget( parent );
	QGridLayout *l = new QGridLayout( dlg, 2, 3, 10 );
	l->addRowSpacing(0, 10);

	l->addWidget (new QLabel( i18n( "Maildir Path" ), dlg ), 1, 0);

	_pathEdit = new KURLRequester( d->maildir(), dlg );
	_pathEdit->fileDialog()->setMode(2); //directory
	l->addWidget( _pathEdit, 1, 1);		
	return dlg;
}

void KQMailCfg::updateConfig()
{
	assert( _pathEdit != 0 );

	KQMailDrop *d = dynamic_cast<KQMailDrop *>(drop());
  assert(0 != d);

	d->setMaildir( _pathEdit->lineEdit()->text() );
}
