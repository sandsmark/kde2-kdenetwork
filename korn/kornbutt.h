
#ifndef SSK_KORNBUTT_H
#define SSK_KORNBUTT_H

#include <qtoolbutton.h>

class KMailDrop;
class KornSettings;
class KornBtnStyle;

class KornButton: public QToolButton
{
  Q_OBJECT

  public:

    KornButton( QWidget *parent, KMailDrop *box );

  public slots:

    // draws button with the number.
    void setNumber( int );
    // runs the click command associated with this mailbox.
    void runCommand(bool onlyIfUnread);

  protected slots:

    void disconnectMonitor();
    void monitorUpdated();

  protected:

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

  signals:

    void rightClick();
    void dying(KornButton *);

  private:

    KMailDrop *_box;
    int _lastNum;
	KornBtnStyle *_style;
};


//FIXME: move this class to a separate file

/** this class sets a head on top the buttons
 * which display the box monitors
 * This is usefull, if the last monitor is erased
 * to provide a clickable widget and a describing relation
 * to the buttons below
 */
class HeadButton: public QToolButton
{
  Q_OBJECT

public:
  HeadButton( QWidget *parent);

protected:

  void mousePressEvent(QMouseEvent *);
  
signals:

  void rightClick();

};

#endif
