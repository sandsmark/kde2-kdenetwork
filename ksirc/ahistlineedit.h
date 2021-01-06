#ifndef AHISTLINEEDIT_H
#define AHISTLINEEDIT_H

#include <klineedit.h>
#include <qevent.h>
#include <qstrlist.h>
#include <qkeycode.h>

class aHistLineEdit : public KLineEdit
{
Q_OBJECT;
public:
  aHistLineEdit(QWidget *parent = 0, const char *name = 0);

signals:
  void gotFocus();
  void lostFocus();
  void pasteText();
  void notTab();

protected:
  void processKeyEvent( QKeyEvent * );
  virtual void keyPressEvent( QKeyEvent * ) {}
  virtual void focusInEvent ( QFocusEvent * );
  virtual void focusOutEvent ( QFocusEvent * );
  virtual void mousePressEvent ( QMouseEvent * );

  virtual bool eventFilter( QObject *o, QEvent *e );

protected slots:
  void slot_insert( QString );

private:
  QStrList hist;
  int current;
  void ColourPickerPopUp();

};

#endif
