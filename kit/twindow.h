// twindow.h
// 
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef TWINDOW_H
#define TWINDOW_H

#include <kpopupmenu.h>
#include <kmainwindow.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlistview.h>
#include <qdict.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qprogressdialog.h>
#include <qvbox.h>

#include "kituserprofile.h"
#include "taim.h"
#include "tbuddylist.h"
#include "tdockwindow.h"
#include "tmessage.h"

// used several times
#define ID_TONLINE TAIM_ONLINE
#define ID_TAWAY TAIM_AWAY
#define ID_TOFFLINE TAIM_OFFLINE

class TWindow : public KMainWindow
{
	Q_OBJECT

	public:
		// This constructor is for normal use
		TWindow(KitUserProfile *, const char * = 0, WFlags = WDestructiveClose);
		// This one is for session restoration
		TWindow(const char * = 0, WFlags = WDestructiveClose);
		~TWindow();

		enum menuItems {ID_TOPEN = 3, ID_TEXIT, ID_TPREFERENCES, ID_TPROFILES, ID_TABOUT,
		                ID_TABOUTKDE, ID_TBUG, ID_THELP, ID_TMESSAGE, ID_TINFO, ID_TADDBUDDY,
		                ID_TADDGROUP, ID_TDELBUDDY, ID_TDIRECTORY, ID_TNICK, ID_TPASSCHANGE};

		inline KitUserProfile *userProfile(void) { return profile; };
	private:
		// starts up widgets
		// had to pull out of constructor, to account for session management
		void starsAndTheMoon(void);
	public slots:
		void commandCallback(int command);
	signals:
		void closing(void);
		// INTERNAL ONLY...
		// for TMessage
		void globalAway(bool, const QString &);
		void globalLogging(bool);
		void globalTimestamping(bool);
		void globalMyName(const QString &);
		void globalColors(const QColor &, const QColor &, bool);
		void globalICQ(bool);
	protected:
		void closeEvent(QCloseEvent *);
		// session management
		virtual void saveProperties(KConfig *);
		virtual void readProperties(KConfig *);
	protected slots:
		// tree functions
		void initTree(void);
		void updateTree(int);
		void selectedTree(QListViewItem *);
		void infoTree(QListViewItem *);
		void directoryTree(QListViewItem *);
		void nickTree(QListViewItem *);
		void clickedTree(QListViewItem *, const QPoint &, int);
		void clickedTreeMenu(int);
		// status
		void statusClicked(const QString &);
		void setStatus(int);
		// TMessage
		void receivedIM(QString, QString, bool);
		void sendingIM(QString, QString, bool);
		void getInfo(QString);
		void sendingWarning(QString, bool);
		void awayPicker(TMessage &);
		void closingTMessage(const QString &);
		// display info URLs
		void displayInfo(QString);
		// progress bar for aim
		void initProgress(int, int, QString);
		void updateProgress(int, QString);
		void endProgress(void);
		// error display for aim
		void displayError(QString);
		void displayWarning(int, QString);
		// configuration application
		void profileUpdated(void);
		// password
		void passwordChanged(void);
	private:
		void initMenu(void);
		void updateTreeItem(QListViewItem *item, TBuddy &currentBuddy, int listColumns);
		QString doAway(void);
		// tree helpers
		void addBuddyToTree(void);
		void delBuddyFromTree(void);
		void addGroupToTree(void);

		// TMessage handling
		TMessage &messageWindow(const QString &);
		void closeMessageWindows(void);
		QDict<TMessage> messageWindows;
		QString awayMessage;

		// main view
		QVBox *mainView;

		// widgets at top
		QPopupMenu *file, *settings, *help;
		// in the middle
		QListView *treelist;
		KPopupMenu *treeMenu, *treeGroupMenu, *treeEmptyMenu;
		QListViewItem *treeCurrent;
		// along the bottom
		QComboBox *statusButton;
		// KDE convenience
		TDockWindow *dockWindow;
		KPopupMenu *dockMenu;

		// connectivity
		TAim aim;
		QProgressDialog *progress;

		// configuration
		KitUserProfile *profile;

		QString newPassword;
};

#endif
