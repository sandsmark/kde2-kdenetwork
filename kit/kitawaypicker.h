// kitawaypicker.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef KITAWAYPICKER_H
#define KITAWAYPICKER_H

#include <kdialogbase.h>

#include "kituserprofile.h"
#include "tedit.h"

class AwayPicker : public KDialogBase
{
	Q_OBJECT

	public:
		AwayPicker(KitUserProfile *, const char * = 0);
		~AwayPicker();

		inline const QString &message() { return chosen; };
	private:
		void initWidgets(void);
		KitUserProfile *profile;
		QString chosen;
		// Part 4
		QListBox *mAwayList;
		TEdit *mAway;
		QStringList awayMessages, awayMessageNames;
	protected slots:
		virtual void accept(void);
		virtual void reject(void);
		void mAwayListHighlighted(int);
		void mAwayTextChanged(void);
		// Here's the dark side of a large dialog like this
		void mRenAwayClicked(void);
		void mAddAwayClicked(void);
		void mDelAwayClicked(void);
};

#endif
