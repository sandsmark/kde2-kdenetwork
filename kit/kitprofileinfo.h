// kitprofileinfo.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef KITPROFILEINFO_H
#define KITPROFILEINFO_H

#include <kdialogbase.h>
#include <qlistbox.h>
#include <qbutton.h>

class KitProfileInfo : public KDialogBase
{
	Q_OBJECT

	public:
		KitProfileInfo(QWidget * = 0, const char * = 0);
		~KitProfileInfo();
		
		const QString &profileSelected(void);
	protected slots:
		void useClicked(void);
		void setClicked(void);
		void addClicked(void);
		void deleteClicked(void);
		virtual void slotOk(void);
		virtual void slotDefault(void);
		virtual void slotCancel(void);
	protected:
		void updateList(void);
	private:
		QListBox *profileList;
		QButton *useButton;
		QButton *setButton;
		QButton *addButton;
		QButton *deleteButton;
		QString retval;
};

#endif
