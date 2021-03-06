// kmcomposewin.cpp
// Author: Markus Wuebben <markus.wuebben@kde.org>
// This code is published under the GPL.

#include <qdir.h>
#undef GrayScale
#undef Color
#include <qprinter.h>
#include <qcombobox.h>
#include <qdragobject.h>
#include <qlistview.h>
#include <qcombobox.h>

#include "kmcomposewin.h"
#include "kmmessage.h"
#include "kmmsgbase.h"
#include "kmmsgpart.h"
#include "kmsender.h"
#include "kmidentity.h"
#include "kfileio.h"
#include "kbusyptr.h"
#include "kmmsgpartdlg.h"
#include <kpgp.h>
#include "kmaddrbookdlg.h"
#include "kmaddrbook.h"
#include "kmidentity.h"
#include "kmfolder.h"

#include <kabapi.h>
#include <kaction.h>
#include <kcharsets.h>
#include <kcompletion.h>
#include <kcompletionbox.h>
#include <kcursor.h>
#include <kstdaction.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>
#include <kdebug.h>

#include "kmmainwin.h"
#include "configuredialog.h"
#include "kfileio.h"
#include "kmreaderwin.h"

#include <assert.h>
#include <kapp.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kstdaccel.h>
#include <mimelib/mimepp.h>
#include <kfiledialog.h>
#include <kwin.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kurldrag.h>

#include <kspell.h>

#include <qtabdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlist.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qcursor.h>
#include <qbuffer.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <klocale.h>
#include <ktempfile.h>
#include <fcntl.h>

#include "kmglobal.h"
#include "kmrecentaddr.h"

#include "kmcomposewin.moc"

//-----------------------------------------------------------------------------
KMComposeWin::KMComposeWin(KMMessage *aMsg, QString id )
  : KMTopLevelWidget (), MailComposerIface(),
  mMainWidget(this),
  mIdentity(&mMainWidget), mTransport(true, &mMainWidget),
  mEdtFrom(this,false,&mMainWidget), mEdtReplyTo(this,false,&mMainWidget),
  mEdtTo(this,true,&mMainWidget),  mEdtCc(this,true,&mMainWidget),
  mEdtBcc(this,true,&mMainWidget),
  mEdtSubject(this,false,&mMainWidget, "subjectLine"),
  mLblIdentity(&mMainWidget), mLblTransport(&mMainWidget),
  mLblFrom(&mMainWidget), mLblReplyTo(&mMainWidget), mLblTo(&mMainWidget),
  mLblCc(&mMainWidget), mLblBcc(&mMainWidget), mLblSubject(&mMainWidget),
  mBtnIdentity(i18n("Sticky"),&mMainWidget),
  mBtnTransport(i18n("Sticky"),&mMainWidget),
  mBtnTo("...",&mMainWidget), mBtnCc("...",&mMainWidget),
  mBtnBcc("...",&mMainWidget),  mBtnFrom("...",&mMainWidget),
  mBtnReplyTo("...",&mMainWidget),
  mId( id )

{
  //setWFlags( WType_TopLevel | WStyle_Dialog );
  mDone = false;
  mGrid = NULL;
  mAtmListBox = NULL;
  mAtmList.setAutoDelete(TRUE);
  mAtmTempList.setAutoDelete(TRUE);
  mAutoDeleteMsg = FALSE;
  mFolder = NULL;
  mEditor = new KMEdit(&mMainWidget, this);
  disableBreaking = false;

  mSpellCheckInProgress=FALSE;

  setCaption( i18n("Composer") );
  setMinimumSize(200,200);

  mBtnIdentity.setFocusPolicy(QWidget::NoFocus);
  mBtnTransport.setFocusPolicy(QWidget::NoFocus);
  mBtnTo.setFocusPolicy(QWidget::NoFocus);
  mBtnCc.setFocusPolicy(QWidget::NoFocus);
  mBtnBcc.setFocusPolicy(QWidget::NoFocus);
  mBtnFrom.setFocusPolicy(QWidget::NoFocus);
  mBtnReplyTo.setFocusPolicy(QWidget::NoFocus);

  mAtmListBox = new QListView(&mMainWidget, "mAtmListBox");
  mAtmListBox->setFocusPolicy(QWidget::NoFocus);
  mAtmListBox->addColumn(i18n("Name"), 200);
  mAtmListBox->addColumn(i18n("Size"), 80);
  mAtmListBox->addColumn(i18n("Encoding"), 120);
  mAtmListBox->addColumn(i18n("Type"), 150);
  connect(mAtmListBox,
	  SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
	  SLOT(slotAttachPopupMenu(QListViewItem *, const QPoint &, int)));

  readConfig();
  setupStatusBar();
  setupEditor();
  setupActions();
  applyMainWindowSettings(kapp->config(), "Composer");
  toolbarAction->setChecked(!toolBar()->isHidden());
  statusbarAction->setChecked(!statusBar()->isHidden());

  connect(&mEdtSubject,SIGNAL(textChanged(const QString&)),
	  SLOT(slotUpdWinTitle(const QString&)));
  connect(&mBtnTo,SIGNAL(clicked()),SLOT(slotAddrBookTo()));
  connect(&mBtnCc,SIGNAL(clicked()),SLOT(slotAddrBookCc()));
  connect(&mBtnBcc,SIGNAL(clicked()),SLOT(slotAddrBookBcc()));
  connect(&mBtnReplyTo,SIGNAL(clicked()),SLOT(slotAddrBookReplyTo()));
  connect(&mBtnFrom,SIGNAL(clicked()),SLOT(slotAddrBookFrom()));
  connect(&mIdentity,SIGNAL(activated(int)),SLOT(slotIdentityActivated(int)));

  connect(&mEdtTo,SIGNAL(completionModeChanged(KGlobalSettings::Completion)),
          SLOT(slotCompletionModeChanged(KGlobalSettings::Completion)));
  connect(&mEdtCc,SIGNAL(completionModeChanged(KGlobalSettings::Completion)),
          SLOT(slotCompletionModeChanged(KGlobalSettings::Completion)));
  connect(&mEdtBcc,SIGNAL(completionModeChanged(KGlobalSettings::Completion)),
          SLOT(slotCompletionModeChanged(KGlobalSettings::Completion)));
  connect(&mEdtReplyTo,SIGNAL(completionModeChanged(KGlobalSettings::Completion)),
          SLOT(slotCompletionModeChanged(KGlobalSettings::Completion)));
  connect(&mEdtFrom,SIGNAL(completionModeChanged(KGlobalSettings::Completion)),
          SLOT(slotCompletionModeChanged(KGlobalSettings::Completion)));

  mMainWidget.resize(480,510);
  setCentralWidget(&mMainWidget);
  rethinkFields();

  if (useExtEditor) {
    mEditor->setExternalEditor(true);
    mEditor->setExternalEditorPath(mExtEditor);
  }

  // As family may change with charset, we must save original settings
  mSavedEditorFont=mEditor->font();

  mMsg = NULL;
  if (aMsg)
    setMsg(aMsg);

  mEdtTo.setFocus();
  mDone = true;
}


//-----------------------------------------------------------------------------
KMComposeWin::~KMComposeWin()
{
  writeConfig();
  if (mFolder && mMsg)
  {
    mAutoDeleteMsg = FALSE;
    mFolder->addMsg(mMsg);
    emit messageQueuedOrDrafted();
  }
  if (mAutoDeleteMsg && mMsg) delete mMsg;
  QMap<KIO::Job*, atmLoadData>::Iterator it = mapAtmLoadData.begin();
  while ( it != mapAtmLoadData.end() )
  {
    KIO::Job *job = it.key();
    mapAtmLoadData.remove( it );
    job->kill();
    it = mapAtmLoadData.begin();
  }
}

//-----------------------------------------------------------------------------
void KMComposeWin::send(int how)
{
  switch (how) {
    case 1:
      slotSendNow();
      break;
    default:
    case 0:
      // TODO: find out, what the default send method is and send it this way
    case 2:
      slotSendLater();
      break;
  }
}

//-----------------------------------------------------------------------------
void KMComposeWin::addAttachment(KURL url,QString /*comment*/)
{
  addAttach(url);
}

//-----------------------------------------------------------------------------
void KMComposeWin::addAttachment(const QString &name,
                                 const QCString &cte,
                                 const QByteArray &data,
                                 const QCString &type,
                                 const QCString &subType,
                                 const QCString &paramAttr,
                                 const QString &paramValue,
                                 const QCString &contDisp)
{
  if (!data.isEmpty()) {
    KMMessagePart *msgPart = new KMMessagePart;
    msgPart->setName(name);
    msgPart->setCteStr(cte);
    msgPart->setBodyEncodedBinary(data);
    msgPart->setTypeStr(type);
    msgPart->setSubtypeStr(subType);
    msgPart->setParameter(paramAttr,paramValue);
    msgPart->setContentDisposition(contDisp);
    addAttach(msgPart);
  }
}

//-----------------------------------------------------------------------------
void KMComposeWin::setBody(QString body)
{
  // TODO: implement KMComposeWin::setBody().
  kdDebug() << "KMComposeWin::setBody() isn't implemented yet." << endl;
}

//-----------------------------------------------------------------------------
void KMComposeWin::makeDescriptiveNames(QStringList &encodings)
{
  QStringList encodingNames = KGlobal::charsets()->availableEncodingNames();
  QMap<QTextCodec*, QString> encodingMap;
  for (QStringList::Iterator it = encodingNames.begin();
    it != encodingNames.end(); it++)
  {
    QTextCodec *codec = KMMsgBase::codecForName((*it).latin1());
    if (codec) encodingMap.insert(codec, *it);
  }
  for (QStringList::Iterator it = encodings.begin();
    it != encodings.end(); it++)
  {
    QTextCodec *codec = KMMsgBase::codecForName((*it).latin1());
    QMap<QTextCodec*, QString>::Iterator it2 = encodingMap.find(codec);
    if (it2 == encodingMap.end())
      (*it) = KGlobal::charsets()->languageForEncoding(*it)
      + " ( " + *it + " )";
    else
      (*it) = KGlobal::charsets()->languageForEncoding(*it2)
      + " ( " + *it + " )";
  }
}


//-----------------------------------------------------------------------------
bool KMComposeWin::event(QEvent *e)
{
  if (e->type() == QEvent::ApplicationPaletteChange)
  {
     readColorConfig();
     return true;
  }
  return KMTopLevelWidget::event(e);
}


//-----------------------------------------------------------------------------
void KMComposeWin::readColorConfig(void)
{
  KConfig *config = kapp->config();
  KConfigGroupSaver saver(config, "Reader");
  QColor c1=QColor(kapp->palette().normal().text());
  QColor c4=QColor(kapp->palette().normal().base());

  if (!config->readBoolEntry("defaultColors",TRUE)) {
    foreColor = config->readColorEntry("ForegroundColor",&c1);
    backColor = config->readColorEntry("BackgroundColor",&c4);
  }
  else {
    foreColor = c1;
    backColor = c4;
  }

  // Color setup
  mPalette = kapp->palette();
  QColorGroup cgrp  = mPalette.normal();
  cgrp.setColor( QColorGroup::Base, backColor);
  cgrp.setColor( QColorGroup::Text, foreColor);
  mPalette.setNormal(cgrp);
  mPalette.setDisabled(cgrp);
  mPalette.setActive(cgrp);
  mPalette.setInactive(cgrp);

  mEdtTo.setPalette(mPalette);
  mEdtFrom.setPalette(mPalette);
  mEdtCc.setPalette(mPalette);
  mEdtSubject.setPalette(mPalette);
  mEdtReplyTo.setPalette(mPalette);
  mEdtBcc.setPalette(mPalette);
  mTransport.setPalette(mPalette);
  mEditor->setPalette(mPalette);
}

