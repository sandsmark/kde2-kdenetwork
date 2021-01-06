// kitdirectorysearch.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef KITDIRECTORYSEARCH_H
#define KITDIRECTORYSEARCH_H

#include <kdialogbase.h>
#include <klineedit.h>
#include <qcheckbox.h>

#include "taim.h"

class KitDirectorySearch : public KDialogBase
{
	Q_OBJECT

	public:
		KitDirectorySearch(const char * = 0);
		~KitDirectorySearch();

		inline const TAim::directory &result() { return dir; };
	private:
		TAim::directory dir;

		KLineEdit *uDirFirstName, *uDirLastName, *uDirMiddleName;
		KLineEdit *uDirMaidenName;
		KLineEdit *uDirCity, *uDirState, *uDirCountry;
		KLineEdit *uDirEmail;
		QCheckBox *uDirWebSearches;
	protected slots:
		virtual void accept(void);
		virtual void reject(void);
};

#endif
