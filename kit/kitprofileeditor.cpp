// kitprofileeditor.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include <qvbox.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qradiobutton.h>

#include <kbuttonbox.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "aim.h"
#include "kitprofileeditor.h"
#include "kitapp.h"

// *********************************
// * class ProfileEditor -- public *
// *********************************

ProfileEditor::ProfileEditor(KitUserProfile *parent, const char *name)
	: KDialogBase(KDialogBase::TreeList, name, Ok|Cancel|Help, Ok, 0, name)
{
	// Global
	profile = parent;

#define PE_SETUPBOX(a) a->setMargin( marginHint() ); a->setSpacing( spacingHint() );
	// *******************************************
	// Part 1 : (u)ser information
	// userinfo, infoinfo
	QVBox *uBox = addVBoxPage(i18n("User Information")); PE_SETUPBOX(uBox);

	QHBox *uNameBox = new QHBox(uBox); PE_SETUPBOX(uNameBox);
	new QLabel(i18n("Screen Name"), uNameBox);
	uName = new KLineEdit(uNameBox);
	uName->setMaxLength(30);
	new QLabel(i18n("Password"), uNameBox);
	uPwd = new KLineEdit(uNameBox);
	uPwd->setMaxLength(30);
	uPwd->setEchoMode(KLineEdit::Password);

	uSave = new QCheckBox(i18n("Save Password"), uBox);

	QVBox *uInfoBox = new QVBox(uBox); PE_SETUPBOX(uInfoBox);
	new QLabel(i18n("User Information"), uInfoBox);
	uInfo = new KEdit(uInfoBox);
	
	// *******************************************
	// Part 2 : buddy (l)ist information
	// listinfo parts, tab 4 parts
	QHBox *lBox = addHBoxPage(i18n("Buddy Lists")); PE_SETUPBOX(lBox);

	QVBox *lBox1 = new QVBox(lBox);
	new QLabel(i18n("Groups"), lBox1);

	QHBox *lGroupBox = new QHBox(lBox1); PE_SETUPBOX(lGroupBox);
	KButtonBox *lGroupBBox = new KButtonBox(lGroupBox, Qt::Vertical);
	QButton *lAddGroup = lGroupBBox->addButton(i18n("Add"));
	QButton *lDelGroup = lGroupBBox->addButton(i18n("Remove"));
	QButton *lRenGroup = lGroupBBox->addButton(i18n("Rename"));
	lGroupBBox->layout();
	lGroup = new QListBox(lGroupBox);

	QVBox *lBox2 = new QVBox(lBox);
	new QLabel(i18n("Group Members"), lBox2);

	QHBox *lBuddyBox = new QHBox(lBox2); PE_SETUPBOX(lBuddyBox);
	KButtonBox *lBuddyBBox = new KButtonBox(lBuddyBox, Qt::Vertical);
	QButton *lAddBuddy = lBuddyBBox->addButton(i18n("Add"));
	QButton *lDelBuddy = lBuddyBBox->addButton(i18n("Remove"));
	lBuddyBBox->layout();
	lBuddy = new QListBox(lBuddyBox);
	// *******************************************
	// Part 3 : (p)ermissions
	// listinfo parts
	QFrame *pBox = addPage(i18n("Permissions"));

	QHBox *pBoxRoot = new QHBox(pBox);

	QVBox *pBox1 = new QVBox(pBoxRoot);
	new QLabel(i18n("Permit List"), pBox1);

	QHBox *pPermitBox = new QHBox(pBox1); PE_SETUPBOX(pPermitBox);
	KButtonBox *pPermitBBox = new KButtonBox(pPermitBox, Qt::Vertical);
	QButton *pAddPermit = pPermitBBox->addButton(i18n("Add"));
	QButton *pDelPermit = pPermitBBox->addButton(i18n("Remove"));
	pPermitBBox->layout();
	pPermit = new QListBox(pPermitBox);

	QVBox *pBox2 = new QVBox(pBoxRoot);
	new QLabel(i18n("Deny List"), pBox2);

	QHBox *pDenyBox = new QHBox(pBox2); PE_SETUPBOX(pDenyBox);
	KButtonBox *pDenyBBox = new KButtonBox(pDenyBox, Qt::Vertical);
	QButton *pAddDeny = pDenyBBox->addButton(i18n("Add"));
	QButton *pDelDeny = pDenyBBox->addButton(i18n("Remove"));
	pDenyBBox->layout();
	pDeny = new QListBox(pDenyBox);

	pState = new QButtonGroup(2, Vertical, i18n("Permission Setting"), pBox);
	pState->setMargin( marginHint() );
	pState->setExclusive(true);
	new QRadioButton(i18n("Permit All"), pState);
	new QRadioButton(i18n("Use Permit List"), pState);
	new QRadioButton(i18n("Deny All"), pState);
	new QRadioButton(i18n("Use Deny List"), pState);

	QVBoxLayout *pLayout = new QVBoxLayout(pBox, marginHint(), spacingHint());
	pLayout->addWidget(pBoxRoot, true);
	pLayout->addWidget(pState, false);
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
	// *******************************************
	// Part 5 : (a)ppearance
	// tab 1
	QFrame *aBox = addPage(i18n("Main Window"));

	aHide = new QCheckBox(i18n("Hide Window on Start"), aBox);
	
	QButtonGroup *aColumns = new QButtonGroup(2, Vertical, i18n("Display Columns"), aBox);
	aColumns->setMargin( marginHint() );
	aColumns->setExclusive(false);
	aIdle = new QCheckBox(i18n("Idle Time"), aColumns);
	aClass = new QCheckBox(i18n("User Class"), aColumns);
	aWarn = new QCheckBox(i18n("Warning Level"), aColumns);
	aSignon = new QCheckBox(i18n("Signon Time"), aColumns);
	aNick = new QCheckBox(i18n("Nickname"), aColumns);

	aGrouping = new QButtonGroup(2, Horizontal, i18n("Show Buddies by"), aBox);
	aGrouping->setMargin( marginHint() );
	aGrouping->setExclusive(true);
	new QRadioButton(i18n("Group"), aGrouping);
	new QRadioButton(i18n("Status"), aGrouping);
	
	QVBoxLayout *aLayout = new QVBoxLayout(aBox, marginHint(), spacingHint());
	aLayout->addWidget(aHide, false);
	aLayout->addWidget(aColumns, false);
	aLayout->addWidget(aGrouping, false);
	aLayout->addStretch(true);
	// *******************************************
	// Part 6 : (b)ehavior
	// tab 2, tab 4 parts
	QFrame *bBox = addPage(i18n("Chat Windows"));

	bLog = new QCheckBox(i18n("Log Chats"), bBox);
	bTimestamp = new QCheckBox(i18n("Timestamp Chats"), bBox);
	bICQ = new QCheckBox(i18n("ICQ Mode (no HTML sent)"), bBox);

	QVGroupBox *bText = new QVGroupBox(bBox);
	bText->setMargin(marginHint());
	bText->setFrameStyle(QFrame::Box | QFrame::Sunken);
	bText->setTitle(i18n("My message appearance"));
	QHBox *bColor = new QHBox(bText); PE_SETUPBOX(bColor);
	new QLabel(i18n("Text Color"), bColor);
	bForeground = new KColorButton(bColor);
	new QLabel(i18n("Background Color"), bColor);
	bBackground = new KColorButton(bColor);
	bBold = new QCheckBox(i18n("Bold"), bText);

	QVBoxLayout *bLayout = new QVBoxLayout(bBox, marginHint(), spacingHint());
	bLayout->addWidget(bLog, false);
	bLayout->addWidget(bTimestamp, false);
	bLayout->addWidget(bICQ, false);
	bLayout->addWidget(bText, false);
	bLayout->addStretch(true);
	// *******************************************
	// Part 7 : (n)etworking
	// tab 3
	QFrame *nBox = addPage(i18n("Network"));

	nKeepAlive = new QCheckBox(i18n("Keep Connection Alive"), nBox);
	nAutoConnect = new QCheckBox(i18n("Automatically Connect on Startup"), nBox);

	// ******* Server
	QVGroupBox *nBigServerBox = new QVGroupBox(nBox);
	nBigServerBox->setMargin(marginHint());
	nBigServerBox->setFrameStyle(QFrame::Box | QFrame::Sunken);
	nBigServerBox->setTitle(i18n("Server Settings"));

	nUseCustom = new QCheckBox(i18n("Specify custom server, rather than default (AOL) server"), nBigServerBox);
	QHBox *nServerBox = new QHBox(nBigServerBox); PE_SETUPBOX(nServerBox);
	new QLabel(i18n("TOC Server and port"), nServerBox);
	nServer = new QLineEdit(nServerBox);
	nServerPort = new KRestrictedLine(nServerBox);
	nServerPort->setValidChars("0123456789\t");
	nServerPort->setMaxLength(5);
	nServerPort->setFixedWidth(50);

	QHBox *nAuthBox = new QHBox(nBigServerBox); PE_SETUPBOX(nAuthBox);
	new QLabel(i18n("Authorisation server"), nAuthBox);
	nAuth = new QLineEdit(nAuthBox);
	nAuthPort = new KRestrictedLine(nAuthBox);
	nAuthPort->setValidChars("0123456789\t");
	nAuthPort->setMaxLength(5);
	nAuthPort->setFixedWidth(50);

	QVBoxLayout *nLayout = new QVBoxLayout(nBox, marginHint(), spacingHint());
	nLayout->addWidget(nKeepAlive, false);
	nLayout->addWidget(nAutoConnect, false);
	nLayout->addWidget(nBigServerBox, false);
	nLayout->addStretch(true);
#undef PE_SETUPBOX
	// *******************************************
	// Now to initialize values
	initWidgets();
	// *******************************************
	// Now to connect to signals
	connect(lGroup, SIGNAL(highlighted(const QString &)), this, SLOT(lGroupHighlighted(const QString &)) );
	connect(mAwayList, SIGNAL(highlighted(int)), this, SLOT(mAwayListHighlighted(int)) );
	connect(mAway, SIGNAL(textChanged(void)), this, SLOT(mAwayTextChanged(void)) );
#define PE_BUTTONSIGNAL(x, y) connect(x, SIGNAL(clicked(void)), this, SLOT( y(void) ) );
	PE_BUTTONSIGNAL(lAddGroup, lAddGroupClicked)
	PE_BUTTONSIGNAL(lRenGroup, lRenGroupClicked)
	PE_BUTTONSIGNAL(lDelGroup, lDelGroupClicked)
	PE_BUTTONSIGNAL(lAddBuddy, lAddBuddyClicked)
	PE_BUTTONSIGNAL(lDelBuddy, lDelBuddyClicked)
	PE_BUTTONSIGNAL(pAddPermit, pAddPermitClicked)
	PE_BUTTONSIGNAL(pDelPermit, pDelPermitClicked)
	PE_BUTTONSIGNAL(pAddDeny, pAddDenyClicked)
	PE_BUTTONSIGNAL(pDelDeny, pDelDenyClicked)
	PE_BUTTONSIGNAL(mRenAway, mRenAwayClicked)
	PE_BUTTONSIGNAL(mAddAway, mAddAwayClicked)
	PE_BUTTONSIGNAL(mDelAway, mDelAwayClicked)
	PE_BUTTONSIGNAL(nUseCustom, nUseCustomClicked)
#undef PE_BUTTONSIGNAL
}
ProfileEditor::~ProfileEditor()
{
}
void ProfileEditor::initWidgets(void)
{
	QString newText;
	KitUserData &data = *(profile->data());
	// Part 1
	uName->setText(data.name);
	uPwd->setText(data.password);
	uInfo->setText(data.personalInformation);
	uSave->setChecked(data.savePassword);
	// Part 2
	names = data.names;
	for( int i = 0; i < names.getCountGroup(); i++ )
		lGroup->insertItem(names.getNameGroup(i));
	if(names.getCountGroup())
	{
		lGroup->setCurrentItem(0);
		lGroupHighlighted(lGroup->currentText());
	}
	// Part 3
	permit = data.permit;
	deny = data.deny;
	for( int i = 0; i < permit.getCount(); i++ )
		pPermit->insertItem(permit.getName(i));
	for( int i = 0; i < deny.getCount(); i++ )
		pDeny->insertItem(deny.getName(i));
	switch(data.permitStatus)
	{
		case TOC_DENYSOME:
			pState->setButton(3);
			break;
		case TOC_PERMITSOME:
			pState->setButton(1);
			break;
		case TOC_DENYALL:
			pState->setButton(2);
			break;
		default:
		//case TOC_PERMITALL:
			pState->setButton(0);
			break;
	}
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
	// Part 5
	aHide->setEnabled(false);
	aIdle->setChecked( data.listColumns & 1 << COL_IDLE );
	aClass->setChecked( data.listColumns & 1 << COL_CLASS );
	aWarn->setChecked( data.listColumns & 1 << COL_WARN );
	aSignon->setChecked( data.listColumns & 1 << COL_SIGNON );
	aNick->setChecked( data.listColumns & 1 << COL_NICK );
	aGrouping->setEnabled(false);
	// Part 6
	bForeground->setColor(data.foregroundColor);
	bBackground->setColor(data.backgroundColor);
	bBold->setChecked(data.bold);
	bLog->setChecked(data.logging);
	bTimestamp->setChecked(data.timestamping);
	bICQ->setChecked(data.icqMode);
	// Part 7
	nAutoConnect->setChecked(data.autoConnect);
	nKeepAlive->setChecked(data.keepAlive);
	nUseCustom->setChecked(data.useCustomServer);
	nServer->setText(data.server);
	nAuth->setText(data.authorizer);
	newText.setNum(data.serverPort); nServerPort->setText(newText);
	newText.setNum(data.authorizerPort); nAuthPort->setText(newText);
	// Enable/Disable as appropriate
	nUseCustomClicked();
}
// * protected slots *
void ProfileEditor::accept(void)
{
	KitUserData &data = *(profile->data());
	// Part 1
	data.name = uName->text();
	data.password = uPwd->text();
	data.personalInformation = uInfo->text();
	data.savePassword = uSave->isChecked();
	// Part 2
	data.names = names;
	// Part 3
	switch(pState->id(pState->selected()))
	{
		case 3:
			data.permitStatus = TOC_DENYSOME;
			break;
		case 2:
			data.permitStatus = TOC_DENYALL;
			break;
		case 1:
			data.permitStatus = TOC_PERMITSOME;
			break;
		case 0:
			data.permitStatus = TOC_PERMITALL;
			break;
	}
	data.permit = permit;
	data.deny = deny;
	// Part 4
	data.awayMessages = awayMessages;
	data.awayMessageNames = awayMessageNames;
	// Part 5
	data.listColumns = 0;
	if(aIdle->isChecked()) data.listColumns |= 1 << COL_IDLE;
	if(aClass->isChecked()) data.listColumns |= 1 << COL_CLASS;
	if(aWarn->isChecked()) data.listColumns |= 1 << COL_WARN;
	if(aSignon->isChecked()) data.listColumns |= 1 << COL_SIGNON;
	if(aNick->isChecked()) data.listColumns |= 1 << COL_NICK;
	// Part 6
	data.foregroundColor = bForeground->color();
	data.backgroundColor = bBackground->color();
	data.bold = bBold->isChecked();
	data.logging = bLog->isChecked();
	data.timestamping = bTimestamp->isChecked();
	data.icqMode = bICQ->isChecked();
	// Part 7
	data.autoConnect = nAutoConnect->isChecked();
	data.keepAlive = nKeepAlive->isChecked();
	data.useCustomServer = nUseCustom->isChecked();
	data.server = nServer->text();
	data.authorizer = nAuth->text();
	data.serverPort = nServerPort->text().toInt();
	data.authorizerPort = nAuthPort->text().toInt();
	// Write changes
	profile->save();
	// close dialog, return accepted
	KDialogBase::accept();
}
void ProfileEditor::reject(void)
{
	// Nothing needs to be done
	KDialogBase::reject();
}
void ProfileEditor::lGroupHighlighted(const QString &a)
{
	int targetGroup = names.getNumGroup(a);
	lBuddy->clear();
	for(int i = 0; i < names.getCount(); i++)
		if(names.getGroup(i) == targetGroup)
			lBuddy->insertItem(names.getName(i));
}
void ProfileEditor::mAwayListHighlighted(int)
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
void ProfileEditor::mAwayTextChanged(void)
{
	awayMessages[ mAwayList->currentItem() ] = mAway->text();
}
// Big ugly list of slots, and supporting ugly list of macros
#define UPDATE_GROUP_BOX lGroup->clear();\
                         for( int i = 0; i < names.getCountGroup(); i++ )\
                           lGroup->insertItem(names.getNameGroup(i));\
                         lGroup->setCurrentItem(0);

