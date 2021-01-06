#include <kconfig.h>
#include <klocale.h>
#include <kcombobox.h>
#include <qhbox.h>
#include <qlabel.h>

#include "NewWindowDialog.h"

extern KConfig *kConfig;

NewWindowDialog::NewWindowDialog(QWidget * parent, const char * name)
  : KDialogBase(parent, name, true, i18n("New Window For"), Ok|Cancel, Ok, true)
{
  QHBox * w = makeHBoxMainWidget();

  QLabel * l = new QLabel(i18n("C&hannel/Nick:"), w);

  combo_ = new KHistoryCombo(w);

  l->setBuddy(combo_);

  connect(
      combo_, SIGNAL(activated(const QString &)),
      combo_, SLOT(addToHistory(const QString &)));

  KConfigGroupSaver saver(kConfig, "Recent");
  combo_->setHistoryItems(kConfig->readListEntry("Channels"));

  if (combo_->count() > 0)
    combo_->setEditText(combo_->text(0));
}

NewWindowDialog::~NewWindowDialog()
{
  KConfigGroupSaver saver(kConfig, "Recent");
  kConfig->writeEntry("Channels", combo_->historyItems());
}

  void
NewWindowDialog::slotOk()
{
  emit(openTopLevel(combo_->currentText()));
  KDialogBase::slotOk();
}

#include "NewWindowDialog.moc"