//-----------------------------------------------------------------------------
void KMComposeWin::readConfig(void)
{
  KConfig *config = kapp->config();
  QCString str;
  //  int w, h,
  int maxTransportItems;

  KConfigGroupSaver saver(config, "Composer");

  str = config->readEntry("charset", "").latin1();
  if (str.isEmpty() || str=="default")
    mDefCharset = defaultCharset();
  else
  {
    mDefCharset = str;
    if ( !KGlobal::charsets()->isAvailable(
      KGlobal::charsets()->charsetForEncoding(mDefCharset)) )
        mDefCharset = "default";
  }

  kdDebug(5006) << "Default charset: " << mDefCharset << endl;

  mForceReplyCharset = config->readBoolEntry("force-reply-charset", false );
  mAutoSign = config->readEntry("signature","auto") == "auto";
  mShowHeaders = config->readNumEntry("headers", HDR_STANDARD);
  mWordWrap = config->readNumEntry("word-wrap", 1);
  mLineBreak = config->readNumEntry("break-at", 78);
  mBtnIdentity.setChecked(config->readBoolEntry("sticky-identity", false));
  if (mBtnIdentity.isChecked())
    mId = config->readEntry("previous-identity", mId );
  mBtnTransport.setChecked(config->readBoolEntry("sticky-transport", false));
  mTransportHistory = config->readListEntry("transport-history");
  maxTransportItems = config->readNumEntry("max-transport-items",10);

  if ((mLineBreak == 0) || (mLineBreak > 78))
    mLineBreak = 78;
  if (mLineBreak < 60)
    mLineBreak = 60;
  mAutoPgpSign = config->readNumEntry("pgp-auto-sign", 0);
  mConfirmSend = config->readBoolEntry("confirm-before-send", false);

  int mode = config->readNumEntry("Completion Mode",
                                  KGlobalSettings::completionMode() );
  mEdtFrom.setCompletionMode( (KGlobalSettings::Completion) mode );
  mEdtReplyTo.setCompletionMode( (KGlobalSettings::Completion) mode );
  mEdtTo.setCompletionMode( (KGlobalSettings::Completion) mode );
  mEdtCc.setCompletionMode( (KGlobalSettings::Completion) mode );
  mEdtBcc.setCompletionMode( (KGlobalSettings::Completion) mode );

  readColorConfig();

  { // area for config group "General"
    KConfigGroupSaver saver(config, "General");
    mExtEditor = config->readEntry("external-editor", DEFAULT_EDITOR_STR);
    useExtEditor = config->readBoolEntry("use-external-editor", FALSE);

    int headerCount = config->readNumEntry("mime-header-count", 0);
    mCustHeaders.clear();
    mCustHeaders.setAutoDelete(true);
    for (int i = 0; i < headerCount; i++) {
      QString thisGroup;
      _StringPair *thisItem = new _StringPair;
      thisGroup.sprintf("Mime #%d", i);
      KConfigGroupSaver saver(config, thisGroup);
      thisItem->name = config->readEntry("name", "");
      if ((thisItem->name).length() > 0) {
	thisItem->value = config->readEntry("value", "");
	mCustHeaders.append(thisItem);
      } else {
	delete thisItem;
      }
    }
  }

  { // area fo config group "Fonts"
    KConfigGroupSaver saver(config, "Fonts");
    mUnicodeFont = config->readBoolEntry("unicodeFont",FALSE);
    if (!config->readBoolEntry("defaultFonts",TRUE)) {
      mBodyFont = QFont("helvetica");
      mBodyFont = config->readFontEntry("body-font", &mBodyFont);
      QString bodyFamily = mBodyFont.family();
      int pos = bodyFamily.find("-");
      // We ignore the foundary, otherwise we can't set the charset
      if (pos != -1)
	mBodyFont.setFamily(bodyFamily.right(bodyFamily.length()-pos-1));
    }
    else
      mBodyFont = KGlobalSettings::generalFont();
    if (mEditor) mEditor->setFont(mBodyFont);
  }

  { // area fo config group "Fonts"
    KConfigGroupSaver saver(config, "Geometry");
    QSize defaultSize(480,510);
    QSize siz = config->readSizeEntry("composer", &defaultSize);
    if (siz.width() < 200) siz.setWidth(200);
    if (siz.height() < 200) siz.setHeight(200);
    resize(siz);
  }

  mIdentity.clear();
  mIdentity.insertStringList( KMIdentity::identities() );
  for (int i=0; i < mIdentity.count(); ++i)
    if (mIdentity.text(i) == mId) {
      mIdentity.setCurrentItem(i);
      break;
    }

  if (!mBtnTransport.isChecked() || mTransportHistory.isEmpty()) {
    QString curTransport = kernel->msgSender()->transportString();
    mTransportHistory.remove( curTransport );
    mTransportHistory.prepend( curTransport );
  }
  while (mTransportHistory.count() > (uint)maxTransportItems)
    mTransportHistory.remove( mTransportHistory.last());
  mTransport.insertStringList( mTransportHistory );
}


