// twindow.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include <kaboutapplication.h>
#include <kaboutkde.h>
#include <kbugreport.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <kmenubar.h>
#include <knotifyclient.h>
#include <kwin.h>
#include <kmessagebox.h>
#include <qheader.h>
#include <klocale.h>

#include "kitapp.h"
#include "kitdirectorysearch.h"
#include "kitpasswordchanger.h"
#include "twindow.h"

inline bool sanityCheck(TAim &aim)
{
	if(aim.getStatus() == TAIM_OFFLINE)
	{
		KMessageBox::error(0, i18n("You are offline."));
		return false;
	}
	else return true;
}

void TWindow::updateTreeItem(QListViewItem *item, TBuddy &currentBuddy, int listColumns)
{
	QString bName, iTime, wLevel, sTime, uClass;

	// name
	bName = currentBuddy.name;

	if(currentBuddy.status == TAIM_ONLINE)
	{
		// idle time (in seconds)
		iTime.setNum(currentBuddy.idleTime);
		// warning level (percent 0-100)
		wLevel.setNum(currentBuddy.evil); wLevel += '%';
		// signon time (in UNIX time.. needs conversion to something readable)
		QDateTime signonTime;
		signonTime.setTime_t(currentBuddy.signonTime);
		sTime = signonTime.toString();

		// user class
		switch(currentBuddy.userClass)
		{
			case TOC_USER_AOL:
				uClass = i18n("AOL");
				break;
			case TOC_USER_UNCONFIRMED:
				uClass = i18n("Unconfirmed");
				break;
			case TOC_USER_ADMIN:
				uClass = i18n("Administrator");
				break;
			case TOC_USER_NORMAL:
			default:
				uClass = i18n("Normal");
				break;
		}
	}

	// we have the information, now stuff it into the item
	item->setText(COL_NAME, bName);
	int i = 1;
	if(listColumns & (1 << COL_NICK)) item->setText(i++, profile->data()->nicknames[bName]);
	if(listColumns & (1 << COL_IDLE)) item->setText(i++, iTime);
	if(listColumns & (1 << COL_WARN)) item->setText(i++, wLevel);
	if(listColumns & (1 << COL_SIGNON)) item->setText(i++, sTime);
	if(listColumns & (1 << COL_CLASS)) item->setText(i++, uClass);
}

