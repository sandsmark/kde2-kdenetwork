/*
* typedlg.cpp -- Implementation of class TypeDialog.
* Author:	Sirtaj Singh Kang
* Version:	$Id: typedlg.cpp 136290 2002-02-13 10:30:24Z mlaurent $
* Generated:	Sun May 10 09:01:23 EST 1998
*/


#include <qlistbox.h>
#include <qpushbutton.h>
#include <qstrlist.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <kglobal.h>
#include <klocale.h>

#include "typedlg.h"

//#include "typolayout.h"


TypeDialog::TypeDialog( const QStringList& types, QWidget *parent,
		     const char *name, bool modal )
	: QDialog( parent, name, modal )
{
	setCaption( i18n( "Korn: Select mailbox type..." ) );
	
	QBoxLayout *l = new QVBoxLayout (this, 10);

	QGroupBox *aGroup = new QGroupBox ( i18n( "Mailbox type" ), this );
	l->addWidget( aGroup );

	QGridLayout *layout = new QGridLayout( aGroup , 5, 2, 10 );
	layout->addRowSpacing(0,10);
	layout->addRowSpacing(4,30);
	layout->setRowStretch(1, 0);
	layout->setRowStretch(2, 0);
	layout->setRowStretch(3, 0);
	layout->setRowStretch(4, 1);


	_list = new QListBox( aGroup );
	layout->addMultiCellWidget( _list, 1, 4, 0, 0);
	_list->insertStringList( types );
	_list->setMultiSelection( false );

	connect( _list, SIGNAL(selected(const QString&)), 
			this, SLOT(select(const QString&)) );
	connect( _list, SIGNAL(highlighted(const QString&)), 
			this, SLOT(setType(const QString&)) );

	btOk = new QPushButton( i18n( "O&K" ),  aGroup );
	layout->addWidget( btOk, 1, 1);

	connect( btOk, SIGNAL(clicked()), this, SLOT(accept()) );

	QPushButton *bt = new QPushButton( i18n( "&Cancel" ), aGroup );
	layout->addWidget( bt, 3, 1);

	connect( bt, SIGNAL(clicked()), this, SLOT(reject()) );
	
	btOk->setEnabled(_list->currentItem()!=-1);
}

void TypeDialog::setType( const QString& text )
{
	_type = text;
	btOk->setEnabled(true);
}

void TypeDialog::select( const QString& text )
{
	_type = text;
	accept();
}
#include "typedlg.moc"