//-----------------------------------------------------------------------------
void KMComposeWin::writeConfig(void)
{
  KConfig *config = kapp->config();
  QString str;

  {
    KConfigGroupSaver saver(config, "Composer");
    config->writeEntry("signature", mAutoSign?"auto":"manual");
    config->writeEntry("headers", mShowHeaders);
    config->writeEntry("sticky-transport", mBtnTransport.isChecked());
    config->writeEntry("sticky-identity", mBtnIdentity.isChecked());
    config->writeEntry("previous-identity", mIdentity.currentText() );
    mTransportHistory.remove(mTransport.currentText());
    mTransportHistory.prepend(mTransport.currentText());
    config->writeEntry("transport-history", mTransportHistory );
  }

  {
    KConfigGroupSaver saver(config, "Geometry");
    config->writeEntry("composer", size());

    saveMainWindowSettings(config, "Composer");
    config->sync();
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::deadLetter(void)
{
  if (!mMsg) return;

  // This method is called when KMail crashed, so we better use as
  // basic functions as possible here.
  applyChanges();
  QCString msgStr = mMsg->asString();
  QCString fname = getenv("HOME");
  fname += "/dead.letter";
  // Security: the file is created in the user's home directory, which
  // might be readable by other users. So the file only gets read/write
  // permissions for the user himself. Note that we create the file with
  // correct permissions, we do not set them after creating the file!
  // (dnaber, 2000-02-27):
  int fd = open(fname, O_CREAT|O_APPEND|O_WRONLY, S_IWRITE|S_IREAD);
  if (fd != -1)
  {
    QCString startStr = "From " + mMsg->fromEmail() + " " + mMsg->dateShortStr() + "\n";
    ::write(fd, startStr, startStr.length());
    ::write(fd, msgStr, msgStr.length());
    ::write(fd, "\n", 1);
    ::close(fd);
    fprintf(stderr,"appending message to ~/dead.letter\n");
  }
  else perror("cannot open ~/dead.letter for saving the current message");
}



//-----------------------------------------------------------------------------
void KMComposeWin::slotView(void)
{
  if (!mDone)
    return; // otherwise called from rethinkFields during the construction
            // which is not the intended behavior
  int id;

  //This sucks awfully, but no, I cannot get an activated(int id) from
  // actionContainer()
  if (!sender()->isA("KToggleAction"))
    return;
  KToggleAction *act = (KToggleAction *) sender();

  if (act == allFieldsAction)
    id = 0;
  else if (act == identityAction)
    id = HDR_IDENTITY;
  else if (act == transportAction)
    id = HDR_TRANSPORT;
  else if (act == fromAction)
    id = HDR_FROM;
  else if (act == replyToAction)
    id = HDR_REPLY_TO;
  else if (act == toAction)
    id = HDR_TO;
  else if (act == ccAction)
    id = HDR_CC;
  else  if (act == bccAction)
    id = HDR_BCC;
  else if (act == subjectAction)
    id = HDR_SUBJECT;
   else
   {
     id = 0;
     kdDebug(5006) << "Something is wrong (Oh, yeah?)" << endl;
     return;
   }

  // sanders There's a bug here this logic doesn't work if no
  // fields are shown and then show all fields is selected.
  // Instead of all fields being shown none are.
  if (!act->isChecked())
  {
    // hide header
    if (id > 0) mShowHeaders = mShowHeaders & ~id;
    else mShowHeaders = abs(mShowHeaders);
  }
  else
  {
    // show header
    if (id > 0) mShowHeaders |= id;
    else mShowHeaders = -abs(mShowHeaders);
  }
  rethinkFields(true);

}

void KMComposeWin::rethinkFields(bool fromSlot)
{
  //This sucks even more but again no ids. sorry (sven)
  int mask, row, numRows;
  long showHeaders;

  if (mShowHeaders < 0)
    showHeaders = HDR_ALL;
  else
    showHeaders = mShowHeaders;

  for (mask=1,mNumHeaders=0; mask<=showHeaders; mask<<=1)
    if ((showHeaders&mask) != 0) mNumHeaders++;

  numRows = mNumHeaders + 2;

  if (mGrid) delete mGrid;
  mGrid = new QGridLayout(&mMainWidget, numRows, 3, 4, 4);
  mGrid->setColStretch(0, 1);
  mGrid->setColStretch(1, 100);
  mGrid->setColStretch(2, 1);
  mGrid->setRowStretch(mNumHeaders, 100);

  mEdtList.clear();
  row = 0;
  kdDebug(5006) << "KMComposeWin::rethinkFields" << endl;
  if (!fromSlot) allFieldsAction->setChecked(showHeaders==HDR_ALL);

  if (!fromSlot) identityAction->setChecked(abs(mShowHeaders)&HDR_IDENTITY);
  rethinkHeaderLine(showHeaders,HDR_IDENTITY, row, i18n("&Identity:"),
		    &mLblIdentity, &mIdentity, &mBtnIdentity);
  if (!fromSlot) transportAction->setChecked(abs(mShowHeaders)&HDR_TRANSPORT);
  rethinkHeaderLine(showHeaders,HDR_TRANSPORT, row, i18n("Mai&l Transport:"),
		    &mLblTransport, &mTransport, &mBtnTransport);
  if (!fromSlot) fromAction->setChecked(abs(mShowHeaders)&HDR_FROM);
  rethinkHeaderLine(showHeaders,HDR_FROM, row, i18n("Fro&m:"),
		    &mLblFrom, &mEdtFrom, &mBtnFrom);
  if (!fromSlot) replyToAction->setChecked(abs(mShowHeaders)&HDR_REPLY_TO);
  rethinkHeaderLine(showHeaders,HDR_REPLY_TO,row,i18n("&Reply to:"),
		    &mLblReplyTo, &mEdtReplyTo, &mBtnReplyTo);
  if (!fromSlot) toAction->setChecked(abs(mShowHeaders)&HDR_TO);
  rethinkHeaderLine(showHeaders,HDR_TO, row, i18n("&To:"),
		    &mLblTo, &mEdtTo, &mBtnTo);
  if (!fromSlot) ccAction->setChecked(abs(mShowHeaders)&HDR_CC);
  rethinkHeaderLine(showHeaders,HDR_CC, row, i18n("&Cc:"),
		    &mLblCc, &mEdtCc, &mBtnCc);
  if (!fromSlot) bccAction->setChecked(abs(mShowHeaders)&HDR_BCC);
  rethinkHeaderLine(showHeaders,HDR_BCC, row, i18n("&Bcc:"),
		    &mLblBcc, &mEdtBcc, &mBtnBcc);
  if (!fromSlot) subjectAction->setChecked(abs(mShowHeaders)&HDR_SUBJECT);
  rethinkHeaderLine(showHeaders,HDR_SUBJECT, row, i18n("S&ubject:"),
		    &mLblSubject, &mEdtSubject);
  assert(row<=mNumHeaders);

  mGrid->addMultiCellWidget(mEditor, row, mNumHeaders, 0, 2);
  mGrid->addMultiCellWidget(mAtmListBox, mNumHeaders+1, mNumHeaders+1, 0, 2);

  if (mAtmList.count() > 0) mAtmListBox->show();
  else mAtmListBox->hide();
  resize(this->size());
  repaint();

  mGrid->activate();

  identityAction->setEnabled(!allFieldsAction->isChecked());
  transportAction->setEnabled(!allFieldsAction->isChecked());
  fromAction->setEnabled(!allFieldsAction->isChecked());
  replyToAction->setEnabled(!allFieldsAction->isChecked());
  toAction->setEnabled(!allFieldsAction->isChecked());
  ccAction->setEnabled(!allFieldsAction->isChecked());
  bccAction->setEnabled(!allFieldsAction->isChecked());
  subjectAction->setEnabled(!allFieldsAction->isChecked());
}


//-----------------------------------------------------------------------------
void KMComposeWin::rethinkHeaderLine(int aValue, int aMask, int& aRow,
				     const QString &aLabelStr, QLabel* aLbl,
				     QLineEdit* aEdt, QPushButton* aBtn)
{
  if (aValue & aMask)
  {
    aLbl->setText(aLabelStr);
    aLbl->adjustSize();
    aLbl->resize((int)aLbl->sizeHint().width(),aLbl->sizeHint().height() + 6);
    aLbl->setMinimumSize(aLbl->size());
    aLbl->show();
    aLbl->setBuddy(aEdt);
    mGrid->addWidget(aLbl, aRow, 0);

    aEdt->setBackgroundColor( backColor );
    aEdt->show();
    aEdt->setMinimumSize(100, aLbl->height()+2);
    aEdt->setMaximumSize(1000, aLbl->height()+2);
    mEdtList.append(aEdt);

    mGrid->addWidget(aEdt, aRow, 1);
    if (aBtn)
    {
      mGrid->addWidget(aBtn, aRow, 2);
      aBtn->setFixedSize(aBtn->sizeHint().width(), aLbl->height());
      aBtn->show();
    }
    aRow++;
  }
  else
  {
    aLbl->hide();
    aEdt->hide();
    if (aBtn) aBtn->hide();
  }
}

//-----------------------------------------------------------------------------
void KMComposeWin::rethinkHeaderLine(int aValue, int aMask, int& aRow,
				     const QString &aLabelStr, QLabel* aLbl,
				     QComboBox* aCbx, QCheckBox* aChk)
{
  if (aValue & aMask)
  {
    aLbl->setText(aLabelStr);
    aLbl->adjustSize();
    aLbl->resize((int)aLbl->sizeHint().width(),aLbl->sizeHint().height() + 6);
    aLbl->setMinimumSize(aLbl->size());
    aLbl->show();
    aLbl->setBuddy(aCbx);
    mGrid->addWidget(aLbl, aRow, 0);

    //    aCbx->setBackgroundColor( backColor );
    aCbx->show();
    aCbx->setMinimumSize(100, aLbl->height()+2);
    aCbx->setMaximumSize(1000, aLbl->height()+2);

    mGrid->addWidget(aCbx, aRow, 1);
    mGrid->addWidget(aChk, aRow, 2);
    aChk->setFixedSize(aChk->sizeHint().width(), aLbl->height());
    aChk->show();
    aRow++;
  }
  else
  {
    aLbl->hide();
    aCbx->hide();
    aChk->hide();
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::setupActions(void)
{
  if (kernel->msgSender()->sendImmediate()) //default == send now?
  {
    //default = send now, alternative = queue
    (void) new KAction (i18n("&Send"), "mail_send", CTRL+Key_Return,
                        this, SLOT(slotSendNow()), actionCollection(),
                        "send_default");
    (void) new KAction (i18n("&Queue"), "queue", 0,
                        this, SLOT(slotSendLater()),
                        actionCollection(), "send_alternative");
  }
  else //no, default = send later
  {
    //default = queue, alternative = send now
    (void) new KAction (i18n("&Queue"), "queue",
                        CTRL+Key_Return,
                        this, SLOT(slotSendLater()), actionCollection(),
                        "send_default");
    (void) new KAction (i18n("S&end now"), "mail_send", 0,
                        this, SLOT(slotSendNow()),
                        actionCollection(), "send_alternative");
  }

  (void) new KAction (i18n("Save in &drafts folder"), 0,
		      this, SLOT(slotSaveDraft()),
		      actionCollection(), "save_in_drafts");
  (void) new KAction (i18n("&Insert File..."), "fileopen", 0,
                      this,  SLOT(slotInsertFile()),
                      actionCollection(), "insert_file");
  (void) new KAction (i18n("&Addressbook..."), "contents",0,
                      this, SLOT(slotAddrBook()),
                      actionCollection(), "addresbook");
  (void) new KAction (i18n("&New Composer..."), "filenew",
                      KStdAccel::key(KStdAccel::New),
                      this, SLOT(slotNewComposer()),
                      actionCollection(), "new_composer");

  //KStdAction::save(this, SLOT(), actionCollection(), "save_message");
  KStdAction::print (this, SLOT(slotPrint()), actionCollection());
  KStdAction::close (this, SLOT(slotClose()), actionCollection());

  KStdAction::undo (this, SLOT(slotUndo()), actionCollection());
  KStdAction::redo (this, SLOT(slotRedo()), actionCollection());
  KStdAction::cut (this, SLOT(slotCut()), actionCollection());
  KStdAction::copy (this, SLOT(slotCopy()), actionCollection());
  KStdAction::paste (this, SLOT(slotPaste()), actionCollection());
  KStdAction::selectAll (this, SLOT(slotMarkAll()), actionCollection());

  KStdAction::find (this, SLOT(slotFind()), actionCollection());
  KStdAction::replace (this, SLOT(slotReplace()), actionCollection());
  KStdAction::spelling (this, SLOT(slotSpellcheck()), actionCollection(), "spellcheck");

  (void) new KAction (i18n("Cl&ean Spaces"), 0, this, SLOT(slotCleanSpace()),
                      actionCollection(), "clean_spaces");

  //these are checkable!!!
  urgentAction = new KToggleAction (i18n("&Urgent"), 0,
                                    actionCollection(),
                                    "urgent");
  confirmDeliveryAction =  new KToggleAction (i18n("&Confirm delivery"), 0,
                                              actionCollection(),
                                              "confirm_delivery");
  confirmReadAction = new KToggleAction (i18n("Confirm &read"), 0,
                                         actionCollection(), "confirm_read");
//this is obsolete now, we use a pulldown menu
#if defined CHARSETS
  (void) new KAction (i18n("&Charsets..."), 0, this, SLOT(slotConfigureCharsets()),
                      actionCollection(), "charsets");
#endif

  //----- Message-Encoding Submenu
  encodingAction = new KSelectAction( i18n( "Set &Encoding" ), "charset",
				      0, this, SLOT(slotSetCharset() ),
				      actionCollection(), "charsets" );
  wordWrapAction = new KToggleAction (i18n("&Wordwrap"), 0,
		      actionCollection(), "wordwrap");
  wordWrapAction->setChecked(mWordWrap);
  connect(wordWrapAction, SIGNAL(toggled(bool)), SLOT(slotWordWrapToggled(bool)));

  KConfig *config = kapp->config();
  KConfigGroupSaver saver(config, "Composer");
  QStringList encodings = config->readListEntry("charsets");
  makeDescriptiveNames( encodings );

  encodingAction->setItems( encodings );
  encodingAction->setCurrentItem( -1 );

  //these are checkable!!!
  allFieldsAction = new KToggleAction (i18n("&All Fields"), 0, this,
                                       SLOT(slotView()),
                                       actionCollection(), "show_all_fields");
  identityAction = new KToggleAction (i18n("&Identity"), 0, this,
				      SLOT(slotView()),
				      actionCollection(), "show_identity");
  transportAction = new KToggleAction (i18n("&Mail Transport"), 0, this,
				      SLOT(slotView()),
				      actionCollection(), "show_transport");
  fromAction = new KToggleAction (i18n("&From"), 0, this,
                                  SLOT(slotView()),
                                  actionCollection(), "show_from");
  replyToAction = new KToggleAction (i18n("&Reply to"), 0, this,
                                     SLOT(slotView()),
                                     actionCollection(), "show_reply_to");
  toAction = new KToggleAction (i18n("&To"), 0, this,
                                SLOT(slotView()),
                                actionCollection(), "show_to");
  ccAction = new KToggleAction (i18n("&Cc"), 0, this,
                                SLOT(slotView()),
                                actionCollection(), "show_cc");
  bccAction = new KToggleAction (i18n("&Bcc"), 0, this,
                                 SLOT(slotView()),
                                 actionCollection(), "show_bcc");
  subjectAction = new KToggleAction (i18n("&Subject"), 0, this,
                                     SLOT(slotView()),
                                     actionCollection(), "show_subject");
  //end of checkable

  (void) new KAction (i18n("Append S&ignature"), 0, this,
                      SLOT(slotAppendSignature()),
                      actionCollection(), "append_signature");
  attachPK  = new KAction (i18n("Attach &Public Key..."), 0, this,
                           SLOT(slotInsertPublicKey()),
                           actionCollection(), "attach_public_key");
  attachMPK = new KAction (i18n("Attach My &Public Key"), 0, this,
                           SLOT(slotInsertMyPublicKey()),
                           actionCollection(), "attach_my_public_key");
  (void) new KAction (i18n("&Attach..."), "attach",
                      0, this, SLOT(slotAttachFile()),
                      actionCollection(), "attach");
  (void) new KAction (i18n("&Remove"), 0, this,
                      SLOT(slotAttachRemove()),
                      actionCollection(), "remove");
  (void) new KAction (i18n("&Save..."), "filesave",0,
                      this, SLOT(slotAttachSave()),
                      actionCollection(), "attach_save");
  (void) new KAction (i18n("Pr&operties..."), 0, this,
                      SLOT(slotAttachProperties()),
                      actionCollection(), "attach_properties");

  toolbarAction = KStdAction::showToolbar(this, SLOT(slotToggleToolBar()),
    actionCollection());
  statusbarAction = KStdAction::showStatusbar(this, SLOT(slotToggleStatusBar()),
    actionCollection());
  KStdAction::keyBindings(this, SLOT(slotEditKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotEditToolbars()), actionCollection());
  (void) new KAction (i18n("&Spellchecker..."), 0, this, SLOT(slotSpellcheckConfig()),
                      actionCollection(), "setup_spellchecker");

  encryptAction = new KToggleAction (i18n("Encrypt message"),
                                     "unlock", 0,
                                     actionCollection(), "encrypt_message");
  signAction = new KToggleAction (i18n("Sign message"),
                                  "signature", 0,
                                  actionCollection(), "sign_message");

  // get PGP user id for the chosen identity
  QString pgpUserId;
  QString identStr = i18n( "Default" );
  if( !mId.isEmpty() && KMIdentity::identities().contains( mId ) ) {
    identStr = mId;
  }
  KMIdentity ident(identStr);
  ident.readConfig();
  pgpUserId = ident.pgpIdentity();

  if(!Kpgp::getKpgp()->usePGP())
  {
    attachPK->setEnabled(false);
    attachMPK->setEnabled(false);
    encryptAction->setEnabled(false);
    encryptAction->setChecked(false);
    signAction->setEnabled(false);
    signAction->setChecked(false);
  }
  else if (pgpUserId.isEmpty()) {
    attachMPK->setEnabled(false);
    signAction->setEnabled(false);
    signAction->setChecked(false);
  }
  else
    signAction->setChecked(mAutoPgpSign);

  createGUI("kmcomposerui.rc");

  connect(encryptAction, SIGNAL(toggled(bool)), SLOT(slotEncryptToggled(bool)));
}

//-----------------------------------------------------------------------------
void KMComposeWin::setupStatusBar(void)
{
  statusBar()->addWidget( new QLabel( statusBar(), "" ), 1 );
  statusBar()->insertItem(QString(i18n(" Column"))+":     ",2,0,true);
  statusBar()->insertItem(QString(i18n(" Line"))+":     ",1,0,true);
}


//-----------------------------------------------------------------------------
void KMComposeWin::updateCursorPosition()
{
  int col,line;
  QString temp;
  line = mEditor->currentLine();
  col = mEditor->currentColumn();
  temp = QString(" %1: %2 ").arg(i18n("Line")).arg(line+1);
  statusBar()->changeItem(temp,1);
  temp = QString(" %1: %2 ").arg(i18n("Column")).arg(col+1);
  statusBar()->changeItem(temp,2);
}


//-----------------------------------------------------------------------------
void KMComposeWin::setupEditor(void)
{
  QPopupMenu* menu;
  mEditor->setModified(FALSE);
  //mEditor->setFocusPolicy(QWidget::ClickFocus);

  if (mWordWrap)
  {
    mEditor->setWordWrap( QMultiLineEdit::FixedColumnWidth );
    mEditor->setWrapColumnOrWidth(mLineBreak);
  }
  else
  {
    mEditor->setWordWrap( QMultiLineEdit::NoWrap );
  }

  // Font setup
  mEditor->setFont(mBodyFont);

  menu = new QPopupMenu();
  //#ifdef BROKEN
  menu->insertItem(i18n("Undo"),mEditor,
		   SLOT(undo()), KStdAccel::key(KStdAccel::Undo));
  menu->insertItem(i18n("Redo"),mEditor,
		   SLOT(redo()), KStdAccel::key(KStdAccel::Redo));
  menu->insertSeparator();
  //#endif //BROKEN
  menu->insertItem(i18n("Cut"), this, SLOT(slotCut()));
  menu->insertItem(i18n("Copy"), this, SLOT(slotCopy()));
  menu->insertItem(i18n("Paste"), this, SLOT(slotPaste()));
  menu->insertItem(i18n("Mark all"),this, SLOT(slotMarkAll()));
  menu->insertSeparator();
  menu->insertItem(i18n("Find..."), this, SLOT(slotFind()));
  menu->insertItem(i18n("Replace..."), this, SLOT(slotReplace()));
  mEditor->installRBPopup(menu);
  updateCursorPosition();
  connect(mEditor,SIGNAL(CursorPositionChanged()),SLOT(updateCursorPosition()));
  connect(mEditor,SIGNAL(gotUrlDrop(QDropEvent *)),SLOT(slotDropAction(QDropEvent *)));
}

//-----------------------------------------------------------------------------
void KMComposeWin::verifyWordWrapLengthIsAdequate(const QString &body)
{
  int maxLineLength = 0;
  int curPos;
  int oldPos = 0;
  if (mEditor->QMultiLineEdit::wordWrap() == QMultiLineEdit::FixedColumnWidth) {
    for (curPos = 0; curPos < (int)body.length(); ++curPos)
	if (body[curPos] == '\n') {
	  if ((curPos - oldPos) > maxLineLength)
	    maxLineLength = curPos - oldPos;
	  oldPos = curPos;
	}
    if ((curPos - oldPos) > maxLineLength)
      maxLineLength = curPos - oldPos;
    if (mEditor->wrapColumnOrWidth() < maxLineLength) // column
      mEditor->setWrapColumnOrWidth(maxLineLength);
  }
}

//-----------------------------------------------------------------------------
void KMComposeWin::setMsg(KMMessage* newMsg, bool mayAutoSign, bool allowDecryption)
{
  KMMessagePart bodyPart, *msgPart;
  int i, num;

  //assert(newMsg!=NULL);
  if(!newMsg)
    {
      kdDebug(5006) << "KMComposeWin::setMsg() : newMsg == NULL!\n" << endl;
      return;
    }
  mMsg = newMsg;

  mEdtTo.setText(mMsg->to());
  mEdtFrom.setText(mMsg->from());
  mEdtCc.setText(mMsg->cc());
  mEdtSubject.setText(mMsg->subject());
  mEdtReplyTo.setText(mMsg->replyTo());
  mEdtBcc.setText(mMsg->bcc());

  if (!mBtnIdentity.isChecked() && !newMsg->headerField("X-KMail-Identity").isEmpty())
    mId = newMsg->headerField("X-KMail-Identity");

  for (int i=0; i < mIdentity.count(); ++i)
    if (mIdentity.text(i) == mId) {
      mIdentity.setCurrentItem(i);
      if (mBtnIdentity.isChecked()) slotIdentityActivated(i);
      break;
    }

  // get PGP user id for the currently selected identity
  KMIdentity ident(mIdentity.currentText());
  ident.readConfig();
  QString pgpUserId = ident.pgpIdentity();

  if(Kpgp::getKpgp()->usePGP()) {
    if (pgpUserId.isEmpty()) {
      attachMPK->setEnabled(false);
      signAction->setEnabled(false);
      signAction->setChecked(false);
    }
    else {
      attachMPK->setEnabled(true);
      signAction->setEnabled(true);
      signAction->setChecked(mAutoPgpSign);
    }
  }

  QString transport = newMsg->headerField("X-KMail-Transport");
  if (!mBtnTransport.isChecked() && !transport.isEmpty())
    mTransport.insertItem( transport, 0 );

  num = mMsg->numBodyParts();

  if (num > 0)
  {
    QCString bodyDecoded;
    mMsg->bodyPart(0, &bodyPart);

    int firstAttachment = (bodyPart.typeStr().lower() == "text") ? 1 : 0;
    if (firstAttachment)
    {
      mCharset = bodyPart.charset();
      if (mCharset=="")
        mCharset = mDefCharset;
      if ((mCharset=="") || (mCharset == "default"))
        mCharset = defaultCharset();

      bodyDecoded = bodyPart.bodyDecoded();

      verifyWordWrapLengthIsAdequate(bodyDecoded);

      QTextCodec *codec = KMMsgBase::codecForName(mCharset);
      if (codec)
        mEditor->setText(codec->toUnicode(bodyDecoded));
      else
        mEditor->setText(QString::fromLocal8Bit(bodyDecoded));
      mEditor->insertLine("\n", -1);
    } else mEditor->setText("");
    for(i=firstAttachment; i<num; i++)
    {
      msgPart = new KMMessagePart;
      mMsg->bodyPart(i, msgPart);
      addAttach(msgPart);
    }
  } else{
    mCharset=mMsg->charset();
    if (mCharset=="")
      mCharset=mDefCharset;
    if ((mCharset=="") || (mCharset == "default"))
      mCharset = defaultCharset();

    QCString bodyDecoded = mMsg->bodyDecoded();

      Kpgp* pgp = Kpgp::getKpgp();
      assert(pgp != NULL);

      if (allowDecryption && pgp->setMessage(bodyDecoded))
      {
        QCString str = pgp->frontmatter();
        if ((pgp->isEncrypted() && pgp->decrypt()) || pgp->isSigned())
        {
          str += pgp->message();
          str += pgp->backmatter();
          bodyDecoded = str;
        }
      }

    QTextCodec *codec = KMMsgBase::codecForName(mCharset);
    if (codec) {
      mEditor->setText(codec->toUnicode(bodyDecoded));
    } else
      mEditor->setText(QString::fromLocal8Bit(bodyDecoded));
  }

  setCharset(mCharset);

  if( mAutoSign && mayAutoSign )
  {
    //
    // Espen 2000-05-16
    // Delay the signature appending. It may start a fileseletor.
    // Not user friendy if this modal fileseletor opens before the
    // composer.
    //
    QTimer::singleShot( 0, this, SLOT(slotAppendSignature()) );
  }
  mEditor->setModified(FALSE);


  //put font charset as default charset: jstolarz@kde.org
//  QString defCharset = mMsg->charset();
//  if ((mAtmList.count() <= 0) && defCharset.isEmpty() &&
//        KGlobal::charsets()->isAvailable(mBodyFont.charSet()))
//      mMsg->setCharset(KGlobal::charsets()->name(mBodyFont.charSet()));
}

//-----------------------------------------------------------------------------
bool KMComposeWin::applyChanges(void)
{
  QString str, atmntStr;
  QString temp, replyAddr;

  //assert(mMsg!=NULL);
  if(!mMsg)
  {
    kdDebug(5006) << "KMComposeWin::applyChanges() : mMsg == NULL!\n" << endl;
    return FALSE;
  }

  mMsg->setTo(to());
  mMsg->setFrom(from());
  mMsg->setCc(cc());
  mMsg->setSubject(subject());
  mMsg->setReplyTo(replyTo());
  mMsg->setBcc(bcc());

  if (mIdentity.currentText() == i18n("Default"))
    mMsg->removeHeaderField("X-KMail-Identity");
  else mMsg->setHeaderField("X-KMail-Identity", mIdentity.currentText());

  if (!replyTo().isEmpty()) replyAddr = replyTo();
  else replyAddr = from();

  if (confirmDeliveryAction->isChecked())
    mMsg->setHeaderField("Return-Receipt-To", replyAddr);

  if (confirmReadAction->isChecked())
    mMsg->setHeaderField("X-Chameleon-Return-To", replyAddr);

  if (urgentAction->isChecked())
  {
    mMsg->setHeaderField("X-PRIORITY", "2 (High)");
    mMsg->setHeaderField("Priority", "urgent");
  }

  _StringPair *pCH;
  for (pCH  = mCustHeaders.first();
       pCH != NULL;
       pCH  = mCustHeaders.next()) {
    mMsg->setHeaderField(KMMsgBase::toUsAscii(pCH->name), pCH->value);
  }

  bool isQP = kernel->msgSender()->sendQuotedPrintable();
  if(mAtmList.count() <= 0) {
    // If there are no attachments in the list waiting it is a simple
    // text message.
    mMsg->deleteBodyParts();
    mMsg->setAutomaticFields(TRUE);
    mMsg->setHeaderField("Content-Type","text/plain");

    mMsg->setCteStr(isQP ? "quoted-printable": "8bit");

    if (mCharset == "default")
      mCharset = defaultCharset();
    mMsg->setCharset(mCharset);
    QCString body = pgpProcessedMsg();
    if (body.isNull()) return FALSE;
    if (body.isEmpty()) body = "\n";     // don't crash

    if (isQP)
      mMsg->setBodyEncoded(body);
    else
      mMsg->setBody(body);
  }
  else
  {
    KMMessagePart bodyPart, *msgPart;

    mMsg->deleteBodyParts();
    mMsg->removeHeaderField("Content-Type");
    mMsg->setAutomaticFields(TRUE);

    // create informative header for those that have no mime-capable
    // email client
    mMsg->setBody("This message is in MIME format.\n\n");

    // create bodyPart for editor text.
    bodyPart.setTypeStr("text");
    bodyPart.setSubtypeStr("plain");

    bodyPart.setCteStr(isQP ? "quoted-printable": "8bit");

    if (mCharset == "default")
      mCharset = defaultCharset();
    bodyPart.setCharset(mCharset);
    QCString body = pgpProcessedMsg();
    if (body.isNull()) return FALSE;
    if (body.isEmpty()) body = "\n";     // don't crash

    bodyPart.setBodyEncoded(body);
    mMsg->addBodyPart(&bodyPart);

    // Since there is at least one more attachment create another bodypart
    for (msgPart=mAtmList.first(); msgPart; msgPart=mAtmList.next())
      mMsg->addBodyPart(msgPart);
  }
  if (!mAutoDeleteMsg) mEditor->setModified(FALSE);
  mEdtFrom.setEdited(FALSE);
  mEdtReplyTo.setEdited(FALSE);
  mEdtTo.setEdited(FALSE);
  mEdtCc.setEdited(FALSE);
  mEdtBcc.setEdited(FALSE);
  mEdtSubject.setEdited(FALSE);
  if (mTransport.lineEdit())
    mTransport.lineEdit()->setEdited(FALSE);

  // remove fields that contain no data (e.g. an empty Cc: or Bcc:)
  mMsg->cleanupHeader();
  return TRUE;
}


//-----------------------------------------------------------------------------
bool KMComposeWin::queryClose ()
{
  int rc;

  if(mEditor->isModified() || mEdtFrom.edited() || mEdtReplyTo.edited() ||
     mEdtTo.edited() || mEdtCc.edited() || mEdtBcc.edited() ||
     mEdtSubject.edited() ||
     (mTransport.lineEdit() && mTransport.lineEdit()->edited()))
  {
    rc = KMessageBox::warningYesNoCancel(this,
           i18n("Do you want to discard the message or save it for later?"),
           i18n("Discard or save message"),
           i18n("Save as draft"), i18n("Discard") );
    if (rc == KMessageBox::Cancel)
      return false;
    else if (rc == KMessageBox::Yes)
      slotSaveDraft();
  }
  return true;
}

bool KMComposeWin::queryExit ()
{
  return true;
}

//-----------------------------------------------------------------------------
QCString KMComposeWin::pgpProcessedMsg(void)
{
  Kpgp *pgp = Kpgp::getKpgp();
  bool doSign = signAction->isChecked();
  bool doEncrypt = encryptAction->isChecked();
  QString text;
  QCString cText;

  if (disableBreaking)
      text = mEditor->text();
  else
      text = mEditor->brokenText();

  text.truncate(text.length()); // to ensure text.size()==text.length()+1

  {
    // Provide a local scope for newText.
    QString newText;
    QTextCodec *codec = KMMsgBase::codecForName(mCharset);

    if (mCharset == "us-ascii") {
      cText = KMMsgBase::toUsAscii(text);
      newText = QString::fromLatin1(cText);
    } else if (codec == NULL) {
      kdDebug(5006) << "Something is wrong and I can not get a codec." << endl;
      cText = text.local8Bit();
      newText = QString::fromLocal8Bit(cText);
    } else {
      cText = codec->fromUnicode(text);
      newText = codec->toUnicode(cText);
    }

    if (!text.isEmpty() && (newText != text))
    {
      QString oldText = mEditor->text();
      mEditor->setText(newText);
      kernel->kbp()->idle();
      bool anyway = (KMessageBox::warningYesNo(0L,
      i18n("Not all characters fit into the chosen"
      " encoding.\nSend the message anyway?"),
      i18n("Some characters will be lost"),
      i18n("Yes"), i18n("No, let me change the encoding") ) == KMessageBox::Yes);
      if (!anyway)
      {
        mEditor->setText(oldText);
        return QCString();
      }
    }
  }

  if (!doSign && !doEncrypt) return cText;

  pgp->setMessage(cText, mCharset);

  // get PGP user id for the chosen identity
  QString pgpUserId;
  QString identStr = i18n( "Default" );
  if( !mId.isEmpty() && KMIdentity::identities().contains( mId ) ) {
    identStr = mId;
  }
  KMIdentity ident(identStr);
  ident.readConfig();
  pgpUserId = ident.pgpIdentity();

  if (!doEncrypt)
  {
    if(pgp->sign(pgpUserId)) return pgp->message();
  }
  else
  {
    // encrypting

// FIXME: Use QStringList instead of QStrList in order to support unicode
    QStrList recipients;
    QString _to = to();

    if(!cc().isEmpty()) _to += "," + cc();
    if(!bcc().isEmpty()) _to += "," + bcc();
    // split the to, cc and bcc header into separate addresses
    // important: always ignore quoted characters
    //            ignore '(', ')' and ',' inside quoted strings
    //            comments may be nested
    //            ignore '"' and ',' inside comments
    if (!_to.isEmpty()) {
      QString recipient;
      int addrstart = 0;
      int commentlevel = 0;
      bool insidequote = false;

      for (int index=0; index<_to.length(); index++) {
        // the following conversion to latin1 is o.k. because
        // we can safely ignore all non-latin1 characters
        switch (_to[index].latin1()) {
        case '"' : // start or end of quoted string
          if (commentlevel == 0)
            insidequote = !insidequote;
          break;
        case '(' : // start of comment
          if (!insidequote)
            commentlevel++;
          break;
        case ')' : // end of comment
          if (!insidequote) {
            if (commentlevel > 0)
              commentlevel--;
            else
              kdDebug(5006) << "Error in address splitting: Unmatched ')'"
                            << endl;
          }
          break;
        case '\\' : // quoted character
          index++; // ignore the quoted character
          break;
        case ',' :
          if (!insidequote && (commentlevel == 0)) {
            recipient = _to.mid(addrstart, index-addrstart);
            kdDebug(5006) << "Found recipient: " << recipient << endl;
            if (!recipient.isEmpty())
              recipients.append(recipient.simplifyWhiteSpace().latin1());
            addrstart = index+1;
          }
          break;
        }
      }
      // append the last address to the list of recipients
      if (!insidequote && (commentlevel == 0)) {
        recipient = _to.mid(addrstart, _to.length()-addrstart);
        kdDebug(5006) << "Found recipient: " << recipient << endl;
        if (!recipient.isEmpty())
          recipients.append(recipient.simplifyWhiteSpace().latin1());
      }
      else
        kdDebug(5006) << "Error in address splitting: "
                      << "Unexpected end of address list";
    }

    if(pgp->encryptFor(recipients, pgpUserId, doSign))
      return pgp->message();
  }

  // in case of an error we end up here
  //qWarning(i18n("Error during PGP:") + QString("\n") +
  //	  pgp->lastErrorMsg());

  return QCString();
}


//-----------------------------------------------------------------------------
void KMComposeWin::addAttach(const KURL aUrl)
{
  KIO::Job *job = KIO::get(aUrl);
  atmLoadData ld;
  ld.url = aUrl;
  ld.data = QByteArray();
  ld.insert = false;
  mapAtmLoadData.insert(job, ld);
  connect(job, SIGNAL(result(KIO::Job *)),
          this, SLOT(slotAttachFileResult(KIO::Job *)));
  connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
          this, SLOT(slotAttachFileData(KIO::Job *, const QByteArray &)));
}


//-----------------------------------------------------------------------------
void KMComposeWin::addAttach(const KMMessagePart* msgPart)
{
  mAtmList.append(msgPart);

  // show the attachment listbox if it does not up to now
  if (mAtmList.count()==1)
  {
    mGrid->setRowStretch(mNumHeaders+1, 50);
    mAtmListBox->setMinimumSize(100, 80);
    mAtmListBox->setMaximumHeight( 100 );
    mAtmListBox->show();
    resize(size());
  }

  // add a line in the attachment listbox
  QListViewItem *lvi = new QListViewItem(mAtmListBox);
  msgPartToItem(msgPart, lvi);
  mAtmItemList.append(lvi);
}


//-----------------------------------------------------------------------------
void KMComposeWin::msgPartToItem(const KMMessagePart* msgPart,
				 QListViewItem *lvi)
{
  unsigned int len;
  QString lenStr;

  assert(msgPart != NULL);

  len = msgPart->size();
  if (len > 9999) lenStr.sprintf("%uK", (len>>10));
  else lenStr.sprintf("%u", len);

  if (!msgPart->fileName().isEmpty())
    lvi->setText(0, msgPart->fileName());
  else
    lvi->setText(0, msgPart->name());
  lvi->setText(1, lenStr);
  lvi->setText(2, msgPart->contentTransferEncodingStr());
  lvi->setText(3, msgPart->typeStr() + "/" + msgPart->subtypeStr());
}


//-----------------------------------------------------------------------------
void KMComposeWin::removeAttach(const QString &aUrl)
{
  int idx;
  KMMessagePart* msgPart;
  for(idx=0,msgPart=mAtmList.first(); msgPart;
      msgPart=mAtmList.next(),idx++) {
    if (msgPart->name() == aUrl) {
      removeAttach(idx);
      return;
    }
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::removeAttach(int idx)
{
  mAtmList.remove(idx);
  delete mAtmItemList.take(idx);

  if (mAtmList.count()<=0)
  {
    mAtmListBox->hide();
    mGrid->setRowStretch(mNumHeaders+1, 0);
    mAtmListBox->setMinimumSize(0, 0);
    resize(size());
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::addrBookSelInto(KMLineEdit* aLineEdit)
{
  KMAddrBookSelDlg dlg(this, kernel->addrBook());
  QString txt;

  //assert(aLineEdit!=NULL);
  if(!aLineEdit)
    {
      kdDebug(5006) << "KMComposeWin::addrBookSelInto() : aLineEdit == NULL\n" << endl;
      return;
    }
  if (dlg.exec()==QDialog::Rejected) return;
  txt = aLineEdit->text().stripWhiteSpace();
  if (!txt.isEmpty())
    {
      if (txt.right(1).at(0)!=',') txt += ", ";
      else txt += ' ';
    }
  aLineEdit->setText(txt + dlg.address());
}


//-----------------------------------------------------------------------------
void KMComposeWin::setCharset(const QCString& aCharset, bool forceDefault)
{
  if ((forceDefault && mForceReplyCharset) || aCharset.isEmpty())
    mCharset = mDefCharset;
  else mCharset = aCharset;
  mMsg->setCharset(mCharset);

  QStringList encodings = encodingAction->items();
  int i = 0;
  bool charsetFound = FALSE;
  for ( QStringList::Iterator it = encodings.begin(); it != encodings.end();
     ++it, i++ )
  {
    if (KMMsgBase::codecForName(KGlobal::charsets()->encodingForName(*it)
      .latin1()) == KMMsgBase::codecForName(mCharset))
    {
      encodingAction->setCurrentItem( i );
      slotSetCharset();
      charsetFound = TRUE;
      break;
    }
  }
  if (!aCharset.isEmpty() && !charsetFound) setCharset("", TRUE);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBook()
{
  KMAddrBookExternal::launch(this);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBookFrom()
{
  addrBookSelInto(&mEdtFrom);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBookReplyTo()
{
  addrBookSelInto(&mEdtReplyTo);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBookTo()
{
  addrBookSelInto(&mEdtTo);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBookCc()
{
  addrBookSelInto(&mEdtCc);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBookBcc()
{
  addrBookSelInto(&mEdtBcc);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachFile()
{
  // Create File Dialog and return selected file(s)
  // We will not care about any permissions, existence or whatsoever in
  // this function.

  KURL::List files = KFileDialog::getOpenURLs(QString::null, QString::null,
	this, i18n("Attach File"));
  for (KURL::List::Iterator it = files.begin(); it != files.end(); ++it)
    addAttach(*it);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachFileData(KIO::Job *job, const QByteArray &data)
{
  QMap<KIO::Job*, atmLoadData>::Iterator it = mapAtmLoadData.find(job);
  assert(it != mapAtmLoadData.end());
  QBuffer buff((*it).data);
  buff.open(IO_WriteOnly | IO_Append);
  buff.writeBlock(data.data(), data.size());
  buff.close();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachFileResult(KIO::Job *job)
{
  QMap<KIO::Job*, atmLoadData>::Iterator it = mapAtmLoadData.find(job);
  assert(it != mapAtmLoadData.end());
  if (job->error())
  {
    mapAtmLoadData.remove(it);
    job->showErrorDialog();
    return;
  }
  if ((*it).insert)
  {
    int col, line;
    mEditor->getCursorPosition(&line, &col);
    (*it).data.resize((*it).data.size() + 1);
    (*it).data[(*it).data.size() - 1] = '\0';
    mEditor->insertAt(QString::fromLocal8Bit((*it).data), line, col);
    mapAtmLoadData.remove(it);
    return;
  }
  QString name;
  QString urlStr = (*it).url.prettyURL();
  KMMessagePart* msgPart;
  KMMsgPartDlg dlg;
  int i;

  kernel->kbp()->busy();
  i = urlStr.findRev('/');
  name = (i>=0 ? urlStr.mid(i+1, 256) : urlStr);
  QCString encName = KMMsgBase::encodeRFC2231String(name, mCharset);
  bool RFC2231encoded = name != QString(encName);

  // create message part
  msgPart = new KMMessagePart;
  msgPart->setName(name);
  msgPart->setCteStr("base64");
  msgPart->setBodyEncodedBinary((*it).data);
  msgPart->magicSetType();
  msgPart->setContentDisposition(QCString("attachment; filename")
    + ((RFC2231encoded) ? "*" : "") +  "=\"" + encName + "\"");

  mapAtmLoadData.remove(it);

  // show properties dialog
  kernel->kbp()->idle();
  msgPart->setCharset(mCharset);
  dlg.setMsgPart(msgPart);
  if (!dlg.exec())
  {
    delete msgPart;
    return;
  }
  if (msgPart->typeStr().lower() != "text") msgPart->setCharset(QCString());

  // add the new attachment to the list
  addAttach(msgPart);
  rethinkFields(); //work around initial-size bug in Qt-1.32
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotInsertFile()
{
  KFileDialog fdlg(QString::null, QString::null, this, NULL, TRUE);
  fdlg.setCaption(i18n("Include File"));
  if (!fdlg.exec()) return;

  KURL u = fdlg.selectedURL();

  if (u.fileName().isEmpty()) return;

  KIO::Job *job = KIO::get(u);
  atmLoadData ld;
  ld.url = u;
  ld.data = QByteArray();
  ld.insert = true;
  mapAtmLoadData.insert(job, ld);
  connect(job, SIGNAL(result(KIO::Job *)),
          this, SLOT(slotAttachFileResult(KIO::Job *)));
  connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
          this, SLOT(slotAttachFileData(KIO::Job *, const QByteArray &)));
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotSetCharset()
{
  mCharset = KGlobal::charsets()->encodingForName( encodingAction->
    currentText() ).latin1();
  mMsg->setCharset(mCharset);
  setEditCharset();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotInsertMyPublicKey()
{
  QString str, name;
  KMMessagePart* msgPart;

  kernel->kbp()->busy();

  // get PGP user id for the chosen identity
  QString pgpUserId;
  QString identStr = i18n( "Default" );
  if( !mId.isEmpty() && KMIdentity::identities().contains( mId ) ) {
    identStr = mId;
  }
  KMIdentity ident(identStr);
  ident.readConfig();
  pgpUserId = ident.pgpIdentity();

  str=Kpgp::getKpgp()->getAsciiPublicKey(pgpUserId);
  if (str.isEmpty())
  {
    kernel->kbp()->idle();
    KMessageBox::sorry( 0L, i18n("Couldn't get your public key for\n%1.")
      .arg(Kpgp::getKpgp()->user()) );
    return;
  }

  // create message part
  msgPart = new KMMessagePart;
  msgPart->setName(i18n("my pgp key"));
  msgPart->setCteStr("base64");
  msgPart->setTypeStr("application");
  msgPart->setSubtypeStr("pgp-keys");
  msgPart->setBodyEncoded(QCString(str.ascii()));
  msgPart->setContentDisposition("attachment; filename=public_key.asc");

  // add the new attachment to the list
  addAttach(msgPart);
  rethinkFields(); //work around initial-size bug in Qt-1.32

  kernel->kbp()->idle();
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotInsertPublicKey()
{
  QString str, name;
  KMMessagePart* msgPart;
  Kpgp *pgp;

  pgp=Kpgp::getKpgp();

  name = KpgpKey::getKeyName(this, pgp->keys());
  if (name.isEmpty())
    return;

  str = pgp->getAsciiPublicKey(name);
  if (!str.isEmpty()) {
    // create message part
    msgPart = new KMMessagePart;
    msgPart->setName(i18n("pgp key"));
    msgPart->setCteStr("base64");
    msgPart->setTypeStr("application");
    msgPart->setSubtypeStr("pgp-keys");
    msgPart->setBodyEncoded(QCString(str.ascii()));
    msgPart->setContentDisposition("attachment; filename=public_key.asc");

    // add the new attachment to the list
    addAttach(msgPart);
    rethinkFields(); //work around initial-size bug in Qt-1.32
  } else {
    KMessageBox::sorry( 0L, i18n( "Could not attach public key, perhaps its format is invalid\n"
    	"(e.g. key contains umlaut with wrong encoding)." ) );
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachPopupMenu(QListViewItem *, const QPoint &, int)
{
  QPopupMenu *menu = new QPopupMenu;

  menu->insertItem(i18n("View..."), this, SLOT(slotAttachView()));
  menu->insertItem(i18n("Save..."), this, SLOT(slotAttachSave()));
  menu->insertItem(i18n("Properties..."),
		   this, SLOT(slotAttachProperties()));
  menu->insertItem(i18n("Remove"), this, SLOT(slotAttachRemove()));
  menu->insertSeparator();
  menu->insertItem(i18n("Attach..."), this, SLOT(slotAttachFile()));
  menu->popup(QCursor::pos());
}

//-----------------------------------------------------------------------------
int KMComposeWin::currentAttachmentNum()
{
  int idx = -1;
  QListIterator<QListViewItem> it(mAtmItemList);
  for ( int i = 0; it.current(); ++it, ++i )
    if (*it == mAtmListBox->currentItem()) {
      idx = i;
      break;
    }

  return idx;
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachProperties()
{
  KMMsgPartDlg dlg;
  KMMessagePart* msgPart;
  int idx = currentAttachmentNum();

  if (idx < 0) return;

  msgPart = mAtmList.at(idx);
  msgPart->setCharset(mCharset);
  dlg.setMsgPart(msgPart);
  if (dlg.exec())
  {
    // values may have changed, so recreate the listbox line
    msgPartToItem(msgPart, mAtmItemList.at(idx));
  }
  if (msgPart->typeStr().lower() != "text") msgPart->setCharset(QCString());
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachView()
{
  QString str, pname;
  KMMessagePart* msgPart;
  int idx = currentAttachmentNum();

  if (idx < 0) return;

  msgPart = mAtmList.at(idx);
  pname = msgPart->name();
  if (pname.isEmpty()) pname=msgPart->contentDescription();
  if (pname.isEmpty()) pname="unnamed";

  KTempFile* atmTempFile = new KTempFile();
  mAtmTempList.append( atmTempFile );
  atmTempFile->setAutoDelete( true );
  kByteArrayToFile(msgPart->bodyDecodedBinary(), atmTempFile->name(), false, false,
    false);
  KMReaderWin::atmView(NULL, msgPart, false, atmTempFile->name(), pname,
    KMMsgBase::codecForName(mCharset));
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachSave()
{
  KMMessagePart* msgPart;
  QString fileName, pname;
  int idx = currentAttachmentNum();

  if (idx < 0) return;

  msgPart = mAtmList.at(idx);
  pname = msgPart->name();
  if (pname.isEmpty()) pname="unnamed";

  KURL url = KFileDialog::getSaveURL(QString::null, QString::null, 0, pname);

  if( url.isEmpty() )
    return;

  kernel->byteArrayToRemoteFile(msgPart->bodyDecodedBinary(), url);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachRemove()
{
  int idx = currentAttachmentNum();

  if (idx >= 0)
  {
    removeAttach(idx);
    mEditor->setModified(TRUE);
  }
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotFind()
{
  mEditor->search();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotReplace()
{
  mEditor->replace();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotUndo()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("KEdit"))
    ((QMultiLineEdit*)fw)->undo();
  else if (fw->inherits("KMLineEdit"))
    ((KMLineEdit*)fw)->undo();
}

void KMComposeWin::slotRedo()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("KEdit"))
    ((QMultiLineEdit*)fw)->redo();

}

//-----------------------------------------------------------------------------
void KMComposeWin::slotCut()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("KEdit"))
    ((QMultiLineEdit*)fw)->cut();
  else if (fw->inherits("KMLineEdit"))
    ((KMLineEdit*)fw)->cut();
  else kdDebug(5006) << "wrong focus widget" << endl;
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotCopy()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

#ifdef KeyPress
#undef KeyPress
#endif

  QKeyEvent k(QEvent::KeyPress, Key_C , 0 , ControlButton);
  kapp->notify(fw, &k);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotPaste()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

#ifdef KeyPress
#undef KeyPress
#endif

  QKeyEvent k(QEvent::KeyPress, Key_V , 0 , ControlButton);
  kapp->notify(fw, &k);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotMarkAll()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("QLineEdit"))
      ((QLineEdit*)fw)->selectAll();
  else if (fw->inherits("QMultiLineEdit"))
    ((QMultiLineEdit*)fw)->selectAll();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotClose()
{
  close(FALSE);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotNewComposer()
{
  KMComposeWin* win;
  KMMessage* msg = new KMMessage;

  msg->initHeader();
  win = new KMComposeWin(msg);
  win->show();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotNewMailReader()
{
  KMMainWin *kmmwin = new KMMainWin(NULL);
  kmmwin->show();
  //d->resize(d->size());
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotUpdWinTitle(const QString& text)
{
  if (text.isEmpty())
       setCaption("("+QString(i18n("unnamed"))+")");
  else setCaption(text);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotEncryptToggled(bool on)
{
  if (on) encryptAction->setIcon("lock");
  else encryptAction->setIcon("unlock");
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotWordWrapToggled(bool on)
{
  if (on)
  {
    mEditor->setWordWrap( QMultiLineEdit::FixedColumnWidth );
    mEditor->setWrapColumnOrWidth(mLineBreak);
  }
  else
  {
    mEditor->setWordWrap( QMultiLineEdit::NoWrap );
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotPrint()
{
  KMReaderWin rw;
  applyChanges();
  rw.setMsg(mMsg, true);
  rw.printMsg();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotDropAction(QDropEvent *e)
{
  KURL::List urlList;
  if(QUriDrag::canDecode(e) && KURLDrag::decode( e, urlList ))
  {
    for (KURL::List::Iterator it = urlList.begin(); it != urlList.end(); ++it)
      addAttach(*it);
  }
  else
    KMessageBox::sorry( 0L, i18n( "Only local files are supported." ) );
}


//----------------------------------------------------------------------------
void KMComposeWin::doSend(int aSendNow, bool saveInDrafts)
{
  QString hf = mMsg->headerField("X-KMail-Transport");
  QString msgId = mMsg->msgId();
  static bool busy = 0;
  bool sentOk;
  bool editSubject = true;   // This is an inverted variable due to return type
  if (busy) return;
  busy = true;

  kernel->kbp()->busy();
  //applyChanges();  // is called twice otherwise. Lars
  mMsg->setDateToday();

  // If a user sets up their outgoing messages preferences wrong and then
  // sends mail that gets 'stuck' in their outbox, they should be able to
  // rectify the problem by editing their outgoing preferences and
  // resending.
  // Hence this following conditional
  if ((mTransport.currentText() != kernel->msgSender()->transportString()) ||
      (!hf.isEmpty() && (hf != kernel->msgSender()->transportString())))
    mMsg->setHeaderField("X-KMail-Transport", mTransport.currentText());

  disableBreaking = saveInDrafts;
  if (saveInDrafts)
      sentOk = (applyChanges() && !(kernel->draftsFolder()->addMsg(mMsg)));
  else
      sentOk = (applyChanges() && (editSubject = kernel->msgSender()->send(mMsg, aSendNow)));
  disableBreaking = false;

  kernel->kbp()->idle();

  if (saveInDrafts || !aSendNow)
      emit messageQueuedOrDrafted();

  if (sentOk)
  {
    KMRecentAddresses::self()->add( bcc() );
    KMRecentAddresses::self()->add( cc() );
    KMRecentAddresses::self()->add( to() );

    mAutoDeleteMsg = FALSE;
    mFolder = NULL;
    close();
  }

  // Warning this is an ugly hack but it covers all the changes needed
  // with minimal code changes.  If you want to add more warning boxes
  // (say if the From: field is empty), then you will need to modify this.
  if (!editSubject) {
    if (mEdtTo.text().stripWhiteSpace().isEmpty())
      mEdtTo.setFocus();
    else
      mEdtSubject.setFocus();
  }

  busy = false;
}



//----------------------------------------------------------------------------
void KMComposeWin::slotSendLater()
{
  doSend(FALSE);
}


//----------------------------------------------------------------------------
void KMComposeWin::slotSaveDraft()
{
  doSend(FALSE, TRUE);
}


//----------------------------------------------------------------------------
void KMComposeWin::slotSendNow()
{
  if (mConfirmSend) {
    switch(KMessageBox::warningYesNoCancel(&mMainWidget,
                                    i18n("About to send email..."),
                                    i18n("Send Confirmation"),
                                    i18n("Send &Now"),
                                    i18n("Send &Later"))) {
    case KMessageBox::Yes:        // send now
        doSend(TRUE);
      break;
    case KMessageBox::No:        // send later
        doSend(FALSE);
      break;
    case KMessageBox::Cancel:        // cancel
      break;
    default:
      ;    // whoa something weird happened here!
    }
    return;
  }

  doSend(TRUE);
}





void KMComposeWin::slotAppendSignature()
{
  QString identStr = i18n( "Default" );
  if( !mId.isEmpty() && KMIdentity::identities().contains( mId ) )
  {
    identStr = mId;
  }

  bool mod = mEditor->isModified();

  KMIdentity ident( identStr );
  ident.readConfig();
  QString sigText = ident.signature();
  if( sigText.isEmpty() && ident.useSignatureFile() )
  {
    // open a file dialog and let the user choose manually
    KFileDialog dlg( QDir::homeDirPath(), QString::null, this, 0, TRUE );
    dlg.setCaption(i18n("Choose Signature File"));
    if( !dlg.exec() )
    {
      return;
    }
    KURL url = dlg.selectedURL();
    if( url.isEmpty() )
    {
      return;
    }
    if( !url.isLocalFile() )
    {
      KMessageBox::sorry( 0L, i18n( "Only local files are supported." ) );
      return;
    }
    QString sigFileName = url.path();
    sigText = QString::fromLocal8Bit(kFileToString(sigFileName, TRUE));
    ident.setSignatureFile(sigFileName);
    ident.writeConfig(true);
  }

  mOldSigText = sigText;
  if( !sigText.isEmpty() )
  {
    /* actually "\n-- \n" (note the space) is a convention for attaching
    signatures and we should respect it, unless the user has already done so. */
    if (!sigText.startsWith("-- \n")) mEditor->insertLine("-- ", -1);
    mEditor->insertLine(sigText, -1);
    mEditor->update();
    mEditor->setModified(mod);
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotHelp()
{
  kapp->invokeHelp();
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotCleanSpace()
{
  if (!mEditor) return;

  mEditor->cleanWhiteSpace();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotSpellcheck()
{
  if (mSpellCheckInProgress) return;

  mSpellCheckInProgress=TRUE;
  /*
    connect (mEditor, SIGNAL (spellcheck_progress (unsigned)),
    this, SLOT (spell_progress (unsigned)));
    */

  if(mEditor){
    connect (mEditor, SIGNAL (spellcheck_done()),
	     this, SLOT (slotSpellcheckDone ()));
    mEditor->spellcheck();
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotSpellcheckDone()
{
  kdDebug(5006) << "spell check complete" << endl;
  mSpellCheckInProgress=FALSE;
  statusBar()->changeItem(i18n("Spellcheck complete."),0);

}


//-----------------------------------------------------------------------------
void KMComposeWin::setEditCharset()
{
  QFont fnt=mSavedEditorFont;
  if (mCharset == "default" || mCharset.isEmpty())
    mCharset = defaultCharset();
  QString cset = mCharset;
  if (mUnicodeFont) cset = "iso10646-1";
  fnt.setCharSet(KGlobal::charsets()->charsetForEncoding(cset));
  mEditor->setFont(fnt);
  mEdtFrom.setFont(fnt);
  mEdtReplyTo.setFont(fnt);
  mEdtTo.setFont(fnt);
  mEdtCc.setFont(fnt);
  mEdtBcc.setFont(fnt);
  mEdtSubject.setFont(fnt);
}

//-----------------------------------------------------------------------------
QCString KMComposeWin::defaultCharset(void) const
{
  // first try
  QCString retval = KGlobal::locale()->charset().latin1();
  // however KGlobal::locale->charset() has a fallback value of iso-8859-1,
  // we try to be smarter
  QCString aStr = QTextCodec::codecForLocale()->name();
  if ((retval == "iso-8859-1") && (aStr != "ISO 8859-1"))
  {
    // read locale if it really gives iso-8859-1
    KConfig *globalConfig = KGlobal::instance()->config();
    if (globalConfig)
    {
      KConfigGroupSaver saver(globalConfig, "Locale");
      retval = globalConfig->readEntry("Charset").latin1();
      if (retval.isNull())  //this means iso-8859-1 was a fallback, make your own guess
      {
        //we basicly use LANG envvar here
        QString bStr = "";
        QChar spaceChar(' ');
        for (int i = 0; i < (int)aStr.length(); i++)
           if (aStr[i] != spaceChar)
             bStr += QChar(aStr[i]).lower();
         retval = KGlobal::charsets()->name(KGlobal::charsets()->nameToID(bStr))
           .latin1();
      }
    }
    //we should be pretty safe: still if sth goes wrong we return iso-8859-1
  }
  else if (retval == "jisx0208.1983-0") retval = "iso-2022-jp";
  else if (retval == "ksc5601.1987-0") retval = "euc-kr";
  return retval;
}

//-----------------------------------------------------------------------------
void KMComposeWin::focusNextPrevEdit(const QWidget* aCur, bool aNext)
{
  QWidget* cur;

  if (!aCur)
  {
    cur=mEdtList.last();
  }
  else
  {
    for (cur=mEdtList.first(); aCur!=cur && cur; cur=mEdtList.next())
      ;
    if (!cur) return;
    if (aNext) cur = mEdtList.next();
    else cur = mEdtList.prev();
  }
  if (cur) cur->setFocus();
  else if (aNext) mEditor->setFocus(); //Key up from first doea nothing (sven)
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotIdentityActivated(int)
{
  QString identStr = mIdentity.currentText();
  if (!KMIdentity::identities().contains(identStr))
    return;
  KMIdentity ident(identStr);
  ident.readConfig();

  if(!ident.fullEmailAddr().isNull())
    mEdtFrom.setText(ident.fullEmailAddr());
  if(!ident.replyToAddr().isNull())
    mEdtReplyTo.setText(ident.replyToAddr());
  else
    mEdtReplyTo.setText("");
  if (ident.organization().isEmpty())
    mMsg->removeHeaderField("Organization");
  else
    mMsg->setHeaderField("Organization", ident.organization());

  QString edtText = mEditor->text();
  int pos = edtText.findRev( "\n-- \n" + mOldSigText);

  if (!mBtnTransport.isChecked()) {
    if (ident.transport().isEmpty())
      mMsg->removeHeaderField("X-KMail-Transport");
    else
      mMsg->setHeaderField("X-KMail-Transport", ident.transport());
    QString transp = ident.transport();
    if (transp.isEmpty()) transp = kernel->msgSender()->transportString();
    bool found = false;
    int i;
    for (i = 0; i < mTransport.count(); i++) {
      if (mTransport.text(i) == transp) {
        found = true;
        mTransport.setCurrentItem(i);
        break;
      }
    }
    if (found == false) {
      if (i == mTransport.maxCount()) mTransport.setMaxCount(i + 1);
      mTransport.insertItem(transp,i);
      mTransport.setCurrentItem(i);
    }
  }

  if (((pos >= 0) && (pos + mOldSigText.length() + 5 == edtText.length())) ||
      (mOldSigText.isEmpty())) {
    if (pos >= 0)
      edtText.truncate(pos);
    if (!ident.signature().isEmpty() && mAutoSign) {
      edtText.append( "\n" );
      if (!ident.signature().startsWith("-- \n")) edtText.append( "-- \n" );
      edtText.append( ident.signature() );
    }
    mEditor->setText( edtText );
  }
  mOldSigText = ident.signature();
  
  // disable certain actions if there is no PGP user identity set
  // for this profile
  if (ident.pgpIdentity().isEmpty()) {
    attachMPK->setEnabled(false);
    signAction->setEnabled(false);
    signAction->setChecked(false);
  }
  else {
    attachMPK->setEnabled(true);
    // don't change the state of the sign button if the button
    // was already enabled for the former identity
    if (!signAction->isEnabled()) {
      signAction->setEnabled(true);
      signAction->setChecked(mAutoPgpSign);
    }
  }

  mEditor->setModified(TRUE);
  mId = identStr;
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotSpellcheckConfig()
{
  KWin kwin;
  QTabDialog qtd (this, "tabdialog", true);
  KSpellConfig mKSpellConfig (&qtd);

  qtd.addTab (&mKSpellConfig, i18n("Spellchecker"));
  qtd.setCancelButton ();

  kwin.setIcons (qtd.winId(), kapp->icon(), kapp->miniIcon());

  if (qtd.exec())
    mKSpellConfig.writeGlobalSettings();
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotToggleToolBar()
{
  if(toolBar("mainToolBar")->isVisible())
    toolBar("mainToolBar")->hide();
  else
    toolBar("mainToolBar")->show();
}

void KMComposeWin::slotToggleStatusBar()
{
  if (statusBar()->isVisible())
    statusBar()->hide();
  else
    statusBar()->show();
}

void KMComposeWin::slotEditToolbars()
{
  KEditToolbar dlg(actionCollection(), "kmcomposerui.rc");

  if (dlg.exec() == true)
  {
    createGUI("kmcomposerui.rc");
    toolbarAction->setChecked(!toolBar()->isHidden());
  }
}

void KMComposeWin::slotEditKeys()
{
  KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}

void KMComposeWin::setReplyFocus( bool hasMessage )
{
  mEditor->setFocus();
  if ( hasMessage )
    mEditor->setCursorPosition( 1, 0 );
}

void KMComposeWin::slotCompletionModeChanged( KGlobalSettings::Completion mode)
{
    KConfig *config = KGlobal::config();
    KConfigGroupSaver cs( config, "Composer" );
    config->writeEntry( "Completion Mode", (int) mode );
    config->sync(); // maybe not?

    // sync all the lineedits to the same completion mode
    mEdtFrom.setCompletionMode( mode );
    mEdtReplyTo.setCompletionMode( mode );
    mEdtTo.setCompletionMode( mode );
    mEdtCc.setCompletionMode( mode );
    mEdtBcc.setCompletionMode( mode );
}

//=============================================================================
//
//   Class  KMLineEdit
//
//=============================================================================

KCompletion * KMLineEdit::s_completion = 0L;
bool KMLineEdit::s_addressesDirty = false;

KMLineEdit::KMLineEdit(KMComposeWin* composer, bool useCompletion,
                       QWidget *parent, const char *name)
    : KMLineEditInherited(parent,name)
{
  mComposer = composer;
  m_useCompletion = useCompletion;

  if ( !s_completion ) {
      s_completion = new KCompletion();
      s_completion->setOrder( KCompletion::Sorted );
  }

  installEventFilter(this);

  if ( m_useCompletion )
  {
      setCompletionObject( s_completion, false ); // we handle it ourself
      connect( this, SIGNAL( completion(const QString&)),
               this, SLOT(slotCompletion() ));

      KCompletionBox *box = completionBox();
      connect( box, SIGNAL( highlighted( const QString& )),
               this, SLOT( slotPopupCompletion( const QString& ) ));
      connect( completionBox(), SIGNAL( userCancelled( const QString& )),
               SLOT( setText( const QString& )));


      // Whenever a new KMLineEdit is created (== a new composer is created),
      // we set a dirty flag to reload the addresses upon the first completion.
      // The address completions are shared between all KMLineEdits.
      // Is there a signal that tells us about addressbook updates?
      s_addressesDirty = true;
  }
}


//-----------------------------------------------------------------------------
KMLineEdit::~KMLineEdit()
{
  removeEventFilter(this);
}

//-----------------------------------------------------------------------------
void KMLineEdit::setFont( const QFont& font )
{
    KMLineEditInherited::setFont( font );
    if ( m_useCompletion )
        completionBox()->setFont( font );
}

//-----------------------------------------------------------------------------
bool KMLineEdit::eventFilter(QObject *o, QEvent *e)
{
#ifdef KeyPress
#undef KeyPress
#endif

  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent* k = (QKeyEvent*)e;

    if (k->state()==ControlButton && k->key() == Key_T)
    {
      doCompletion(true);
      return TRUE;
    }
    // ---sven's Return is same Tab and arrow key navigation start ---
    if ((k->key() == Key_Enter || k->key() == Key_Return) &&
        !completionBox()->isVisible())
    {
      mComposer->focusNextPrevEdit(this,TRUE);
      return TRUE;
    }
    if (k->state()==ControlButton && k->key() == Key_Right)
    {
      if ((int)text().length() == cursorPosition()) // at End?
      {
        doCompletion(true);
        return TRUE;
      }
      return FALSE;
    }
    if (k->key() == Key_Up)
    {
      mComposer->focusNextPrevEdit(this,FALSE); // Go up
      return TRUE;
    }
    if (k->key() == Key_Down)
    {
      mComposer->focusNextPrevEdit(this,TRUE); // Go down
      return TRUE;
    }
    // ---sven's Return is same Tab and arrow key navigation end ---

  }
  return KMLineEditInherited::eventFilter(o, e);
}


//-----------------------------------------------------------------------------
void KMLineEdit::cursorAtEnd()
{
    setCursorPosition( text().length() );
}


void KMLineEdit::undo()
{
    QKeyEvent k(QEvent::KeyPress, 90, 26, 16 ); // Ctrl-Z
    keyPressEvent( &k );
}

//-----------------------------------------------------------------------------
void KMLineEdit::doCompletion(bool ctrlT)
{
    if ( !m_useCompletion )
        return;

    QString s(text());
    QString prevAddr;
    int n = s.findRev(',');
    if (n>= 0)
    {
        prevAddr = s.left(n+1) + ' ';
        s = s.mid(n+1,255).stripWhiteSpace();
    }

    KCompletionBox *box = completionBox();
    box->clear();

    if ( s.isEmpty() )
    {
        box->hide();
        return;
    }

    KGlobalSettings::Completion  mode = completionMode();

    if ( s_addressesDirty )
        loadAddresses();

    QString match;
    int curPos = cursorPosition();
    if ( mode != KGlobalSettings::CompletionNone )
        match = s_completion->makeCompletion( s );

    // kdDebug(5006) << "** completion for: " << s << " : " << match << endl;

    if ( ctrlT )
    {
        QStringList addresses = s_completion->items();
        QStringList::Iterator it = addresses.begin();
        for (; it != addresses.end(); ++it)
        {
            if ((*it).find(s,0,false) >= 0)
                box->insertItem(*it);
        }

        if (box->count() > 1) {
            m_previousAddresses = prevAddr;
            box->setCancelledText( text() );
            box->popup();
        }
        else if (box->count() == 1)
            setText(prevAddr + box->text(0));
        else
            box->hide();
        
        cursorAtEnd();
        return;
    }
        
    switch ( mode )
    {
        case KGlobalSettings::CompletionPopup:
        {
            if ( !match.isNull() )
            {
                m_previousAddresses = prevAddr;
                box->insertStringList( s_completion->allMatches( s ));
                box->setCancelledText( text() );
                box->popup();
            }
            else
                box->hide();

            break;
        }

        case KGlobalSettings::CompletionShell:
        {
            if ( !match.isNull() && match != s )
            {
                setText( prevAddr + match );
                cursorAtEnd();
            }
            break;
        }

        case KGlobalSettings::CompletionMan: // Short-Auto in fact
        case KGlobalSettings::CompletionAuto:
        {
            if ( !match.isNull() && match != s )
            {
                QString adds = prevAddr + match;
                validateAndSet( adds, curPos, curPos, adds.length() );
            }
            break;
        }

        default: // fall through
        case KGlobalSettings::CompletionNone:
            break;
    }
}

//-----------------------------------------------------------------------------
void KMLineEdit::slotPopupCompletion( const QString& completion )
{
    setText( m_previousAddresses + completion );
    cursorAtEnd();
}

//-----------------------------------------------------------------------------
void KMLineEdit::loadAddresses()
{
    s_completion->clear();
    s_addressesDirty = false;

    QStringList recent = KMRecentAddresses::self()->addresses();
    QStringList::Iterator it = recent.begin();
    for ( ; it != recent.end(); ++it )
        s_completion->addItem( *it );

    KMAddrBook adb;
    adb.readConfig();

    if(adb.load() == IO_FatalError)
        return;

    if (!KMAddrBookExternal::useKAB()) {
        QStringList::ConstIterator it = adb.begin();
        for ( ; it != adb.end(); ++it )
            s_completion->addItem( *it );
    }

    else {
        QStringList addresses;
        KabBridge::addresses(&addresses);
        QStringList::Iterator it = addresses.begin();
        for (; it != addresses.end(); ++it)
            s_completion->addItem( *it );
    }
}


//-----------------------------------------------------------------------------
void KMLineEdit::dropEvent(QDropEvent *e)
{
  QStrList uriList;
  if(QUriDrag::canDecode(e) && QUriDrag::decode( e, uriList ))
  {
    QString ct = text();
    for (QStrListIterator it(uriList); it; ++it)
    {
      if (!ct.isEmpty()) ct.append(", ");
      KURL u(*it);
      if (u.protocol() == "mailto") ct.append(QString::fromUtf8(u.path().latin1()));
      else ct.append(QString::fromUtf8(*it));
    }
    setText(ct);
  }
  else QLineEdit::dropEvent(e);
}


//=============================================================================
//
//   Class  KMEdit
//
//=============================================================================
KMEdit::KMEdit(QWidget *parent, KMComposeWin* composer,
	       const char *name):
  KMEditInherited(parent, name)
{
  mComposer = composer;
  installEventFilter(this);
  KCursor::setAutoHideCursor( this, true, true );

  extEditor = false;     // the default is to use ourself

  mKSpell = NULL;
  mTempFile = NULL;
  mExtEditorProcess = NULL;
}


//-----------------------------------------------------------------------------
KMEdit::~KMEdit()
{

  removeEventFilter(this);

  if (mKSpell) delete mKSpell;

}


//-----------------------------------------------------------------------------
QString KMEdit::brokenText() const
{
    QString temp;

    for (int i = 0; i < numLines(); ++i) {
      temp += *getString(i);
      if (i + 1 < numLines())
	temp += '\n';
    }

    return temp;
}

//-----------------------------------------------------------------------------
bool KMEdit::eventFilter(QObject*o, QEvent* e)
{
  if (o == this)
    KCursor::autoHideEventFilter(o, e);

  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent *k = (QKeyEvent*)e;

    if (extEditor) {
      if (k->key() == Key_Up)
      {
        mComposer->focusNextPrevEdit(0, false); //take me up
        return TRUE;
      }

      if (mTempFile) return TRUE;
      QRegExp repFn("\\%f");
      QString sysLine = mExtEditor;
      mTempFile = new KTempFile();

      mTempFile->setAutoDelete(true);

      fprintf(mTempFile->fstream(), "%s", (const char *)text().local8Bit());

      mTempFile->close();
      // replace %f in the system line
      sysLine.replace(repFn, mTempFile->name());
      mExtEditorProcess = new KProcess();
      sysLine += " ";
      while (!sysLine.isEmpty())
      {
        *mExtEditorProcess << sysLine.left(sysLine.find(" ")).local8Bit();
        sysLine.remove(0, sysLine.find(" ") + 1);
      }
      connect(mExtEditorProcess, SIGNAL(processExited(KProcess*)),
              SLOT(slotExternalEditorDone(KProcess*)));
      if (!mExtEditorProcess->start())
      {
        KMessageBox::error(NULL, i18n("Unable to start external editor."));
        delete mExtEditorProcess;
        delete mTempFile;
        mExtEditorProcess = NULL;
        mTempFile = NULL;
      }

      return TRUE;
    } else {
    if (k->key()==Key_Tab)
    {
      int col, row;
      getCursorPosition(&row, &col);
      insertAt("	", row, col); // insert tab character '\t'
      emit CursorPositionChanged();
      return TRUE;
    }
    // ---sven's Arrow key navigation start ---
    // Key Up in first line takes you to Subject line.
    if (k->key() == Key_Up && currentLine() == 0)
    {
      mComposer->focusNextPrevEdit(0, false); //take me up
      return TRUE;
    }
    // ---sven's Arrow key navigation end ---
    }
  }
  return FALSE;
}


//-----------------------------------------------------------------------------
void KMEdit::slotExternalEditorDone(KProcess* proc)
{
  assert(proc == mExtEditorProcess);
  setAutoUpdate(false);
  clear();

  // read data back in from file
  insertLine(QString::fromLocal8Bit(kFileToString(mTempFile->name(),
    TRUE, FALSE)), -1);

  setAutoUpdate(true);
  repaint();
  delete proc;
  delete mTempFile;
  mTempFile = NULL;
  mExtEditorProcess = NULL;
}


//-----------------------------------------------------------------------------
void KMEdit::spellcheck()
{
  mKSpell = new KSpell(this, i18n("Spellcheck - KMail"), this,
		       SLOT(slotSpellcheck2(KSpell*)));
  connect (mKSpell, SIGNAL( death()),
          this, SLOT (slotSpellDone()));
  connect (mKSpell, SIGNAL (misspelling (QString, QStringList *, unsigned)),
          this, SLOT (misspelling (QString, QStringList *, unsigned)));
  connect (mKSpell, SIGNAL (corrected (QString, QString, unsigned)),
          this, SLOT (corrected (QString, QString, unsigned)));
  connect (mKSpell, SIGNAL (done(const QString &)),
          this, SLOT (slotSpellResult (const QString&)));
}


//-----------------------------------------------------------------------------
void KMEdit::slotSpellcheck2(KSpell*)
{
  spellcheck_start();
  //we're going to want to ignore quoted-message lines...
  mKSpell->check(text());
}

//-----------------------------------------------------------------------------
void KMEdit::slotSpellResult(const QString &aNewText)
{
  spellcheck_stop();
  if (mKSpell->dlgResult() == 0)
  {
     setText(aNewText);
  }
  mKSpell->cleanUp();
  emit spellcheck_done();
}

//-----------------------------------------------------------------------------
void KMEdit::slotSpellDone()
{
  KSpell::spellStatus status = mKSpell->status();
  delete mKSpell;
  mKSpell = 0;
  if (status == KSpell::Error)
  {
     KMessageBox::sorry(this, i18n("ISpell could not be started.\n Please make sure you have ISpell properly configured and in your PATH."));
  }
  else if (status == KSpell::Crashed)
  {
     spellcheck_stop();
     KMessageBox::sorry(this, i18n("ISpell seems to have crashed."));
  }
  else
  {
     emit spellcheck_done();
  }
}
