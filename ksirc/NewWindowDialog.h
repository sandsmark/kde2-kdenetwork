#ifndef NEW_WINDOW_DIALOG_H
#define NEW_WINDOW_DIALOG_H

#include <kdialogbase.h>

class KHistoryCombo;

class NewWindowDialog : public KDialogBase
{
  Q_OBJECT

  public:

    NewWindowDialog(QWidget * parent = 0, const char * name = 0);
    ~NewWindowDialog();

  protected slots:

    void slotOk();

  signals:

    void openTopLevel(QString);

  private:

    KHistoryCombo * combo_;
};

#endif
