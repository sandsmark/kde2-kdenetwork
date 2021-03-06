/////////////////////////////////////////////////////////////////////////////
//
// A QGroupBox like box with title, optional a checkbox and a peer window 
// inside
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __KGROUPBOX__H__
#define __KGROUPBOX__H__

#include <qframe.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

class KGroupBoxBase : public QFrame {
  Q_OBJECT
public:
  KGroupBoxBase(QWidget *parent = 0, const char *name = 0);
  inline QWidget *peer() const { return _peer; }

  void setTitleWidget(QWidget *title);

  virtual void resizeEvent(QResizeEvent *);
  virtual QSize minimumSize() const;
  virtual QSize sizeHint() const;
  virtual void show();

protected:
  void initBox();

  QRect frect;
  QWidget *_title;
  QWidget *_peer;
};

class KGroupBox : public KGroupBoxBase {
  Q_OBJECT
public:
  KGroupBox(const QString &title, QWidget *parent = 0, const char *name = 0);

  void setTitle(const QString &title);
};


class KCheckGroupBox : public KGroupBoxBase {
  Q_OBJECT
public:
  KCheckGroupBox(const QString & title, QWidget *parent = 0, const char *name = 0);

  void setTitle(const QString & title);
  bool isChecked();
  void setChecked(bool);

private slots:
  void slot_toggled(bool);

signals:
  void toggled(bool);

private:
  QCheckBox *cbox;
};


class KRadioGroupBox : public KGroupBoxBase {
  Q_OBJECT
public:
  KRadioGroupBox(const QString & title, QWidget *parent = 0, const char *name = 0);

  void setTitle(const QString & title);
  bool isChecked();

private slots:
  void slot_toggled(bool);

signals:
  void toggled(bool);

private:
  QRadioButton *rb;
};

#endif
