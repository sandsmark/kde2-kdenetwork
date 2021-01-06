/**********************************************************************

	--- Qt Architect generated file ---

	File: UserMenuRefData.cpp
	Last generated: Mon Jan 19 21:30:48 1998

	DO NOT EDIT!!!  This file will be automatically
	regenerated by qtarch.  All changes will be lost.

 *********************************************************************/

#include "UserMenuRefData.h"
#include "../config.h"

#undef Inherited
#define Inherited QFrame

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <klocale.h>

UserMenuRefData::UserMenuRefData
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, 241664 )
{
        QGridLayout *l = new QGridLayout(this, 1, 1, 10);

	MainListBox = new QListBox( this, "ListBox_1" );
	connect( MainListBox, SIGNAL(highlighted(int)), SLOT(newHighlight(int)) );
	connect( MainListBox, SIGNAL(selected(int)), SLOT(newHighlight(int)) );
	MainListBox->setMultiSelection( FALSE );
	l->addMultiCellWidget(MainListBox, 0, 6, 0, 0);

	QLabel* dlgedit_Label_1;
	dlgedit_Label_1 = new QLabel( i18n("Menu Name"),
						this, "Label_1" );
	l->addWidget(dlgedit_Label_1, 0, 1);

	QLabel* dlgedit_Label_2;
	dlgedit_Label_2 = new QLabel( i18n("Type"), 
						this, "Label_2" );
	l->addWidget(dlgedit_Label_2, 1, 1);

	QLabel* dlgedit_Label_3;
	dlgedit_Label_3 = new QLabel( i18n("Command"), 
						this, "Label_3" );
	l->addWidget(dlgedit_Label_3, 2, 1);

	MenuName = new QLineEdit( "", this, "LineEdit_1" );
	MenuName->setEchoMode( QLineEdit::Normal );
	MenuName->setFrame( TRUE );
	l->addWidget(MenuName, 0, 2);

	MenuType = new QComboBox( FALSE, this, "ComboBox_1" );
	connect( MenuType, SIGNAL(highlighted(int)), SLOT(typeSetActive(int)) );
	MenuType->setSizeLimit( 2 );
	MenuType->setAutoResize( FALSE );
	MenuType->insertItem( i18n("Separator") );
	MenuType->insertItem( i18n("Action") );
	l->addWidget(MenuType, 1, 2);

	MenuCommand = new QLineEdit("", this, "LineEdit_2" );
	MenuCommand->setEchoMode( QLineEdit::Normal );
	MenuCommand->setFrame( TRUE );
	l->addWidget(MenuCommand, 2, 2);


	MenuOpOnly = new QCheckBox( i18n("OP Only"), this, "CheckBox_1" );
	MenuOpOnly->setAutoRepeat( FALSE );
	l->addWidget(MenuOpOnly, 3, 2);

	insertButton = new QPushButton( i18n("Insert"), this, "PushButton_2" );
	connect( insertButton, SIGNAL(clicked()), SLOT(insertMenu()) );
	insertButton->setAutoRepeat( FALSE );
	l->addWidget(insertButton, 4, 2);

	QPushButton* dlgedit_PushButton_4;
	dlgedit_PushButton_4 = new QPushButton(i18n("Modify"), this, "PushButton_4" );
	connect( dlgedit_PushButton_4, SIGNAL(clicked()), SLOT(modifyMenu()) );
	dlgedit_PushButton_4->setAutoRepeat( FALSE );
	l->addWidget(dlgedit_PushButton_4, 5, 2);

	QPushButton* dlgedit_PushButton_3;
	dlgedit_PushButton_3 = new QPushButton( i18n("Delete"), this, "PushButton_3" );
	connect( dlgedit_PushButton_3, SIGNAL(clicked()), SLOT(deleteMenu()) );
	dlgedit_PushButton_3->setAutoRepeat( FALSE );
	l->addWidget(dlgedit_PushButton_3, 6, 2);

	ApplyButton = new QPushButton( i18n("Close"), this, "PushButton_1" );
	connect( ApplyButton, SIGNAL(clicked()), SLOT(terminate()) );
	ApplyButton->setAutoRepeat( FALSE );
	l->addWidget(ApplyButton, 7, 2);


}


UserMenuRefData::~UserMenuRefData()
{
}
void UserMenuRefData::typeSetActive(int)
{
}
void UserMenuRefData::terminate()
{
}
void UserMenuRefData::insertMenu()
{
}
void UserMenuRefData::newHighlight(int)
{
}
void UserMenuRefData::deleteMenu()
{
}
void UserMenuRefData::modifyMenu()
{
}
#include "UserMenuRefData.moc"
