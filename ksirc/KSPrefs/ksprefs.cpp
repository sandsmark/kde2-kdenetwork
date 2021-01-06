#include "ksprefs.h"

#include "../config.h"
#include <qlayout.h>
#include <kapp.h>
#include <klocale.h>

extern KConfig *kConfig;

KSPrefs::KSPrefs(QWidget * parent, const char * name)
  : QTabDialog(parent, name)
{
  setCaption(i18n("kSirc Preferences"));

//  pTab = new QTabDialog(0x0, "prefstabs");
//  pTab->show();

  // Start Sub Dialog items.
  pGeneral = new general(this);
  pStart = new StartUp(this);
  pServerChannel = new ServerChannel(this);
  pMenu = new UserMenuRef(this);
  pFilters = new DefaultFilters(this);

  this->setCancelButton(i18n("Cancel"));
  this->setOkButton(i18n("OK"));
  this->addTab(pGeneral, i18n("&General"));
  this->addTab(pStart, i18n("&StartUp"));
  this->addTab(pServerChannel, i18n("Servers/&Channels"));
  this->addTab(pMenu, i18n("&User Menu"));
  this->addTab(pFilters, i18n("&Default Filters"));


  connect(this, SIGNAL(applyButtonPressed()),
	  this, SLOT(slot_apply()));
  connect(this, SIGNAL(cancelButtonPressed()),
	  this, SLOT(slot_cancel()));
  connect(this, SIGNAL(defaultButtonPressed()),
	  this, SLOT(slot_cancel()));
}

KSPrefs::~KSPrefs(){
//  delete pTab;
//  pTab = 0;
}

void KSPrefs::slot_apply()
{
  pGeneral->slot_apply();
  pStart->slot_apply();
  pServerChannel->slot_apply();
  pFilters->slot_apply();
  kConfig->sync();
  
  emit update();
  delete this;
}

void KSPrefs::slot_cancel()
{
  emit update();
  delete this;
}

void KSPrefs::hide()
{
  delete this;
}


#include "ksprefs.moc"
