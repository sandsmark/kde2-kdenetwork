// kitprofileeditor.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef KITPROFILEEDITOR_H
#define KITPROFILEEDITOR_H

#include <kdialogbase.h>
#include <krestrictedline.h>
#include <keditcl.h>
#include <klineedit.h>
#include <kcolorbtn.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>

#include "kituserprofile.h"
#include "tedit.h"

class ProfileEditor : public KDialogBase
{
	Q_OBJECT

	public:
		ProfileEditor(KitUserProfile *, const char * = 0);
		~ProfileEditor();
	private:
		void initWidgets(void);
		KitUserProfile *profile;
		// Part 1
		KLineEdit *uName, *uPwd;
		KEdit *uInfo;
		QCheckBox *uSave;
		// Part 2
		QListBox *lGroup, *lBuddy;
		TBuddyList names;
		// Part 3
		QListBox *pPermit, *pDeny;
		QButtonGroup *pState;
		TBuddyList permit, deny;
		// Part 4
		QListBox *mAwayList;
		TEdit *mAway;
		QStringList awayMessages, awayMessageNames;
		// Part 5
		QCheckBox *aHide;
		QCheckBox *aIdle, *aClass, *aWarn, *aSignon, *aNick;
		QButtonGroup *aGrouping;
		// Part 6
		KColorButton *bForeground, *bBackground;
		QCheckBox *bBold;
		QCheckBox *bLog, *bTimestamp;
		QCheckBox *bICQ;
		// Part 7
		QCheckBox *nKeepAlive;
		QCheckBox *nAutoConnect;
		QCheckBox *nUseCustom;
		QLineEdit *nServer, *nAuth;
		KRestrictedLine *nServerPort, *nAuthPort;
	protected slots:
		virtual void accept(void);
		virtual void reject(void);
		void lGroupHighlighted(const QString &);
		void mAwayListHighlighted(int);
		void mAwayTextChanged(void);
		// Here's the dark side of a large dialog like this
		void lAddGroupClicked(void);
		void lRenGroupClicked(void);
		void lDelGroupClicked(void);
		void lAddBuddyClicked(void);
		void lDelBuddyClicked(void);
		void pAddPermitClicked(void);
		void pDelPermitClicked(void);
		void pAddDenyClicked(void);
		void pDelDenyClicked(void);
		void mRenAwayClicked(void);
		void mAddAwayClicked(void);
		void mDelAwayClicked(void);
		void nUseCustomClicked(void);
};

#endif
