/*
 * kntsrcfilepropsdlg.cpp
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */

#include "kntsrcfilepropsdlg.h"
#include "kntsrcfilepropsdlgwidget.h"
#include "xmlnewsaccess.h"
#include "newsiconmgr.h"

#include <kapp.h>
#include <klistbox.h>
#include <klocale.h>
#include <kpropsdlg.h>
#include <kurl.h>
#include <kurllabel.h>
#include <kstddirs.h>

#include <qmultilineedit.h>
#include <qvbox.h>

KntSrcFilePropsDlg::KntSrcFilePropsDlg(KPropertiesDialog *props)
	: KPropsDlgPlugin(props),
	m_xmlSrc(new XMLNewsSource()),
	m_child(new KntSrcFilePropsDlgWidget(properties->addVBoxPage(i18n("News resource")))),
	m_newsIconMgr(NewsIconMgr::self())
{
	connect(m_child->urlName, SIGNAL(leftClickedURL(const QString &)),
			SLOT(slotOpenURL(const QString &)));
	connect(m_child->lbArticles, SIGNAL(executed(QListBoxItem *)),
			SLOT(slotClickedArticle(QListBoxItem *)));

	connect(m_xmlSrc, SIGNAL(loadComplete(XMLNewsSource *, bool)),
			SLOT(slotConstructUI(XMLNewsSource *, bool)));
	m_xmlSrc->loadFrom(props->item()->url());
	
	connect(m_newsIconMgr, SIGNAL(gotIcon(const KURL &, const QPixmap &)),
			SLOT(slotGotIcon(const KURL &, const QPixmap &)));

	m_child->show();
}

void KntSrcFilePropsDlg::slotConstructUI(XMLNewsSource *, bool loadingSucceeded)
{
	if (!loadingSucceeded)
		return;
	
	KURL iconURL = m_xmlSrc->link();
	iconURL.setEncodedPathAndQuery(QString::fromLatin1("/favicon.ico"));
	m_newsIconMgr->getIcon(iconURL);

	m_child->urlName->setText(m_xmlSrc->newsSourceName());
	m_child->urlName->setURL(m_xmlSrc->link());

	m_child->mleDescription->setText(m_xmlSrc->description());

	XMLNewsArticle::List::ConstIterator it = m_xmlSrc->articles().begin();
	XMLNewsArticle::List::ConstIterator end = m_xmlSrc->articles().end();
	for (; it != end; ++it)
		(void)new QListBoxText(m_child->lbArticles, (*it).headline());
}

KntSrcFilePropsDlg::~KntSrcFilePropsDlg()
{
	delete m_xmlSrc;
}

void KntSrcFilePropsDlg::slotOpenURL(const QString &url)
{
	kapp->invokeBrowser(url);
}

void KntSrcFilePropsDlg::slotGotIcon(const KURL &, const QPixmap &pixmap)
{
	m_child->pixmapIcon->setPixmap(pixmap);
}

void KntSrcFilePropsDlg::slotClickedArticle(QListBoxItem *item)
{
	// FIXME: This will break if two or more articles share the same
	// headline.
	XMLNewsArticle::List::ConstIterator it = m_xmlSrc->articles().begin();
	XMLNewsArticle::List::ConstIterator end = m_xmlSrc->articles().end();
	for (; it != end; ++it)
		if ((*it).headline() == item->text())
			slotOpenURL((*it).address().url());
}

QObject *KntSrcFilePropsFactory::createObject(QObject *parent, const char *,
		const char *classname, const QStringList &)
{
	if (QString::fromLatin1(classname) == "KPropsDlgPlugin")
	{
		if (!parent->inherits("KPropertiesDialog"))
			return 0L;

		QObject *obj = new KntSrcFilePropsDlg(static_cast<KPropertiesDialog *>(parent));
		return obj;
	}
	return 0L;
}

extern "C"
{
	void *init_libkntsrcfilepropsdlg()
	{
		return new KntSrcFilePropsFactory();
	}
};

#include "kntsrcfilepropsdlg.moc"
