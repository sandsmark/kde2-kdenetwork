// tedit.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef TEDIT_H
#define TEDIT_H

#include <keditcl.h>

class TEdit : public KEdit
{
	Q_OBJECT

	public:
		TEdit(QWidget * = 0, const char * = 0);
	signals:
		void returnPressed(void);
	protected:
		void keyPressEvent(QKeyEvent *);
};

#endif
