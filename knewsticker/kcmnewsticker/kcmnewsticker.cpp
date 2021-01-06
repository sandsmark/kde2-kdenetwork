/*
 * kcmnewsticker.cpp
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "kcmnewsticker.h"
#include "configaccess.h"
#include "newsengine.h"
#include "newsiconmgr.h"
#include "newssourcedlgimpl.h"

#include <dcopclient.h>

#include <kapp.h>
#include <kcolorbtn.h>
#include <kcombobox.h>
#include <kfontdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kpopupmenu.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include <qcheckbox.h>
#include <qdatastream.h>
#include <qdragobject.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qslider.h>

CategoryItem::CategoryItem(QListView *parent, const QString &text)
	: QListViewItem(parent, text)
{
	setOpen(true);
}

void CategoryItem::setOpen(bool open)
{
	if (open)
		setPixmap(0, SmallIcon(QString::fromLatin1("folder_open")));
	else
		setPixmap(0, SmallIcon(QString::fromLatin1("folder")));

	QListViewItem::setOpen(open);
}

NewsSourceItem::NewsSourceItem(CategoryItem *parent, NewsIconMgr *newsIconMgr,
		const NewsSourceBase::Data &nsd)
	: QObject(0L, 0L), QCheckListItem(parent, QString::null, QCheckListItem::CheckBox),
	m_parent(parent),
	m_newsIconMgr(newsIconMgr)
{
	connect(m_newsIconMgr, SIGNAL(gotIcon(const KURL &, const QPixmap &)),
			SLOT(slotGotIcon(const KURL &, const QPixmap &)));

	setData(nsd);
}

void NewsSourceItem::setOn(bool open)
{
	emit toggled();

	QCheckListItem::setOn(open);
}

NewsSourceBase::Data NewsSourceItem::data() const
{
	NewsSourceBase::Data nsd;
	nsd.enabled = isOn();
	nsd.name = text(0);
	nsd.sourceFile = text(1);
	nsd.maxArticles = text(2).toUInt();
	nsd.icon = m_icon;
	nsd.isProgram = m_isProgram;
	for (unsigned int i = 0; i < DEFAULT_SUBJECTS; i++) {
		NewsSourceBase::Subject subject = static_cast<NewsSourceBase::Subject>(i);
		if (m_parent->text(0) == NewsSourceBase::subjectText(subject)) {
			nsd.subject = subject;
			break;
		}
	}
	return nsd;
}

void NewsSourceItem::setData(const NewsSourceBase::Data &nsd)
{
	setOn(nsd.enabled);
	setText(0, nsd.name);
	setText(1, nsd.sourceFile);
	setText(2, QString::number(nsd.maxArticles));

	m_icon = nsd.icon;
	m_isProgram = nsd.isProgram;
	m_newsIconMgr->getIcon(m_icon);
}

void NewsSourceItem::slotGotIcon(const KURL &url, const QPixmap &pixmap)
{
	if (url.url() == m_icon)
		setPixmap(0, pixmap);
}

KCMNewsTicker::KCMNewsTicker(QWidget *parent,  const char *name)
	: KCModule(parent, name),
	m_config(new KConfig(QString::fromLatin1("knewstickerappletrc"), false, false)),
	m_cfg(new ConfigAccess(m_config)),
	m_child(new KCMNewsTickerWidget(this)),
	m_newsIconMgr(NewsIconMgr::self())
{
	if (!kapp->dcopClient()->isAttached())
		kapp->dcopClient()->attach();

	// Change various properties of the view which the Qt Designer cannot
	// set at design time yet.
	m_child->niInterval->setLabel(i18n("News query interval:"));
	m_child->niInterval->setRange(0, 180, 1);

	m_child->lvNewsSources->setShowSortIndicator(true);

	connect(m_child->sliderMouseWheelSpeed, SIGNAL(valueChanged(int)), SLOT(slotConfigChanged()));
	connect(m_child->niInterval, SIGNAL(valueChanged(int)), SLOT(slotConfigChanged()));
	connect(m_child->cbCustomNames, SIGNAL(clicked()), SLOT(slotConfigChanged()));
	connect(m_child->cbEndlessScrolling, SIGNAL(clicked()), SLOT(slotConfigChanged()));
	connect(m_child->cbScrollMostRecentOnly, SIGNAL(clicked()), SLOT(slotConfigChanged()));
	connect(m_child->sliderScrollSpeed, SIGNAL(valueChanged(int)), SLOT(slotConfigChanged()));
	connect(m_child->comboDirection, SIGNAL(activated(const QString &)), SLOT(slotConfigChanged()));
	connect(m_child->bChooseFont, SIGNAL(clicked()), SLOT(slotChooseFont()));
	connect(m_child->colorForeground, SIGNAL(changed(const QColor &)), SLOT(slotConfigChanged()));
	connect(m_child->colorBackground, SIGNAL(changed(const QColor &)), SLOT(slotConfigChanged()));
	connect(m_child->colorHighlighted, SIGNAL(changed(const QColor &)), SLOT(slotConfigChanged()));
	connect(m_child->cbUnderlineHighlighted, SIGNAL(clicked()), SLOT(slotConfigChanged()));
	connect(m_child->cbShowIcons, SIGNAL(clicked()), SLOT(slotConfigChanged()));
	connect(m_child->cbSlowedScrolling, SIGNAL(clicked()), SLOT(slotConfigChanged()));

	connect(m_child->lvNewsSources, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)),
			SLOT(slotNewsSourceContextMenu(KListView *, QListViewItem *, const QPoint &)));
	connect(m_child->lvNewsSources, SIGNAL(selectionChanged(QListViewItem *)),
			SLOT(slotNewsSourceSelectionChanged(QListViewItem *)));
	connect(m_child->lvNewsSources, SIGNAL(doubleClicked(QListViewItem *, const QPoint &, int)),
			SLOT(slotModifyNewsSource(QListViewItem *, const QPoint &, int)));
	connect(m_child->bAddNewsSource, SIGNAL(clicked()), SLOT(slotAddNewsSource()));
	connect(m_child->bRemoveNewsSource, SIGNAL(clicked()), SLOT(slotRemoveNewsSource()));
	connect(m_child->bModifyNewsSource, SIGNAL(clicked()), SLOT(slotModifyNewsSource()));

	connect(m_child->lvFilters, SIGNAL(selectionChanged(QListViewItem *)),
			SLOT(slotFilterSelectionChanged(QListViewItem *)));
	connect(m_child->comboFilterAction, SIGNAL(activated(const QString &)),
			SLOT(slotFilterActionChanged(const QString &)));
	connect(m_child->comboFilterNewsSource, SIGNAL(activated(const QString &)),
			SLOT(slotFilterNewsSourceChanged(const QString &)));
	connect(m_child->comboFilterCondition, SIGNAL(activated(const QString &)),
			SLOT(slotFilterConditionChanged(const QString &)));
	connect(m_child->leFilterExpression, SIGNAL(textChanged(const QString &)),
			SLOT(slotFilterExpressionChanged(const QString &)));
	connect(m_child->bAddFilter, SIGNAL(clicked()), SLOT(slotAddFilter()));
	connect(m_child->bAddFilter, SIGNAL(clicked()), SLOT(slotConfigChanged()));
	connect(m_child->bRemoveFilter, SIGNAL(clicked()), SLOT(slotRemoveFilter()));

	load();

	m_child->show();
}

KCMNewsTicker::~KCMNewsTicker()
{
	delete m_cfg;
	delete m_config;
}

void KCMNewsTicker::load()
{
	m_child->comboFilterNewsSource->clear();
	m_child->comboFilterNewsSource->insertItem(i18n("all news sources"));

	m_child->niInterval->setValue(m_cfg->interval());
	m_child->sliderMouseWheelSpeed->setValue(m_cfg->mouseWheelSpeed());
	m_child->cbCustomNames->setChecked(m_cfg->customNames());
	m_child->cbEndlessScrolling->setChecked(m_cfg->endlessScrolling());
	m_child->cbScrollMostRecentOnly->setChecked(m_cfg->scrollMostRecentOnly());
	m_child->cbSlowedScrolling->setChecked(m_cfg->slowedScrolling());

	m_child->sliderScrollSpeed->setValue(m_cfg->scrollingSpeed());
	m_child->comboDirection->setCurrentItem(m_cfg->scrollingDirection());

	m_font = m_cfg->font();
	m_child->colorForeground->setColor(m_cfg->foregroundColor());
	m_child->colorBackground->setColor(m_cfg->backgroundColor());
	m_child->colorHighlighted->setColor(m_cfg->highlightedColor());
	m_child->cbUnderlineHighlighted->setChecked(m_cfg->underlineHighlighted());
	m_child->cbShowIcons->setChecked(m_cfg->showIcons());

	m_child->lvNewsSources->clear();
	QStringList nsList = m_cfg->newsSources();
	for (QStringList::Iterator it = nsList.begin(); it != nsList.end(); ++it)
		addNewsSource(m_cfg->newsSource((*it))->data());

	ArticleFilter::List filterList = m_cfg->filters();
	ArticleFilter::List::ConstIterator it = filterList.begin();
	ArticleFilter::List::ConstIterator end = filterList.end();
	for (; it != end; ++it)
		addFilter(m_cfg->filter((*it)));

	slotNewsSourceSelectionChanged(0);

	emit changed(false);
}

void KCMNewsTicker::save()
{
	m_cfg->setInterval(m_child->niInterval->value());
	m_cfg->setMouseWheelSpeed(m_child->sliderMouseWheelSpeed->value());
	m_cfg->setCustomNames(m_child->cbCustomNames->isChecked());
	m_cfg->setEndlessScrolling(m_child->cbEndlessScrolling->isChecked());
	m_cfg->setScrollMostRecentOnly(m_child->cbScrollMostRecentOnly->isChecked());
	m_cfg->setSlowedScrolling(m_child->cbSlowedScrolling->isChecked());

	m_cfg->setScrollingSpeed(m_child->sliderScrollSpeed->value());
	m_cfg->setScrollingDirection(static_cast<ConfigAccess::Direction>(m_child->comboDirection->currentItem()));

	m_cfg->setFont(m_font);
	m_cfg->setForegroundColor(m_child->colorForeground->color());
	m_cfg->setBackgroundColor(m_child->colorBackground->color());
	m_cfg->setHighlightedColor(m_child->colorHighlighted->color());
	m_cfg->setUnderlineHighlighted(m_child->cbUnderlineHighlighted->isChecked());
	m_cfg->setShowIcons(m_child->cbShowIcons->isChecked());

	QStringList newsSources;
	for (QListViewItemIterator it(m_child->lvNewsSources); it.current(); it++)
		if (NewsSourceItem *item = dynamic_cast<NewsSourceItem *>(it.current())) {
			newsSources += item->data().name;
			m_cfg->setNewsSource(item->data());
		}
	m_cfg->setNewsSources(newsSources);

	ArticleFilter::List filters;
	ArticleFilter f;
	unsigned int i = 0;
	for (QListViewItemIterator it(m_child->lvFilters); it.current(); it++)
		if (QCheckListItem *item = dynamic_cast<QCheckListItem *>(it.current())) {
			filters.append(i);
			f.setAction(item->text(0));
			f.setNewsSource(item->text(2));
			f.setCondition(item->text(4));
			f.setExpression(item->text(5));
			f.setEnabled(item->isOn());
			f.setId(i++);
			m_cfg->setFilter(f);
		}
	m_cfg->setFilters(filters);

	QByteArray data;
	kapp->dcopClient()->send("knewsticker", "KNewsTicker", "reparseConfig()", data);

	emit changed(false);
}

void KCMNewsTicker::defaults()
{
	m_child->comboFilterNewsSource->clear();
	m_child->comboFilterNewsSource->insertItem(i18n("all news sources"));
	m_child->lvFilters->clear();
	m_child->bRemoveFilter->setEnabled(false);

	ConfigAccess defCfg;

	m_child->niInterval->setValue(defCfg.interval());
	m_child->sliderMouseWheelSpeed->setValue(defCfg.mouseWheelSpeed());
	m_child->cbCustomNames->setChecked(defCfg.customNames());
	m_child->cbEndlessScrolling->setChecked(defCfg.endlessScrolling());
	m_child->cbScrollMostRecentOnly->setChecked(defCfg.scrollMostRecentOnly());
	m_child->cbShowIcons->setChecked(defCfg.showIcons());
	m_child->cbSlowedScrolling->setChecked(defCfg.slowedScrolling());

	m_child->sliderScrollSpeed->setValue(defCfg.scrollingSpeed());
	m_child->comboDirection->setCurrentItem(defCfg.scrollingDirection());

	m_font = defCfg.font();
	m_child->colorForeground->setColor(defCfg.foregroundColor());
	m_child->colorBackground->setColor(defCfg.backgroundColor());
	m_child->colorHighlighted->setColor(defCfg.highlightedColor());
	m_child->cbUnderlineHighlighted->setChecked(defCfg.underlineHighlighted());

	m_child->lvNewsSources->clear();
	m_child->bRemoveNewsSource->setEnabled(false);
	QStringList newsSources = defCfg.newsSources();
	defCfg.setNewsSources(newsSources);
	for (QStringList::Iterator it = newsSources.begin(); it != newsSources.end(); ++it)
		addNewsSource(defCfg.newsSource((*it))->data());

	emit changed(true);
}

QString KCMNewsTicker::quickHelp() const
{
	return i18n("<h1>News Ticker</h1> This module allows you to configure the"
			" news ticker applet for KDE's panel. Here, you can configure"
			" general settings such as how often KNewsTicker will check for"
			" new articles as well as manage the list of news sources"
			" which KNewsTicker will query for new articles or setup filters.");
}

void KCMNewsTicker::slotConfigChanged()
{
	emit changed(true);
}

void KCMNewsTicker::resizeEvent(QResizeEvent *)
{
	m_child->resize(width(), height());
}

void KCMNewsTicker::addNewsSource(const NewsSourceBase::Data &nsd,
		bool select)
{
	CategoryItem *catItem = 0L;

	for (QListViewItemIterator it(m_child->lvNewsSources); it.current(); it++) {
		if (it.current()->text(0) == NewsSourceBase::subjectText(nsd.subject)) {
			catItem = static_cast<CategoryItem *>(it.current());
			break;
		}
	}

	if (!catItem)
		catItem = new CategoryItem(m_child->lvNewsSources,
				NewsSourceBase::subjectText(nsd.subject));

	NewsSourceItem *item = new NewsSourceItem(catItem, m_newsIconMgr, nsd);
	connect(item, SIGNAL(toggled()), SLOT(slotConfigChanged()));
	if (select)
		m_child->lvNewsSources->setCurrentItem(item);

	m_child->comboFilterNewsSource->insertItem(item->data().name);

	emit changed(true);
}

void KCMNewsTicker::addFilter(const ArticleFilter &fd)
{
	QCheckListItem *item = new QCheckListItem(m_child->lvFilters, fd.action(), QCheckListItem::CheckBox);
	item->setOn(fd.enabled());
	item->setText(1, m_child->lArticles->text());
	item->setText(2, fd.newsSource());
	item->setText(3, m_child->lHeadlines->text());
	item->setText(4, fd.condition());
	item->setText(5, fd.expression());
}

void KCMNewsTicker::removeNewsSource(QListViewItem *item)
{
	if (KMessageBox::warningYesNo(this, i18n("Do you really want to remove the news"
					" source '%1'?<br><br>Press 'Yes' to remove the news source from the list,"
					" press 'No' to keep it and close this dialog.").arg(item->text(0))) == KMessageBox::Yes) {
		for (int i = 0; i < m_child->comboFilterNewsSource->count(); i++)
			if (m_child->comboFilterNewsSource->text(i) == item->text(0)) {
				m_child->comboFilterNewsSource->removeItem(i);
				break;
			}
		if (dynamic_cast<NewsSourceItem *>(item) && item->parent()->childCount() == 1)
			delete item->parent();
		else
			delete item;
		m_child->bRemoveNewsSource->setEnabled(false);
		m_child->bModifyNewsSource->setEnabled(false);
		emit changed(true);
	}
}

void KCMNewsTicker::removeFilter(QListViewItem *item)
{
	if (KMessageBox::warningYesNo(this, i18n("Do you really want to remove the selected"
					" filter?<br><br>Press 'Yes' to remove the filter from the list, press"
					" 'No' to keep it and close this dialog.").arg(item->text(0))) == KMessageBox::Yes) {
		delete item;
		m_child->bRemoveFilter->setEnabled(false);
		emit changed(true);
	}
}

void KCMNewsTicker::slotNewsSourceContextMenu(KListView *, QListViewItem *item, const QPoint &)
{
	if (!dynamic_cast<NewsSourceItem *>(item))
		return;

	KPopupMenu *menu = new KPopupMenu();

	QPixmap addIcon = SmallIcon(QString::fromLatin1("news_subscribe"));
	QPixmap modifyIcon = SmallIcon(QString::fromLatin1("edit"));
	QPixmap removeIcon = SmallIcon(QString::fromLatin1("news_unsubscribe"));
	QPixmap logoIcon = SmallIcon(QString::fromLatin1("knewsticker"));

	menu->insertTitle(logoIcon, i18n("Edit news source"));
	menu->insertItem(addIcon, i18n("&Add news source"), 0);
	if (item) {
		menu->insertItem(modifyIcon, i18n("&Modify '%1'").arg(item->text(0)), 1);
		menu->insertItem(removeIcon, i18n("&Remove '%1'").arg(item->text(0)), 2);
	} else {
		menu->insertItem(modifyIcon, i18n("&Modify news source"), 1);
		menu->insertItem(removeIcon, i18n("&Remove news source"), 2);
		menu->setItemEnabled(1, false);
		menu->setItemEnabled(2, false);
	}

	switch (menu->exec(QCursor::pos())) {
		case 0: slotAddNewsSource(); break;
		case 1: modifyNewsSource(item); break;
		case 2: removeNewsSource(item); break;
	}

	delete menu;
}

void KCMNewsTicker::slotChooseFont()
{
	KFontDialog fd(this, "Font Dialog", false, true);

	fd.setFont(m_font);

	if (fd.exec() == KFontDialog::Accepted) {
		if (m_font != fd.font()) {
			m_font = fd.font();
			emit changed(true);
		}
	}
}

void KCMNewsTicker::slotAddNewsSource()
{
	NewsSourceDlgImpl nsDlg(this, 0L, true);
	connect(&nsDlg, SIGNAL(newsSource(const NewsSourceBase::Data &)),
			SLOT(slotAddNewsSource(const NewsSourceBase::Data &)));
	nsDlg.exec();
}

void KCMNewsTicker::slotAddNewsSource(const NewsSourceBase::Data &nsd)
{
	addNewsSource(nsd);
}

void KCMNewsTicker::slotModifyNewsSource()
{
	if ((m_modifyItem = dynamic_cast<NewsSourceItem *>(m_child->lvNewsSources->selectedItem())))
		openModifyDialog();
}

void KCMNewsTicker::slotModifyNewsSource(const NewsSourceBase::Data &nsd)
{
	m_modifyItem->setData(nsd);

	// FIXME: Only emit changed(true) in case nsd != m_modifyItem->data()
	emit changed(true);
}

void KCMNewsTicker::slotModifyNewsSource(QListViewItem *item, const QPoint &, int)
{
	if (dynamic_cast<NewsSourceItem *>(item))
		modifyNewsSource(item);
}

void KCMNewsTicker::modifyNewsSource(QListViewItem *item)
{
	if ((m_modifyItem = dynamic_cast<NewsSourceItem *>(item)))
		openModifyDialog();
}

void KCMNewsTicker::openModifyDialog()
{
	NewsSourceDlgImpl nsDlg(this, 0L, true);
	connect(&nsDlg, SIGNAL(newsSource(const NewsSourceBase::Data &)),
			SLOT(slotModifyNewsSource(const NewsSourceBase::Data &)));
	nsDlg.setup(m_modifyItem->data());
	nsDlg.exec();
}

void KCMNewsTicker::slotAddFilter()
{
	ArticleFilter fd;
	fd.setAction(m_child->comboFilterAction->currentText());
	fd.setNewsSource(m_child->comboFilterNewsSource->currentText());
	fd.setCondition(m_child->comboFilterCondition->currentText());
	fd.setExpression(m_child->leFilterExpression->text());
	fd.setEnabled(true);
	addFilter(fd);

	emit changed(true);
}

void KCMNewsTicker::slotRemoveNewsSource()
{
    QListViewItem *item=m_child->lvNewsSources->selectedItem();
    if(!item)
        return;
    removeNewsSource(item);
}

void KCMNewsTicker::slotRemoveFilter()
{
    QListViewItem *item=m_child->lvFilters->selectedItem();
    if(!item)
        return;
    removeFilter(item);
}

void KCMNewsTicker::slotNewsSourceSelectionChanged(QListViewItem *item)
{
	m_child->bRemoveNewsSource->setEnabled(item);
	m_child->bModifyNewsSource->setEnabled(dynamic_cast<NewsSourceItem *>(item));
}

void KCMNewsTicker::slotFilterSelectionChanged(QListViewItem *item)
{
	for (int i = 0; i < m_child->comboFilterAction->count(); i++)
		if (m_child->comboFilterAction->text(i) == item->text(0)) {
			m_child->comboFilterAction->setCurrentItem(i);
			break;
		}

	for (int i = 0; i < m_child->comboFilterNewsSource->count(); i++)
		if (m_child->comboFilterNewsSource->text(i) == item->text(2)) {
			m_child->comboFilterNewsSource->setCurrentItem(i);
			break;
		}

	for (int i = 0; i < m_child->comboFilterCondition->count(); i++)
		if (m_child->comboFilterCondition->text(i) == item->text(4)) {
			m_child->comboFilterCondition->setCurrentItem(i);
			break;
		}

	m_child->leFilterExpression->setText(item->text(5));

	m_child->bRemoveFilter->setEnabled(item);
}

void KCMNewsTicker::slotFilterActionChanged(const QString &action)
{
	QListViewItem *item = m_child->lvFilters->selectedItem();

	if (item) {
		item->setText(0, action);
		emit changed(true);
	}
}

void KCMNewsTicker::slotFilterNewsSourceChanged(const QString &newsSource)
{
	QListViewItem *item = m_child->lvFilters->selectedItem();

	if (item) {
		item->setText(2, newsSource);
		emit changed(true);
	}
}

void KCMNewsTicker::slotFilterConditionChanged(const QString &condition)
{
	QListViewItem *item = m_child->lvFilters->selectedItem();

	if (item) {
		item->setText(4, condition);
		emit changed(true);
	}
}

void KCMNewsTicker::slotFilterExpressionChanged(const QString &expression)
{
	QListViewItem *item = m_child->lvFilters->selectedItem();

	if (item) {
		item->setText(5, expression);
		emit changed(true);
	}
}

extern "C"
{
	KCModule *create_newsticker(QWidget *parent, const char *name)
	{
		KGlobal::locale()->insertCatalogue(QString::fromLatin1("kcmnewsticker"));
		return new KCMNewsTicker(parent, name);
	};
}

#include "kcmnewsticker.moc"
