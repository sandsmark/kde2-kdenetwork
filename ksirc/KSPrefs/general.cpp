/**********************************************************************

        --- Qt Architect generated file ---

        File: general.cpp
        Last generated: Sun Jul 26 16:28:55 1998

 *********************************************************************/

#include "general.h"
#include "../config.h"
#include <kconfig.h>
#include <kfiledialog.h>
#include <knuminput.h>

extern KConfig *kConfig;
extern global_config *kSircConfig;

#undef Inherited
#define Inherited generalData

general::general
(
        QWidget* parent,
        const char* name
)
        :
        Inherited( parent, name )
{
  kConfig->setGroup("General");

  kSircConfig->AutoCreateWin = kConfig->readBoolEntry("AutoCreateWin", false);
  CB_AutoCreateWin->setChecked(kSircConfig->AutoCreateWin);

  kSircConfig->BeepNotify = kConfig->readBoolEntry("BeepNotify", true);
  CB_BeepNotify->setChecked(kSircConfig->BeepNotify);

  kSircConfig->NickCompletion = kConfig->readBoolEntry("NickCompletion", true);
  CB_NickCompletion->setChecked(kSircConfig->NickCompletion);

  kSircConfig->ColourPicker = kConfig->readBoolEntry("ColourPicker", true);
  CB_ColourPicker->setChecked(kSircConfig->ColourPicker);

  kSircConfig->AutoRejoin = kConfig->readBoolEntry("AutoRejoin", true);
  CB_AutoRejoin->setChecked(kSircConfig->AutoRejoin);

  kSircConfig->DisplayMode = kConfig->readNumEntry("DisplayMode", 0);
  if (kSircConfig->DisplayMode == 1)
      CB_MDIMode->setChecked(true);

  CB_DisplayTopic->setChecked(kSircConfig->DisplayTopic);

  KIL_WindowLength->setValue(kSircConfig->WindowLength);


  kSircConfig->timestamp = kConfig->readBoolEntry("TimeStamp", false);
  CB_TimeStamp->setChecked(kSircConfig->timestamp);

  // not yet =P
//*  CB_BackgroundPix->hide();
  CB_BackgroundPix->setChecked(kConfig->readNumEntry("BackgroundPix", false));
  kSircConfig->BackgroundPix = kConfig->readNumEntry("BackgroundPix", false);
  SLE_BackgroundFile->setText(kConfig->readEntry("BackgroundFile"));
//  SLE_BackgroundFile->hide();
  kSircConfig->BackgroundFile = kConfig->readEntry("BackgroundFile");
  connect(PB_BackgroundBrowse, SIGNAL(clicked()),
          this, SLOT(slot_openBrowser()));
  //PB_BackgroundBrowse->hide();
}

general::~general()
{
}

void general::slot_openBrowser()
{
  KFileDialog *FileDialog = new KFileDialog( ".", "*.gif", 0,
                            "filedialog", true );
  connect(FileDialog, SIGNAL(fileSelected(const char*)),
          this, SLOT(slot_setBackgroundFile(const char*)));
  FileDialog->show();
}

void general::slot_setBackgroundFile(const char* filename)
{
  SLE_BackgroundFile->setText( filename );
}

void general::slot_apply()
{
  kSircConfig->AutoCreateWin = CB_AutoCreateWin->isChecked();
  kSircConfig->BeepNotify = CB_BeepNotify->isChecked();
  kSircConfig->NickCompletion = CB_NickCompletion->isChecked();
  kSircConfig->ColourPicker = CB_ColourPicker->isChecked();
  kSircConfig->AutoRejoin = CB_AutoRejoin->isChecked();
  kSircConfig->BackgroundPix = CB_BackgroundPix->isChecked();
  kSircConfig->transparent = CB_BackgroundPix->isChecked();
  kSircConfig->BackgroundFile = SLE_BackgroundFile->text();
  kSircConfig->DisplayTopic = CB_DisplayTopic->isChecked();
  kSircConfig->WindowLength = KIL_WindowLength->value();
  kSircConfig->timestamp = CB_TimeStamp->isChecked();

  if (CB_MDIMode->isChecked())
      kSircConfig->DisplayMode = 1;
  else
      kSircConfig->DisplayMode = 0;

  if(kSircConfig->WindowLength < 25)
      kSircConfig->WindowLength = 25;

  kConfig->setGroup("General");
  kConfig->writeEntry("AutoCreateWin", kSircConfig->AutoCreateWin);
  kConfig->writeEntry("BeepNotify", kSircConfig->BeepNotify);
  kConfig->writeEntry("NickCompletion", kSircConfig->NickCompletion);
  kConfig->writeEntry("ColourPicker", kSircConfig->ColourPicker);
  kConfig->writeEntry("AutoRejoin", kSircConfig->AutoRejoin);
  kConfig->writeEntry("BackgroundPix", kSircConfig->BackgroundPix);
  kConfig->writeEntry("transparent", kSircConfig->transparent);
  kConfig->writeEntry("BackgroundFile", kSircConfig->BackgroundFile);
  kConfig->writeEntry("DisplayTopic", kSircConfig->DisplayTopic);
  kConfig->writeEntry("DisplayMode", kSircConfig->DisplayMode);
  kConfig->writeEntry("WindowLength", kSircConfig->WindowLength);
  kConfig->writeEntry("TimeStamp", kSircConfig->timestamp);
  kConfig->writeEntry("DisplayMode", kSircConfig->DisplayMode);
}
#include "general.moc"
