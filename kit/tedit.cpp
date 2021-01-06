// tedit.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include "tedit.h"

TEdit::TEdit(QWidget *parent, const char *name)
	: KEdit(parent, name)
{
}
void TEdit::keyPressEvent(QKeyEvent *a)
{
	if( (a->key() == Qt::Key_Return) | (a->key() == Qt::Key_Enter) )
		emit returnPressed();
	else
		KEdit::keyPressEvent(a);
}

#include "tedit.moc"
