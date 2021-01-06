#ifndef __ksprefs_h
#define __ksprefs_h

#include <qwidget.h>
#include <qobject.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qtabdialog.h>

#include "general.h"
#include "startup.h"
#include "serverchannel.h"
#include "UserMenuRef.h"
#include "defaultfilters.h"

struct KSPrefsSettings
{
  QString nick;
  QString real_name;
  QString startup_commands;
};

class KSPrefs : public QTabDialog
{
  Q_OBJECT
public:
  KSPrefs(QWidget * parent=0, const
	  char * name=0);

  virtual ~KSPrefs();

  void hide();

signals:
  void update();
  void settings(KSPrefsSettings *);

protected slots:
  void slot_apply();
  void slot_cancel();

private:
//  QTabDialog *pTab;
  general *pGeneral;
  StartUp *pStart;
  UserMenuRef *pMenu;
  ServerChannel *pServerChannel;
  DefaultFilters *pFilters;

};
#endif