// ***************************
// * class TWindow -- public *
// ***************************
TWindow::TWindow(KitUserProfile *user, const char *name, WFlags f)
	: KMainWindow(0, name, f)
{
	profile = user;
	// this is important, because closing a chat window will cause problems otherwise
	messageWindows.setAutoDelete(false);
	resize(320, 320);
	starsAndTheMoon();
}
TWindow::TWindow(const char *name, WFlags f)
	: KMainWindow(0, name, f)
{
	messageWindows.setAutoDelete(false);
	profile = 0;
}
TWindow::~TWindow()
{
	// unlock
	delete profile;
	// now to let the app know I'm done, so the app can quit if necessary
	emit closing();
	// close windows
	closeMessageWindows();
}
void TWindow::starsAndTheMoon(void)
{
	connect(profile, SIGNAL(updated(void)), this, SLOT(profileUpdated(void)) );

	// nowhere else to put this
	awayMessage = QString::null;

	// dock
	// dock comes before menu because we need the dockWindow's context menu
	dockWindow = new TDockWindow(this);
	dockWindow->show();
	dockMenu = dockWindow->contextMenu();
	
	// menu
	initMenu();
	
	// aim
	connect( &aim, SIGNAL(statusChanged(int)), this, SLOT(setStatus(int)) );
	connect( &aim, SIGNAL(userInfoReceived(QString)), this, SLOT(displayInfo(QString)) );
	connect( &aim, SIGNAL(IMReceived(QString, QString, bool)), this, SLOT(receivedIM(QString, QString, bool)) );
	connect( &aim, SIGNAL(initProgress(int, int, QString)), this, SLOT(initProgress(int, int, QString)) );
	connect( &aim, SIGNAL(updateProgress(int, QString)), this, SLOT(updateProgress(int, QString)) );
	connect( &aim, SIGNAL(endProgress()), this, SLOT(endProgress()) );
	connect( &aim, SIGNAL(displayError(QString)), this, SLOT(displayError(QString)) );
	connect( &aim, SIGNAL(configChanged()), this, SLOT(initTree()) );
	connect( &aim, SIGNAL(buddyChanged(int)), this, SLOT(updateTree(int)) );
	connect( &aim, SIGNAL(warningReceived(int, QString)), this, SLOT(displayWarning(int, QString)) );
	connect( &aim, SIGNAL(passwordChanged()), this, SLOT(passwordChanged()) );
	aim.doIdleTime(true);

	progress = 0;

	// Visible Widgets
	mainView = new QVBox(this);
	mainView->setMargin(KDialog::marginHint());
	mainView->setSpacing(KDialog::spacingHint());
	mainView->setMinimumSize(100, 250);
	setCentralWidget(mainView);

	treelist = new QListView(mainView);
	treelist->setAllColumnsShowFocus(true);
	treelist->setRootIsDecorated(true);
	treelist->setTreeStepSize(10);
	treelist->setSorting(0);
	treelist->show();
	connect( treelist, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(selectedTree(QListViewItem *)) );
	connect( treelist, SIGNAL(returnPressed(QListViewItem *)), this, SLOT(selectedTree(QListViewItem *)) );
	connect( treelist, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)), this, SLOT(clickedTree(QListViewItem *, const QPoint &, int)) );

	statusButton = new QComboBox(mainView);
	statusButton->insertItem( SmallIcon("kit_online"), i18n("Online") );
	statusButton->insertItem( SmallIcon("kit_away"), i18n("Away") );
	statusButton->insertItem( SmallIcon("kit_offline"), i18n("Offline") );
	connect( statusButton, SIGNAL(activated(const QString &)), this, SLOT(statusClicked(const QString &)) );
	
	// we obviously start offline.. this gets all the widgets together
	setStatus(TAIM_OFFLINE);

	// Window properties
	if(profile->data()->width > 0)
		setGeometry(profile->data()->x, profile->data()->y, profile->data()->width, profile->data()->height);
	else
		resize(300, 300);
	if(profile->data()->name != QString::null) setCaption(profile->data()->name);
	switch((NET::MappingState)profile->data()->state)
	{	
		case NET::Withdrawn:
			hide();
			break;
		case NET::Visible:
			showNormal();
			break;
		case NET::Iconic:
			showMinimized();
			break;
	}
	profileUpdated();

	if(profile->data()->autoConnect)
		commandCallback(ID_TONLINE);
}
void TWindow::commandCallback(int command)
{
	switch(command)
	{
		case ID_TONLINE:
			setStatus(TAIM_ONLINE);
			awayMessage = QString::null;
			emit globalAway(false, QString::null);
			if(aim.getStatus() != TAIM_ONLINE)
			{
				// if no password or username specified, ask for one
				// if the dialog returns successfully, try again
				if( profile->data()->name.isNull() ||
				    (!profile->data()->password.length() && !profile->data()->savePassword)
				  )
					profile->execEditor();
				aim.setPassword(profile->data()->password);
				aim.setStatus(TAIM_ONLINE);
				initTree();
			}
			break;
		case ID_TAWAY:
			if(aim.getStatus() != TAIM_ONLINE) break;
			awayMessage = doAway();
			if(awayMessage != QString::null)
			{
				emit globalAway(true, awayMessage);
				setStatus(TAIM_AWAY);
			}
			else setStatus(TAIM_ONLINE);
			break;
		case ID_TOFFLINE:
			aim.setStatus(TAIM_OFFLINE);
			break;
		case ID_TOPEN:
			if(!sanityCheck(aim)) return;
			{
			KLineEditDlg lineDialog(i18n("Enter the screen name to open."), QString::null, this);
			if(lineDialog.exec() != KLineEditDlg::Accepted) return;
			TMessage &targetWindow = messageWindow(lineDialog.text());
			targetWindow.show();
			}
			break;
		case ID_TEXIT:
			if(QApplication::sendEvent(this, new QCloseEvent())) delete this;
			break;
		case ID_TPREFERENCES:
			profile->execEditor();
			break;
		case ID_TPROFILES:
			((KitApp *)KApplication::kApplication())->popProfileInfo();
			break;
		case ID_TABOUT:
			{
			KAboutApplication aboutBox;
			aboutBox.exec();
			}
			break;
		case ID_TABOUTKDE:
			{
			KAboutKDE aboutBox;
			aboutBox.exec();
			}
			break;
		case ID_TBUG:
			{
			KBugReport bugBox;
			bugBox.exec();
			}
			break;
		case ID_TDIRECTORY:
			if(!sanityCheck(aim)) return;
			{
			KitDirectorySearch searchBox;
			TAim::directory directory;
			if(searchBox.exec())
			{
				directory = searchBox.result();
				aim.searchDirectory(directory);
			}
			}
			break;
		case ID_THELP:
			appHelpActivated();
			break;
		case ID_TPASSCHANGE:
			if(!sanityCheck(aim)) return;
			{
			KitPasswordChanger dlg;
			dlg.setOldPassword(profile->data()->password);
			if(dlg.exec())
			{
				aim.changePassword(dlg.oldPassword(), dlg.newPassword());
				newPassword = dlg.newPassword();
			}
			}
			break;
  	};
}
// *****************************
// * class TWindow - protected *
// *****************************
void TWindow::closeEvent(QCloseEvent *event)
{
	/* Getting too many complaints about this
	if(aim.getStatus() != TAIM_OFFLINE)
	if(KMessageBox::No == KMessageBox::warningYesNo(this, i18n("Are you sure you want to disconnect and quit?")))
	{
		event->ignore();
		return;
	}
	*/
	profile->data()->x = x();
	profile->data()->y = y();
	profile->data()->width = width();
	profile->data()->height = height();
	profile->data()->state = (int)KWin::info(winId()).mappingState;
	event->accept();
}
void TWindow::saveProperties(KConfig *sConfig)
{
	sConfig->writeEntry("user", profile->name());
	sConfig->writeEntry("windowState", (int)KWin::info(winId()).mappingState);
}
void TWindow::readProperties(KConfig *sConfig)
{
	QString user = sConfig->readEntry("user");
	profile = new KitUserProfile(user);
	if(!profile->valid())
	{
		profile->unlock();
		delete profile;
		profile = new KitUserProfile(user);
	}
	starsAndTheMoon();
	switch((NET::MappingState)sConfig->readNumEntry("windowState"))
	{	
		case NET::Withdrawn:
			hide();
			break;
		case NET::Visible:
			showNormal();
			break;
		case NET::Iconic:
			showMinimized();
			break;
	}
}
void TWindow::initTree(void)
{
	treelist->clear();

	// remove all columns
	while(treelist->header()->count() > 0)
		treelist->removeColumn(0);

	if(aim.getStatus() == TAIM_OFFLINE) return;

	// we're online, so get going...
	
	// add columns as necessary, and headers
	treelist->addColumn("Screen Name", -1);
	if(profile->data()->listColumns & (1 << COL_NICK)) treelist->addColumn(i18n("Nickname"), -1);
	if(profile->data()->listColumns & (1 << COL_IDLE)) treelist->addColumn(i18n("Idle"), -1);
	if(profile->data()->listColumns & (1 << COL_WARN)) treelist->addColumn(i18n("Warn"), -1);
	if(profile->data()->listColumns & (1 << COL_SIGNON)) treelist->addColumn(i18n("Signon"), -1);
	if(profile->data()->listColumns & (1 << COL_CLASS)) treelist->addColumn(i18n("Class"), -1);

	// get list to use...
	const TBuddyList &list = aim.buddyList();
	TBuddy currentBuddy;

	// display by group, with icons for status
	int currentGroup = -1;
	QListViewItem *groupItem = 0;
	QString groupName;

	for(int i = 0; i < list.getCount(); i++)
	{
		// set the group, to put the buddy item
		if(currentGroup < list.getGroup(i))
		{
			currentGroup = list.getGroup(i);
			if(list.getCountGroup() <= currentGroup)
				groupName = i18n("Group %1").arg(currentGroup);
			else
				groupName = list.getNameGroup(currentGroup);
				
			groupItem = new QListViewItem(treelist, groupName);
			groupItem->setPixmap(COL_NAME, SmallIcon("kit_group"));
			groupItem->setOpen(true);
			treelist->insertItem(groupItem);
		}
			
		// we've set the group, now to get the info to put into the actual item
		// this stuff used to be in TBuddyInfo
		QListViewItem *newChild = new QListViewItem(groupItem);
		
		list.get(&currentBuddy, i);
		updateTreeItem(newChild, currentBuddy, profile->data()->listColumns);

		newChild->setPixmap(COL_NAME, currentBuddy.status == TAIM_ONLINE ? SmallIcon("kit_online") : SmallIcon("kit_offline") );
		groupItem->insertItem(newChild);
	}
	treelist->triggerUpdate();
}
void TWindow::updateTree(int updated)
{
	if(treelist == 0) return;

	const TBuddyList &list = aim.buddyList();
	QListViewItem *item;
	QListViewItem *parent;
	TBuddy currentBuddy;
	
	parent = treelist->firstChild();
	if(parent == 0) return;

	while( parent != 0 )
	{
		item = parent->firstChild();
		while( item != 0 )
		{
			if( tocNormalize(item->text(COL_NAME)) == tocNormalize(list.getName(updated)) )
			{
				list.get(&currentBuddy, updated);
				
				updateTreeItem(item, currentBuddy, profile->data()->listColumns);
				if(currentBuddy.status)
				{
					item->setPixmap(COL_NAME, SmallIcon("kit_online") );
				}
				else
				{
					item->setPixmap(COL_NAME, SmallIcon("kit_offline") );
				}

				treelist->repaint();
				return;
			}
			item = item->nextSibling();
		}
		parent = parent->nextSibling();
	}
	treelist->triggerUpdate();
}
void TWindow::selectedTree(QListViewItem *listItem)
{
	// if the item has no parent, then it's a group, and I don't care if it is selected
	if(listItem->parent() == 0) return;
	// if the person's offline, back out
	int num = aim.buddyList().getNum(listItem->text(COL_NAME));
	if(aim.buddyList().getStatus(num) != TAIM_ONLINE)
	{
		QString errorMessage = QString::fromLatin1("%1 is not available.").arg(listItem->text(COL_NAME));
		KMessageBox::error(0, errorMessage);
		return;
	}
	// get the name of the requested person
	QString name = listItem->text(COL_NAME);
	TMessage &targetWindow = messageWindow(name);
	targetWindow.show();
}
void TWindow::infoTree(QListViewItem *listItem)
{
	// if the item has no parent, then it's a group, and I don't care if it is selected
	if(!listItem || !listItem->parent()) return;
	// if the person's offline, back out
	int num = aim.buddyList().getNum(listItem->text(COL_NAME));
	if(aim.buddyList().getStatus(num) != TAIM_ONLINE)
	{
		QString errorMessage = QString::fromLatin1("%1 is not available.").arg(listItem->text(COL_NAME));
		KMessageBox::error(0, errorMessage);
		return;
	}
	// send the request
	aim.requestUserInfo( listItem->text(COL_NAME) );
}
void TWindow::directoryTree(QListViewItem *listItem)
{
	// if the item has no parent, then it's a group, and I don't care if it is selected
	if(!listItem || !listItem->parent()) return;
	// if the person's offline, back out
	int num = aim.buddyList().getNum(listItem->text(COL_NAME));
	if(aim.buddyList().getStatus(num) != TAIM_ONLINE)
	{
		QString errorMessage = QString::fromLatin1("%1 is not available.").arg(listItem->text(COL_NAME));
		KMessageBox::error(0, errorMessage);
		return;
	}
	// send the request
	aim.requestDirectory( listItem->text(COL_NAME) );
}
void TWindow::nickTree(QListViewItem *listItem)
{
	// if the item has no parent, then it's a group, and I don't care if it is selected
	if(!listItem || !listItem->parent()) return;

	QString name = listItem->text(COL_NAME);

	QString msg(i18n("Enter a nickname for %1"));
	msg = msg.arg(name);
	
	KLineEditDlg nameDialog(msg, profile->data()->nicknames[name], this);
	if(nameDialog.exec() != KLineEditDlg::Accepted) return;

	profile->data()->nicknames[name] = nameDialog.text();
	initTree();
}
void TWindow::clickedTree(QListViewItem *item, const QPoint &point, int)
{
	if(aim.getStatus() == TAIM_OFFLINE) return;
	// save item, for the menu to know later
	treeCurrent = item;
	if(!item)
	{
		treeEmptyMenu->popup(point);
		return;
	}
	// if the item has a parent, then it's a name, and we want the name menu
	// otherwise, we want the group menu
	KPopupMenu *menu = !(item->parent()) ? treeGroupMenu : treeMenu;
	menu->removeItemAt(0);
	menu->insertTitle(item->text(COL_NAME), -1, 0);
	menu->popup(point);
}
void TWindow::clickedTreeMenu(int item)
{
	switch(item)
	{
	case ID_TNICK:
		nickTree(treeCurrent);
		break;
	case ID_TDIRECTORY:
		directoryTree(treeCurrent);
		break;
	case ID_TMESSAGE:
		selectedTree(treeCurrent);
		break;
	case ID_TINFO:
		infoTree(treeCurrent);
		break;
	case ID_TADDBUDDY:
		addBuddyToTree();
		break;
	case ID_TADDGROUP:
		addGroupToTree();
		break;
	case ID_TDELBUDDY:
		delBuddyFromTree();
		break;
	};
}
void TWindow::statusClicked(const QString &status)
{
	if(status == i18n("Online"))
		commandCallback(ID_TONLINE);
	else if(status == i18n("Away"))
		commandCallback(ID_TAWAY);
	else if(status == i18n("Offline"))
		commandCallback(ID_TOFFLINE);
}
void TWindow::setStatus(int status)
{
	switch(status)
	{
	case TAIM_ONLINE:
		dockWindow->setPixmap( SmallIcon("kit_online") );
		statusButton->setCurrentItem(0);
		KNotifyClient::event("Online");
		break;
	case TAIM_AWAY:
		dockWindow->setPixmap( SmallIcon("kit_away") );
		statusButton->setCurrentItem(1);
		break;
	case TAIM_OFFLINE:
		dockWindow->setPixmap( SmallIcon("kit_offline") );
		statusButton->setCurrentItem(2);
		initTree();
		KNotifyClient::event("Offline");
		break;
	};
}
void TWindow::receivedIM(QString name, QString message, bool isAuto)
{
	TMessage &targetWindow = messageWindow(name);
	targetWindow.show();
	targetWindow.messageIn(message, isAuto);
}
void TWindow::sendingIM(QString message, QString target, bool automatic)
{
	if(!sanityCheck(aim)) return;
	aim.sendIM(message, target, automatic);
}
void TWindow::getInfo(QString name)
{
	if(!sanityCheck(aim)) return;
	aim.requestUserInfo( name );
}
void TWindow::sendingWarning(QString name, bool isAnonymous)
{
	if(!sanityCheck(aim)) return;
	aim.sendWarning(name, isAnonymous);
}
void TWindow::awayPicker(TMessage &a)
{
	QString message = doAway();
	if(message != QString::null)
		a.setAway(true, message);
}
void TWindow::closingTMessage(const QString &n)
{
	if(messageWindows[n])
	{
		delete messageWindows[n];
		messageWindows.remove(n);
	}
}
void TWindow::displayInfo(QString url)
{
	KApplication::kApplication()->invokeBrowser(url);
}
void TWindow::initProgress(int max, int cur, QString msg)
{
	if(progress) delete progress;
	progress = new QProgressDialog;
	progress->setTotalSteps(max);
	connect( progress, SIGNAL(cancelled()), &aim, SLOT(cancelProgress()) );
	progress->setProgress(cur);
	progress->setLabelText(msg);
	progress->show();
}
void TWindow::updateProgress(int cur, QString msg)
{
	if(!progress) return;
	progress->setProgress(cur);
	progress->setLabelText(msg);
}
void TWindow::endProgress(void)
{
	if(!progress) return;
	delete progress;
	progress = 0;
}
void TWindow::displayError(QString msg)
{
	KMessageBox::error(0, msg);
}
void TWindow::displayWarning(int percent, QString victim)
{
	QString message;
	if (victim.isNull())
		message = i18n("You have been warned anonymously.");
	else
		message = i18n("You have been warned by %1.").arg(victim);
	
	QString message2;
	message2 = i18n("\nYour new warning level is %1%.").arg(percent);
	KNotifyClient::event("Warning Received", message + message2);
}
void TWindow::profileUpdated(void)
{
	initTree();
	// AIM Configuration
	aim.setUserName(profile->data()->name);
	aim.setPassword(profile->data()->password);
	aim.setDirectory(profile->data()->dir);
	aim.setBuddyList(profile->data()->names);
	aim.setPermitList(profile->data()->permit);
	aim.setDenyList(profile->data()->deny);
	aim.setPermissions(profile->data()->permitStatus);
	aim.setUserInfo(profile->data()->personalInformation);
	aim.doKeepAlive(profile->data()->keepAlive);
	aim.useCustomServer(profile->data()->useCustomServer);
	aim.setCustomServer(profile->data()->server, profile->data()->serverPort, profile->data()->authorizer, profile->data()->authorizerPort);

	// message window configuration
	emit globalLogging(profile->data()->logging);
	emit globalTimestamping(profile->data()->timestamping);
	emit globalMyName(profile->data()->name);
	emit globalColors(profile->data()->foregroundColor, profile->data()->backgroundColor, profile->data()->bold);
	emit globalICQ(profile->data()->icqMode);

	dockMenu->removeItemAt(0);
	dockMenu->insertTitle(profile->data()->name, -1, 0);
}
void TWindow::passwordChanged(void)
{
	KMessageBox::information(this, i18n("Your password change was successful."));
	if(profile->data()->savePassword)
		profile->data()->password = newPassword;
}


