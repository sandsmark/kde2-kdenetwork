/*
 * newssourcedlgimpl.cpp
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "newssourcedlgimpl.h"
#include "xmlnewsaccess.h"
#include "configaccess.h"
#include "newsiconmgr.h"

#include <kconfig.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qdom.h>
#include <qimage.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>

NewsSourceDlgImpl::NewsSourceDlgImpl(QWidget *parent,  const char *name, bool modal, WFlags fl)
	: NewsSourceDlg(parent, name, modal, fl),
	m_modified(false),
	m_gotSourceFile(false),
	m_gotIcon(false),
	m_xmlSrc(new XMLNewsSource()),
	m_newsIconMgr(NewsIconMgr::self())
{
	for (unsigned int i = 0; i < DEFAULT_SUBJECTS; i++)
		comboCategory->insertItem(
				NewsSourceBase::subjectText(static_cast<NewsSourceBase::Subject>(i)));
	
	connect(m_xmlSrc, SIGNAL(loadComplete(XMLNewsSource *, bool)),
			SLOT(slotLoadComplete(XMLNewsSource *, bool)));

	connect(m_newsIconMgr, SIGNAL(gotIcon(const KURL &, const QPixmap &)),
			SLOT(slotGotIcon(const KURL &, const QPixmap &)));
}

NewsSourceDlgImpl::~NewsSourceDlgImpl()
{
	delete m_xmlSrc;
}

void NewsSourceDlgImpl::slotCancelClicked()
{
	close();
}

void NewsSourceDlgImpl::slotOkClicked()
{
	KURL url = polishedURL(urlSourceFile->url());

	if (!validateURL(url))
		return;
		
	if (leName->text().isEmpty()) {
		KMessageBox::error(this, i18n("You have to specify a name for this news"
					" source to be able to use it!"), i18n("No name specified"));
		return;
	}
	
	// This finds out which subject is selected in the 'Subject' combo box.
	NewsSourceBase::Subject subject = NewsSourceBase::Computers;
	for (unsigned int i = 0; i < DEFAULT_SUBJECTS; i++) {
		NewsSourceBase::Subject thisSubj = static_cast<NewsSourceBase::Subject>(i);
		if (comboCategory->currentText() == NewsSourceBase::subjectText(thisSubj)) {
			subject = thisSubj;
			break;
		}
	}
	
	NewsSourceBase::Data nsd(leName->text(), url.url(), leIcon->text(), subject,
			sbMaxArticles->value(), true, cbProgram->isChecked());

	emit newsSource(nsd);
	
	close();
}

void NewsSourceDlgImpl::slotSourceFileChanged()
{
	bSuggest->setEnabled(!urlSourceFile->url().isEmpty());
}

void NewsSourceDlgImpl::slotSuggestClicked()
{
	KURL url = polishedURL(urlSourceFile->url());

	if (!validateURL(url))
		return;
	
	m_gotSourceFile = false;
	m_xmlSrc->loadFrom(url);
	
	if (url.isLocalFile())
		url = QString::null;
	else
		url.setEncodedPathAndQuery(QString::fromLatin1("/favicon.ico"));

	m_gotIcon = false;
	m_newsIconMgr->getIcon(url);
	
	m_origCaption = caption();
	setCaption(i18n("Fetching news source data..."));
	lName->setEnabled(false);
	leName->setEnabled(false);
	lSourceFile->setEnabled(false);
	urlSourceFile->setEnabled(false);
	cbProgram->setEnabled(false);
	lCategory->setEnabled(false);
	comboCategory->setEnabled(false);
	lMaxArticles->setEnabled(false);
	sbMaxArticles->setEnabled(false);
	lIcon->setEnabled(false);
	leIcon->setEnabled(false);
	bOk->setEnabled(false);
	bSuggest->setEnabled(false);
	bCancel->setEnabled(false);
}

void NewsSourceDlgImpl::slotModified()
{
	m_modified = true;
}

void NewsSourceDlgImpl::setup(const NewsSourceBase::Data &nsd)
{
	leName->setText(nsd.name);
	urlSourceFile->setURL(nsd.sourceFile);
	cbProgram->setChecked(nsd.isProgram);
	comboCategory->setCurrentItem(nsd.subject);
	sbMaxArticles->setValue(nsd.maxArticles);
	leIcon->setText(nsd.icon);
	m_newsIconMgr->getIcon(nsd.icon);
	setCaption(i18n("Edit news source..."));
}

void NewsSourceDlgImpl::slotLoadComplete(XMLNewsSource *, bool succeeded)
{
	m_gotSourceFile = true;
	m_succeeded = succeeded;

	if (m_gotIcon)
		showSuggestedValues();
}

void NewsSourceDlgImpl::slotGotIcon(const KURL &url, const QPixmap &pixmap)
{
	m_gotIcon = true;
	m_icon = pixmap;
	m_iconURL = url;

	/*
	 * The icon stuff has to be set here instead of in showSuggestedValues() as
	 * it's possible that slotGotIcon() is called on startup instead of within a
	 * "Suggest values" process.
	 */
	pixmapIcon->setPixmap(m_icon);
	if (m_newsIconMgr->isStdIcon(m_icon))
		leIcon->clear();
	else
		leIcon->setText(m_iconURL.url());

	if (m_gotSourceFile)
		showSuggestedValues();
}

void NewsSourceDlgImpl::showSuggestedValues()
{
	setCaption(m_origCaption);
	lName->setEnabled(true);
	leName->setEnabled(true);
	lSourceFile->setEnabled(true);
	urlSourceFile->setEnabled(true);
	cbProgram->setEnabled(true);
	lCategory->setEnabled(true);
	comboCategory->setEnabled(true);
	lMaxArticles->setEnabled(true);
	sbMaxArticles->setEnabled(true);
	lIcon->setEnabled(true);
	leIcon->setEnabled(true);
	bOk->setEnabled(true);
	bSuggest->setEnabled(true);
	bCancel->setEnabled(true);

	if (!m_succeeded) {
		KMessageBox::error(this, i18n("Couldn't retrieve the specified source file!"));
		return;
	}

	cbProgram->setChecked(false);
	leName->setText(m_xmlSrc->newsSourceName());
	sbMaxArticles->setValue(m_xmlSrc->articles().count());
	// Icon stuff was set in slotGotIcon()
}

KURL NewsSourceDlgImpl::polishedURL(const KURL &url) const
{
	KURL newURL = url;
	
	if (url.protocol().isEmpty())
		if (url.url().startsWith(QString::fromLatin1("ftp")))
			newURL = QString::fromLatin1("ftp://") + url.url();
		else
			newURL = QString::fromLatin1("http://") + url.url();

	return newURL;
}

bool NewsSourceDlgImpl::validateURL(const KURL &url)
{
	if (url.isEmpty()) {
		KMessageBox::error(this, i18n("You have to specify the source file for this"
					" news source to be able to use it!"), i18n("No source file"
					" specified"));
		return false;
	}
	
	if (url.isMalformed() || !url.hasPath() || url.encodedPathAndQuery() == QString::fromLatin1("/")) {
		KMessageBox::error(this, i18n("KNewsTicker needs a valid RDF or RSS file to"
					" suggest sensible values. The specified source file is invalid."),
					i18n("Invalid source file"));
		return false;
	}

	return true;
}

#include "newssourcedlgimpl.moc"