#define UPDATE_BUDDY_BOX lBuddy->clear();\
                         lGroup->setCurrentItem(lGroup->currentItem());\
                         lGroupHighlighted(lGroup->currentText());

#define UPDATE_PERMIT_BOX pPermit->clear();\
                          for( int i = 0; i < permit.getCount(); i++ )\
                            pPermit->insertItem(permit.getName(i));

#define UPDATE_DENY_BOX pDeny->clear();\
                        for( int i = 0; i < deny.getCount(); i++ )\
                          pDeny->insertItem(deny.getName(i));

#define UPDATE_AWAY_BOX mAwayList->clear();\
                        for( unsigned i = 0; i < awayMessageNames.count(); i++ )\
                          mAwayList->insertItem(awayMessageNames[i]);\
                        mAwayListHighlighted(mAwayList->currentItem());

#define CHOOSE_FIRST(a, b) if(a->currentItem() == -1) {\
                             KMessageBox::error(0, b);\
                             return; }

void ProfileEditor::lAddGroupClicked(void)
{
	KLineEditDlg lineDialog(i18n("Enter the name of the group to add."), QString::null, this);
	if(lineDialog.exec() != Accepted) return;

	QString name = lineDialog.text();
	if(names.addGroup(name) == -1)
	{
		KMessageBox::error(0, i18n("You cannot add two groups with the same name."));
		return;
	}
	UPDATE_GROUP_BOX
}
void ProfileEditor::lRenGroupClicked(void)
{
	CHOOSE_FIRST(lGroup, i18n("Choose a group first."));
	KLineEditDlg lineDialog(i18n("Enter a new name for the group."), lGroup->currentText(), this);
	if(lineDialog.exec() != Accepted) return;

	QString name = lineDialog.text();
	if(names.renameGroup(lGroup->currentText(), name) == -1)
	{
		KMessageBox::error(0, i18n("You already have a group named %1.").arg(name));
		return;
	}
	UPDATE_GROUP_BOX
}
void ProfileEditor::lDelGroupClicked(void)
{
	CHOOSE_FIRST(lGroup, i18n("Choose a group first."));
	if(KMessageBox::warningYesNo(0, i18n("Are you sure you want to delete group %1?").arg(lGroup->currentText())) == KMessageBox::Yes)
	{
		names.delGroup(lGroup->currentText());
		UPDATE_GROUP_BOX
	}
}
void ProfileEditor::lAddBuddyClicked(void)
{
	CHOOSE_FIRST(lGroup, i18n("Choose a group first."));
	KLineEditDlg lineDialog(i18n("Enter the name to add to group %1").arg(lGroup->currentText()), QString::null, this);
	if(lineDialog.exec() != Accepted) return;

	QString name = lineDialog.text();
	TBuddy buddy;
	buddy.name = name;
	buddy.group = names.getNumGroup(lGroup->currentText());
	if(names.add(&buddy) == -1)
	{
		KMessageBox::error(0, i18n("%1 is already in your buddy list somewhere.").arg(name));
		return;
	}
	UPDATE_BUDDY_BOX
}
void ProfileEditor::lDelBuddyClicked(void)
{
	CHOOSE_FIRST(lBuddy, i18n("Choose a buddy first."));
	if(KMessageBox::warningYesNo(0, i18n("Are you sure you want to delete %1?").arg(lBuddy->currentText())) == KMessageBox::Yes)
	{
		names.del(lBuddy->currentText());
		UPDATE_BUDDY_BOX
	}
}
void ProfileEditor::pAddPermitClicked(void)
{
	KLineEditDlg lineDialog(i18n("Enter the name to add to your permit list."), QString::null, this);
	if(lineDialog.exec() != Accepted) return;

	QString name = lineDialog.text();
	TBuddy buddy;
	buddy.name = name;
	buddy.group = 0;
	if(permit.add(&buddy) == -1)
	{
		KMessageBox::error(0, i18n("%1 is already on your permit list.").arg(name));
		return;
	}
	UPDATE_PERMIT_BOX
}
void ProfileEditor::pDelPermitClicked(void)
{
	CHOOSE_FIRST(pPermit, i18n("Choose a name first."));
	if(KMessageBox::warningYesNo(0, i18n("Are you sure you want to delete %1?").arg(pPermit->currentText())) == KMessageBox::Yes)
	{
		permit.del(pPermit->currentText());
		UPDATE_PERMIT_BOX
	}
}
void ProfileEditor::pAddDenyClicked(void)
{
	KLineEditDlg lineDialog(i18n("Enter the name to add to your deny list."), QString::null, this);
	if(lineDialog.exec() != Accepted) return;

	QString name = lineDialog.text();
	TBuddy buddy;
	buddy.name = name;
	buddy.group = 0;
	if(deny.add(&buddy) == -1)
	{
		KMessageBox::error(0, i18n("%1 is already on your deny list.").arg(name));
		return;
	}
	UPDATE_DENY_BOX
}
void ProfileEditor::pDelDenyClicked(void)
{
	CHOOSE_FIRST(pDeny, i18n("Choose a name first."));
	if(KMessageBox::warningYesNo(0, i18n("Are you sure you want to delete %1?").arg(pDeny->currentText())) == KMessageBox::Yes)
	{
		deny.del(pDeny->currentText());
		UPDATE_DENY_BOX
	}
}
void ProfileEditor::mRenAwayClicked(void)
{
	CHOOSE_FIRST(mAwayList, i18n("Choose a message first."));
	KLineEditDlg lineDialog(i18n("Enter a new name for the message."), mAwayList->currentText(), this);
	if(lineDialog.exec() != Accepted) return;

	awayMessageNames[mAwayList->currentItem()] = lineDialog.text();
	UPDATE_AWAY_BOX
}
void ProfileEditor::mAddAwayClicked(void)
{
	KLineEditDlg lineDialog(i18n("Enter a name for the new away message."), QString::null, this);
	if(lineDialog.exec() != Accepted) return;

	awayMessageNames += lineDialog.text();
	awayMessages += QString::null;
	UPDATE_AWAY_BOX
}
void ProfileEditor::mDelAwayClicked(void)
{
	CHOOSE_FIRST(mAwayList, i18n("Choose a message first."));
	if(KMessageBox::warningYesNo(0, i18n("Are you sure you want to delete %1?").arg(mAwayList->currentText())) == KMessageBox::Yes)
	{
		awayMessageNames.remove( awayMessageNames.at(mAwayList->currentItem()) );
		awayMessages.remove( awayMessages.at(mAwayList->currentItem()) );
		UPDATE_AWAY_BOX
	}
}
void ProfileEditor::nUseCustomClicked(void)
{
	bool enabled = nUseCustom->isChecked();
	nServer->setEnabled(enabled);
	nAuth->setEnabled(enabled);
	nServerPort->setEnabled(enabled);
	nAuthPort->setEnabled(enabled);
}

#include "kitprofileeditor.moc"
