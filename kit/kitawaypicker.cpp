// kitawaypicker.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>

#include <kbuttonbox.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "aim.h"
#include "kitawaypicker.h"
#include "kitapp.h"

// ******************************
// * class AwayPicker -- public *
// ******************************

AwayPicker::AwayPicker(KitUserProfile *parent, const char *name)
	: KDialogBase(KDialogBase::Tabbed, name, Ok|Cancel|Help, Ok, 0, name)
{
	// Global
	profile = parent;

#define PE_SETUPBOX(a) a->setMargin( marginHint() ); a->setSpacing( spacingHint() );
	// *******************************************
	// Part 4 : away (m)essages
	// awayinfo
	QHBox *mBox = addHBoxPage(i18n("Away Messages")); PE_SETUPBOX(mBox);

	QVBox *mBox1 = new QVBox(mBox); PE_SETUPBOX(mBox1);
	new QLabel(i18n("Name"), mBox1);

	QHBox *mListBox = new QHBox(mBox1); PE_SETUPBOX(mListBox);
	KButtonBox *mListBBox = new KButtonBox(mListBox, Qt::Vertical);
	QButton *mAddAway = mListBBox->addButton(i18n("Add"));
	QButton *mDelAway = mListBBox->addButton(i18n("Remove"));
	QButton *mRenAway = mListBBox->addButton(i18n("Rename"));
	mListBBox->layout();
	mAwayList = new QListBox(mListBox);

	QVBox *mEditBox = new QVBox(mBox); PE_SETUPBOX(mEditBox);
	new QLabel(i18n("Message"), mEditBox);
	mAway = new TEdit(mEditBox);
	mAway->setWordWrap(TEdit::WidgetWidth);
#undef PE_SETUPBOX
	// *******************************************
	// Now to initialize values
	initWidgets();
	// *******************************************
	// Now to connect to signals
	connect(mAwayList, SIGNAL(highlighted(int)), this, SLOT(mAwayListHighlighted(int)) );
	connect(mAway, SIGNAL(textChanged(void)), this, SLOT(mAwayTextChanged(void)) );
#define PE_BUTTONSIGNAL(x, y) connect(x, SIGNAL(clicked(void)), this, SLOT( y(void) ) );
	PE_BUTTONSIGNAL(mRenAway, mRenAwayClicked)
	PE_BUTTONSIGNAL(mAddAway, mAddAwayClicked)
	PE_BUTTONSIGNAL(mDelAway, mDelAwayClicked)
#undef PE_BUTTONSIGNAL
}
AwayPicker::~AwayPicker()
{
}
void AwayPicker::initWidgets(void)
{
	KitUserData &data = *(profile->data());
	// Part 4
	awayMessages = data.awayMessages;
	awayMessageNames = data.awayMessageNames;
	for( unsigned i = 0; i < awayMessageNames.count(); i++ )
		mAwayList->insertItem(awayMessageNames[i]);
	if(awayMessageNames.count() == 0)
		mAway->setEnabled(false);
	else
	{
		mAway->setEnabled(true);
		mAway->setText(awayMessages[0]);
		mAwayList->setCurrentItem(0);
	}
}
// * protected slots *
void AwayPicker::accept(void)
{
	KitUserData &data = *(profile->data());
	// Part 4
	data.awayMessages = awayMessages;
	data.awayMessageNames = awayMessageNames;
	// Write changes
	profile->save();
	// make chosen one available
	chosen = awayMessages[ mAwayList->currentItem() ];
	// close dialog, return accepted
	KDialogBase::accept();
}
void AwayPicker::reject(void)
{
	// Nothing needs to be done
	KDialogBase::reject();
}
void AwayPicker::mAwayListHighlighted(int)
{
	disconnect(mAway, 0, 0, 0);
	if(mAwayList->currentItem() == -1)
	{
		mAway->setText("");
		mAway->setEnabled(false);
	}
	else
	{
		mAway->setEnabled(true);
		mAway->setText(awayMessages[mAwayList->currentItem()]);
	}
	connect(mAway, SIGNAL(textChanged(void)), this, SLOT(mAwayTextChanged(void)) );
}
void AwayPicker::mAwayTextChanged(void)
{
	awayMessages[ mAwayList->currentItem() ] = mAway->text();
}
// Big ugly list of slots, and supporting ugly list of macros
#define UPDATE_AWAY_BOX mAwayList->clear();\
                        for( unsigned i = 0; i < awayMessageNames.count(); i++ )\
                          mAwayList->insertItem(awayMessageNames[i]);\
                        mAwayListHighlighted(mAwayList->currentItem());

#define CHOOSE_FIRST(a, b) if(a->currentItem() == -1) {\
                             KMessageBox::error(0, b);\
                             return; }

void AwayPicker::mRenAwayClicked(void)
{
	CHOOSE_FIRST(mAwayList, i18n("Choose a message first."));
	KLineEditDlg lineDialog(i18n("Enter a new name for the message."), mAwayList->currentText(), this);
	if(lineDialog.exec() != Accepted) return;

	awayMessageNames[mAwayList->currentItem()] = lineDialog.text();
	UPDATE_AWAY_BOX
}
void AwayPicker::mAddAwayClicked(void)
{
	KLineEditDlg lineDialog(i18n("Enter a name for the new away message."), QString::null, this);
	if(lineDialog.exec() != Accepted) return;

	awayMessageNames += lineDialog.text();
	awayMessages += QString::null;
	UPDATE_AWAY_BOX
}
void AwayPicker::mDelAwayClicked(void)
{
	CHOOSE_FIRST(mAwayList, i18n("Choose a message first."));
	if(KMessageBox::warningYesNo(0, i18n("Are you sure you want to delete %1?").arg(mAwayList->currentText())) == KMessageBox::Yes)
	{
		awayMessageNames.remove( awayMessageNames.at(mAwayList->currentItem()) );
		awayMessages.remove( awayMessages.at(mAwayList->currentItem()) );
		UPDATE_AWAY_BOX
	}
}

#include "kitawaypicker.moc"