// ****************************
// * class TWindow -- private *
// ****************************
void TWindow::initMenu(void)
{
	file = new QPopupMenu(this);
	file->insertItem( SmallIconSet("kit_online"), i18n("&Online"), ID_TONLINE );
	file->insertItem( SmallIconSet("kit_away"), i18n("&Away..."), ID_TAWAY );
	file->insertItem( SmallIconSet("kit_offline"), i18n("O&ffline"), ID_TOFFLINE );
	file->insertSeparator();
	file->insertItem( SmallIconSet("mail_send"), i18n("&Open chat..."), ID_TOPEN );
	file->insertSeparator();
	file->insertItem( i18n("&Change Password..."), ID_TPASSCHANGE );
	file->insertItem( SmallIconSet("find"), i18n("&Search directory..."), ID_TDIRECTORY);
	file->insertSeparator();
	file->insertItem( SmallIconSet("exit"), i18n("E&xit"), ID_TEXIT );
	connect( file, SIGNAL(activated(int)), SLOT(commandCallback(int)) );

	settings = new QPopupMenu(this);
	settings->insertItem( SmallIconSet("configure"), i18n("&Preferences..."), ID_TPREFERENCES  );
	settings->insertSeparator();
	settings->insertItem( SmallIconSet("fileopen"), i18n("&Profiles..."), ID_TPROFILES );
	connect( settings, SIGNAL(activated(int)), SLOT(commandCallback(int)) );

	help = new QPopupMenu(this);
	help->insertItem( SmallIconSet("help"), i18n("&Help"), ID_THELP );
	help->insertSeparator();
	help->insertItem( i18n("&Report Bug..."), ID_TBUG );
	help->insertSeparator();
	help->insertItem( SmallIconSet("kit"), i18n("&About Kit..."), ID_TABOUT );
	help->insertItem( i18n("About &KDE..."), ID_TABOUTKDE );
	connect( help, SIGNAL(activated(int)), SLOT(commandCallback(int)) );
	
	treeMenu = new KPopupMenu(this);
	treeMenu->insertTitle(("Howdy Doody"));
	treeMenu->insertItem( i18n("Set nickname..."), ID_TNICK );
	treeMenu->insertSeparator();
	treeMenu->insertItem( SmallIconSet("mail_send"), i18n("Open chat..."), ID_TMESSAGE );
	treeMenu->insertItem( SmallIconSet("kit_info"), i18n("Information"), ID_TINFO );
	treeMenu->insertItem( SmallIconSet("find"), i18n("Directory"), ID_TDIRECTORY);
	treeMenu->insertSeparator();
	treeMenu->insertItem( i18n("Delete Buddy..."), ID_TDELBUDDY );
	treeMenu->insertSeparator();
	treeMenu->insertItem( i18n("Add Group..."), ID_TADDGROUP );
	connect( treeMenu, SIGNAL(activated(int)), this, SLOT(clickedTreeMenu(int)) );

	treeGroupMenu = new KPopupMenu(this);
	treeGroupMenu->insertTitle(("Howdy Doody"));
	treeGroupMenu->insertItem( i18n("Add Buddy..."), ID_TADDBUDDY );
	treeGroupMenu->insertSeparator();
	treeGroupMenu->insertItem( i18n("Add Group..."), ID_TADDGROUP );
	connect( treeGroupMenu, SIGNAL(activated(int)), this, SLOT(clickedTreeMenu(int)) );

	treeEmptyMenu = new KPopupMenu(this);
	treeEmptyMenu->insertItem( i18n("Add Group..."), ID_TADDGROUP );
	connect( treeEmptyMenu, SIGNAL(activated(int)), this, SLOT(clickedTreeMenu(int)) );

	dockMenu->clear();
	dockMenu->setTitle(profile->data()->name);
	dockMenu->insertItem( SmallIconSet("kit_online"), i18n("Online"), ID_TONLINE );
	dockMenu->insertItem( SmallIconSet("kit_away"), i18n("Away..."), ID_TAWAY );
	dockMenu->insertItem( SmallIconSet("kit_offline"), i18n("Offline"), ID_TOFFLINE );
	dockMenu->insertSeparator();
	dockMenu->insertItem( SmallIconSet("exit"), i18n("Exit"), ID_TEXIT );
	connect( dockMenu, SIGNAL(activated(int)), this, SLOT(commandCallback(int)) );
	
	menuBar()->insertItem(i18n("&File"), file );
	menuBar()->insertItem(i18n("&Settings"), settings );
	menuBar()->insertItem(i18n("&Help"), help );

	menuBar()->show();
}
QString TWindow::doAway(void)
{
	QString result = profile->execAwayPicker();
	return result;
}
void TWindow::addBuddyToTree(void)
{
	TBuddy newBuddy;
	// if this item has a parent, then it's not a group
	QListViewItem *item = treelist->selectedItem();
	if(!item || item->parent())
	{
		KMessageBox::error(0, i18n("Select a group first."));
		return;
	}
	newBuddy.group = profile->data()->names.getNumGroup(item->text(COL_NAME));
	// now get the name from the user
	KLineEditDlg nameDialog(i18n("Enter the screen name to add."), QString::null, this);
	if(nameDialog.exec() != KLineEditDlg::Accepted) return;
	newBuddy.name = nameDialog.text();
	// warn if the add fails
	profile->data()->names = aim.buddyList();
	if(profile->data()->names.add(&newBuddy) == -1)
	{
		KMessageBox::error(0, i18n("You cannot add the same screen name twice."));
		return;
	}
	// now update
	profileUpdated();
}
void TWindow::delBuddyFromTree(void)
{
	QListViewItem *item = treelist->selectedItem();
	if(!item || !item->parent()) return;
	if(KMessageBox::warningYesNo(0, i18n("Are you sure you want to remove %1?").arg(item->text(COL_NAME))) == KMessageBox::No)
		return;
	profile->data()->names.del(item->text(COL_NAME));
	// now update
	profileUpdated();
}
void TWindow::addGroupToTree(void)
{
	TBuddy newBuddy;

	// get a name
	KLineEditDlg groupDialog(i18n("Name the new group."), QString::null, this);
	if(groupDialog.exec() != KLineEditDlg::Accepted) return;
	// save the group number, and check if it's valid
	profile->data()->names = aim.buddyList();
	newBuddy.group = profile->data()->names.addGroup(groupDialog.text());
	if(newBuddy.group == -1)
	{
		KMessageBox::error(0, i18n("You cannot add the same group twice."));
		return;
	}
	// get a name to start off the group
	KLineEditDlg nameDialog(i18n("Enter the screen name to add."), QString::null, this);
	if(nameDialog.exec() != KLineEditDlg::Accepted)
	{
		profile->data()->names.delGroup(newBuddy.group);
		return;
	}
	// now add, and check if it's valid
	newBuddy.name = nameDialog.text();
	if(profile->data()->names.add(&newBuddy) == -1)
	{
		profile->data()->names.delGroup(newBuddy.group);
		KMessageBox::error(0, i18n("You cannot add the same screen name twice."));
		return;
	}
	// now update
	profileUpdated();
}
TMessage &TWindow::messageWindow(const QString &name)
{
	QString normedName = tocNormalize(name);
	TMessage *targetWindow = messageWindows[ normedName ];
	if(!targetWindow)
	{
		targetWindow = new TMessage(name, normedName, profile->data()->name,
			profile->data()->logging, profile->data()->timestamping,
			profile->data()->icqMode,
			profile->data()->foregroundColor, profile->data()->backgroundColor,
			profile->data()->bold);
		messageWindows.insert(normedName, targetWindow);
		connect(targetWindow, SIGNAL(messageOut(QString, QString, bool)), this, SLOT(sendingIM(QString, QString, bool)) );
		connect(targetWindow, SIGNAL(warningOut(QString, bool)), this, SLOT(sendingWarning(QString, bool)) );
		connect(targetWindow, SIGNAL(getInfo(QString)), this, SLOT(getInfo(QString)) );
		connect(targetWindow, SIGNAL(awayPicker(TMessage &)), this, SLOT(awayPicker(TMessage &)) );
		connect(targetWindow, SIGNAL(closing(const QString &)), this, SLOT(closingTMessage(const QString &)) );
		connect(this, SIGNAL(globalAway(bool, const QString &)), targetWindow, SLOT(setAway(bool, const QString &)) );
		connect(this, SIGNAL(globalLogging(bool)), targetWindow, SLOT(setLogging(bool)) );
		connect(this, SIGNAL(globalTimestamping(bool)), targetWindow, SLOT(setTimestamping(bool)) );
		connect(this, SIGNAL(globalMyName(const QString &)), targetWindow, SLOT(setMyName(const QString &)) );
		connect(this, SIGNAL(globalColors(const QColor &, const QColor &, bool)), targetWindow, SLOT(setColors(const QColor &, const QColor &, bool)) );
		connect(this, SIGNAL(globalICQ(bool)), targetWindow, SLOT(setICQ(bool)) );
		if(awayMessage != QString::null) targetWindow->setAway(true, awayMessage);
	}
	return *targetWindow;
}
void TWindow::closeMessageWindows(void)
{
	QDictIterator<TMessage> cur(messageWindows);
	while(cur.current())
	{
		cur.current()->disconnect(this);
		cur.current()->hide();
		delete cur.current();
		++cur;
	}
	messageWindows.clear();
}

#include "twindow.moc"
