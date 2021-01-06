/*
 * knewsticker.cpp
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "knewsticker.h"
#include "newsengine.h"
#include "newsscroller.h"
#include "configaccess.h"
#include "newsiconmgr.h"

#include <kaboutapplication.h>
#include <kaboutdata.h>
#include <kapp.h>
#include <karrowbutton.h>
#include <kbugreport.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kstddirs.h>
#include <knotifyclient.h>
#include <kstyle.h>

#include <qtooltip.h>
#include <qlayout.h>
#include <qregexp.h>

#include <dcopclient.h>

KNewsTicker::KNewsTicker(const QString &cfgFile, Type t, int actions, QWidget *parent, const char *name)
	: KPanelApplet(cfgFile, t, actions, parent, name), ConfigIface(),
	DCOPObject("KNewsTicker"),
	m_instance(new KInstance("knewsticker")),
	m_dcopClient(new DCOPClient()),
	m_cfg(new ConfigAccess(config())),
	m_arrowButton(new KArrowButton(this)),
	m_newsTimer(new QTimer(this)),
	m_updateTimer(new QTimer(this)),
	m_newsIconMgr(NewsIconMgr::self()),
	m_scroller(new NewsScroller(this, m_cfg)),
	m_aboutData(new KAboutData("knewsticker", I18N_NOOP("KNewsTicker"), "v0.2",
			I18N_NOOP("A newsticker applet."), KAboutData::License_BSD,
			I18N_NOOP("(c) 2000, 2001 The KNewsTicker developers"))),
	m_contextMenu(0L)
{
	m_dcopClient->registerAs("knewsticker", false);

	QToolTip::add(m_arrowButton, i18n("Show menu"));
	connect(m_arrowButton, SIGNAL(clicked()), SLOT(slotArrowButtonPressed()));
	m_arrowButton->setFocusPolicy(NoFocus);
	setupArrowButton();

	QToolTip::add(m_scroller, QString::null);
	connect(m_scroller, SIGNAL(contextMenu()), SLOT(slotOpenContextMenu()));

	connect(m_newsTimer, SIGNAL(timeout()), SLOT(slotUpdateNews()));

	connect(m_updateTimer, SIGNAL(timeout()), SLOT(slotNotifyOfFailures()));

	m_aboutData->addAuthor("Frerich Raabe", I18N_NOOP("Original author"),
			"raabe@kde.org");
	m_aboutData->addAuthor("Malte Starostik", I18N_NOOP("Hypertext headlines"
				" and much more"), "malte.starostik@t-online.de");
	m_aboutData->addAuthor("Wilco Greven", I18N_NOOP("Mouse wheel support"),
			"greven@kde.org");
	m_aboutData->addAuthor("Adriaan de Groot", I18N_NOOP("Rotated scrolltext"
			" modes"), "adridg@sci.kun.nl");

	reparseConfig();
}

KNewsTicker::~KNewsTicker()
{
	delete m_cfg;
	delete m_dcopClient;
}

int KNewsTicker::heightForWidth(int) const
{
	return m_scroller->height();
}

int KNewsTicker::widthForHeight(int) const
{
	return QFontMetrics(m_cfg->font()).width(QString::fromLatin1("X")) * 20 + m_arrowButton->width();
}

void KNewsTicker::preferences()
{
	KProcess p;
	p << locate("exe", QString::fromLatin1("kdeinit_wrapper"));
	p << locate("exe", QString::fromLatin1("kcmshell"));
	p << QString::fromLatin1("Network/kcmnewsticker");
	p.start(KProcess::DontCare);
}

void KNewsTicker::about()
{
	KAboutApplication aboutDlg(m_aboutData);
	aboutDlg.show();
}

void KNewsTicker::help()
{
	kapp->invokeHelp(QString::null, QString::fromLatin1("knewsticker"));
}

// TODO: Overload KPanelApplet::reportBug() in KDE 3.0
void KNewsTicker::action(Action a)
{
	if (a & KPanelApplet::ReportBug) {
		KBugReport bugReport(this, true, m_aboutData);
		bugReport.show();
	}

	KPanelApplet::action(a);
}

void KNewsTicker::reparseConfig()
{
	m_cfg->reparseConfiguration();
	m_newsSources.clear();

	QStringList newsSources = m_cfg->newsSources();
	QStringList::ConstIterator it = newsSources.begin();
	QStringList::ConstIterator end = newsSources.end();
	for (; it != end; ++it) {
		NewsSourceBase::Ptr ns = m_cfg->newsSource((*it));
		if (!ns->data().enabled)
			continue;
				
		connect(ns, SIGNAL(newNewsAvailable(const NewsSourceBase::Ptr &, bool)),
				SLOT(slotNewsSourceUpdated(const NewsSourceBase::Ptr &, bool)));
		connect(ns, SIGNAL(invalidInput(const NewsSourceBase::Ptr &)),
				SLOT(slotNewsSourceFailed(const NewsSourceBase::Ptr &)));
		m_newsSources.append(ns);
	}
	
	setOfflineMode(m_cfg->offlineMode());
	if (!m_cfg->offlineMode())
		slotUpdateNews();
}

void KNewsTicker::slotUpdateNews()
{
	kdDebug(5005) << "slotUpdateNews()" << endl;
	m_newNews = false;
	
	// TODO: Make the update timeout value configurable
	m_updateTimer->start(30 * 1000, true);

	m_failedNewsUpdates.clear();
	m_pendingNewsUpdates.clear();

	m_scroller->clear();
	
	NewsSourceBase::List::Iterator it = m_newsSources.begin();
	NewsSourceBase::List::Iterator end = m_newsSources.end();
	for (; it != end; ++it) {
		m_pendingNewsUpdates += (*it)->data().name;
		(*it)->retrieveNews();
	}
	kdDebug(5005) << "m_pendingNewsUpdates = " << m_pendingNewsUpdates.join(",")
	              << endl;
}

void KNewsTicker::slotNewsSourceUpdated(const NewsSourceBase::Ptr &ns,
		bool newNews)
{
	kdDebug(5005) << "slotNewsSourceUpdate()" << endl;
	if (newNews)
		m_newNews = true;
	
	NewsSourceBase::List::ConstIterator it = m_newsSources.begin();
	NewsSourceBase::List::ConstIterator end = m_newsSources.end();
	for (; it != end; ++it)
		if (!(*it)->articles().isEmpty())
			if (m_cfg->scrollMostRecentOnly())
				m_scroller->addHeadline((*it)->articles().first());
			else {
				Article::List articles = (*it)->articles();
				Article::List::ConstIterator artIt = articles.begin();
				Article::List::ConstIterator artEnd = articles.end();
				for (; artIt != artEnd; ++artIt)
					m_scroller->addHeadline(*artIt);
			}

	m_scroller->reset(true);

	m_pendingNewsUpdates.remove(ns->data().name);
	kdDebug(5005) << "Updated news source: '" << ns->data().name << "'" << "\n"
	              << "m_pendingNewsUpdates = " << m_pendingNewsUpdates.join(",") << "\n"
	              << "m_failedNewsUpdates = " << m_failedNewsUpdates.join(",")
	              << endl;

	if (m_pendingNewsUpdates.count())
		return;
	
	m_updateTimer->stop();
	
	if (m_failedNewsUpdates.count())
		slotNotifyOfFailures();

	if (m_newNews) {
		KNotifyClient::Instance instance(m_instance);
		KNotifyClient::event(QString::fromLatin1("NewNews"));
	}
}

void KNewsTicker::slotNewsSourceFailed(const NewsSourceBase::Ptr &ns)
{
	m_failedNewsUpdates += ns->newsSourceName();
	slotNewsSourceUpdated(ns);
}

void KNewsTicker::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == QMouseEvent::RightButton)
		slotOpenContextMenu();
}

void KNewsTicker::slotOpenContextMenu()
{
	if (!m_contextMenu)
		m_contextMenu = new KNewsTickerMenu(this);

	m_contextMenu->exec(QCursor::pos());

	delete m_contextMenu;
	m_contextMenu = 0;
}

void KNewsTicker::slotArrowButtonPressed()
{
	if (!m_contextMenu) {
		m_contextMenu = new KNewsTickerMenu(this);
		connect(m_contextMenu, SIGNAL(aboutToHide()),
				SLOT(slotContextMenuAboutToHide()));
	}

	QPoint pos(m_arrowButton->mapToGlobal(QPoint(0, 0)));
	QSize size(m_arrowButton->size());

	if (orientation() == Horizontal) {
		if (popupDirection() == KPanelApplet::Down)
			pos.setY(pos.y() + size.height() + 2);
		else {
			int y = pos.y() - m_contextMenu->sizeHint().height() - 2;
			pos.setY(QMAX(0, y));
		}
	} else {
		if (popupDirection() == KPanelApplet::Right)
			pos.setX(pos.x() + size.width() + 2);
		else
			pos.setX(pos.x() - m_contextMenu->sizeHint().width() - 2);
	}

	m_contextMenu->exec(pos);

	delete m_contextMenu;
	m_contextMenu = 0;
}

void KNewsTicker::orientationChange(Orientation orientation)
{
	delete layout();

	QBoxLayout *layout;

	if (orientation == Horizontal)
		layout = new QHBoxLayout(this);
	else
		layout = new QVBoxLayout(this);

	if (m_arrowButton) {
		layout->addWidget(m_arrowButton);
		setupArrowButton();
	}
	
	layout->addWidget(m_scroller);
}

void KNewsTicker::popupDirectionChange(KPanelApplet::Direction)
{
	if (m_arrowButton)
		setupArrowButton();
}

void KNewsTicker::slotContextMenuAboutToHide()
{
	if (m_arrowButton)
		m_arrowButton->setDown(false);
}

void KNewsTicker::slotNotifyOfFailures()
{
	KNotifyClient::Instance instance(m_instance);
	QString notification = QString::null;

	if (m_failedNewsUpdates.count() == 1)
		notification = i18n("<qt>Couldn't update news site '%1'.<br>"
					"The supplied resource file is probably invalid or"
					" broken.</qt>").arg(m_failedNewsUpdates.first());
	else if (m_failedNewsUpdates.count() > 1 && m_failedNewsUpdates.count() < 8) {
		notification = i18n("<qt>The following news sites had problems. Their"
				" resource files are probably invalid or broken.<ul>");
		QStringList::ConstIterator it = m_failedNewsUpdates.begin();
		QStringList::ConstIterator end = m_failedNewsUpdates.end();
		for (; it != end; ++it)
			notification += QString::fromLatin1("<li>%1</li>").arg(*it);
		notification += QString::fromLatin1("</ul></qt>");
	} else
		notification = i18n("Failed to update several news"
					" sites. The internet connection might be cut.");
	
	KNotifyClient::event(QString::fromLatin1("InvalidRDF"), notification);
}

void KNewsTicker::setInterval(const uint interval)
{
	m_cfg->setInterval(interval);
	m_newsTimer->changeInterval(interval * 60 * 1000);
}

void KNewsTicker::setScrollingSpeed(const uint scrollingSpeed)
{
	m_cfg->setScrollingSpeed(scrollingSpeed);
	m_scroller->reset(true);
}

void KNewsTicker::setMouseWheelSpeed(const uint mouseWheelSpeed)
{
	m_cfg->setMouseWheelSpeed(mouseWheelSpeed);
}

void KNewsTicker::setScrollingDirection(const uint scrollingDirection)
{
	m_cfg->setScrollingDirection(scrollingDirection);
	m_scroller->reset(true);
}

void KNewsTicker::setCustomNames(bool customNames)
{
	m_cfg->setCustomNames(customNames);
}

void KNewsTicker::setEndlessScrolling(bool endlessScrolling)
{
	m_cfg->setEndlessScrolling(endlessScrolling);
	m_scroller->reset(true);
}

void KNewsTicker::setScrollMostRecentOnly(bool scrollMostRecentOnly)
{
	m_cfg->setScrollMostRecentOnly(scrollMostRecentOnly);
	m_scroller->reset(true);
}

void KNewsTicker::setOfflineMode(bool offlineMode)
{
	if (offlineMode)
		m_newsTimer->stop();
	else
		m_newsTimer->start(m_cfg->interval() * 1000 * 60);

	m_cfg->setOfflineMode(offlineMode);
}

void KNewsTicker::setUnderlineHighlighted(bool underlineHighlighted)
{
	m_cfg->setUnderlineHighlighted(underlineHighlighted);
	m_scroller->reset(true);
}

void KNewsTicker::setShowIcons(bool showIcons)
{
	m_cfg->setShowIcons(showIcons);
	m_scroller->reset(true);
}

void KNewsTicker::setSlowedScrolling(bool slowedScrolling)
{
	m_cfg->setSlowedScrolling(slowedScrolling);
}

void KNewsTicker::setForegroundColor(const QColor &foregroundColor)
{
	m_cfg->setForegroundColor(foregroundColor);
	m_scroller->reset(false);
}

void KNewsTicker::setBackgroundColor(const QColor &backgroundColor)
{
	m_cfg->setBackgroundColor(backgroundColor);
	m_scroller->reset(false);
}

void KNewsTicker::setHighlightedColor(const QColor &highlightedColor)
{
	m_cfg->setHighlightedColor(highlightedColor);
	m_scroller->reset(false);
}

void KNewsTicker::slotKillContextMenu()
{
	delete m_contextMenu;
	m_contextMenu = 0;
}

void KNewsTicker::setupArrowButton()
{
	ArrowType at;

	if (orientation() == Horizontal) {
		m_arrowButton->setFixedWidth(12);
		m_arrowButton->setMaximumHeight(128);
		at = (popupDirection() == KPanelApplet::Down ? DownArrow : UpArrow);
	} else {
		m_arrowButton->setMaximumWidth(128);
		m_arrowButton->setFixedHeight(12);
		at = (popupDirection() == KPanelApplet::Right ? RightArrow : LeftArrow);
	}
	m_arrowButton->setArrowType(at);
}

KNewsTickerMenu::KNewsTickerMenu(KNewsTicker *parent, const char *name)
  : KPopupMenu(parent, name),
	m_parent(parent)
{
	unsigned int index = 0;
	
	/*
	 * Perhaps this hardcoded stuff should be replaced by some kind of
	 * themeing functionality?
	 */
	QPixmap helpIcon = SmallIcon(QString::fromLatin1("help"));
	QPixmap confIcon = SmallIcon(QString::fromLatin1("configure"));
	QPixmap lookIcon = SmallIcon(QString::fromLatin1("viewmag"));
	QPixmap newArticleIcon = SmallIcon(QString::fromLatin1("info"));
	QPixmap oldArticleIcon = SmallIcon(QString::fromLatin1("mime_empty"));
	QPixmap noArticlesIcon = SmallIcon(QString::fromLatin1("remove"));
	QPixmap logoIcon = SmallIcon(QString::fromLatin1("knewsticker"));

	insertTitle(logoIcon, i18n("KNewsTicker"));

	KPopupMenu *submenu;

	NewsSourceBase::List sources = parent->m_newsSources;
	NewsSourceBase::List::ConstIterator nIt = sources.begin();
	NewsSourceBase::List::ConstIterator nEnd = sources.end();
	for (; nIt != nEnd; ++nIt) {
		NewsSourceBase::Ptr ns = *nIt;
		submenu = new KPopupMenu;
		insertItem(ns->icon(), ns->newsSourceName(), submenu, index++);
		submenu->insertItem(lookIcon, i18n("Check news"), index++);
		submenu->insertSeparator();
		if (parent->m_pendingNewsUpdates.contains(ns->newsSourceName()))
			// TODO: Use a better message (e.g. "Currently being updated, no articles
			// available") after the message freeze!
			submenu->insertItem(noArticlesIcon, i18n("No articles available"), index++);
		else if (!ns->articles().isEmpty()) {
			Article::List articles = ns->articles();
			Article::List::ConstIterator artIt = articles.begin();
			Article::List::ConstIterator artEnd = articles.end();
			for (; artIt != artEnd; ++artIt) {
				Article::Ptr a = *artIt;
				QString h = a->headline();
				h.replace(QRegExp(QString::fromLatin1("&")), QString::fromLatin1("&&"));
				submenu->insertItem(a->read() ? oldArticleIcon : newArticleIcon, h, index++);
			}
		} else
			submenu->insertItem(noArticlesIcon, i18n("No articles available"), index++);
	}

	if (!parent->m_cfg->newsSources().isEmpty())
		insertSeparator();

	insertItem(lookIcon, i18n("Check news"), index++);
	insertItem(i18n("Offline mode"), index++);
	setItemChecked(index - 1, parent->m_cfg->offlineMode());
	insertSeparator();
	insertItem(helpIcon, i18n("Help"), index++);
	insertItem(helpIcon, i18n("About"), index++);
	insertSeparator();
	insertItem(confIcon, i18n("Preferences..."), index);

	m_idx = index;
}

int KNewsTickerMenu::exec(const QPoint &pos, int)
{
	unsigned int result = KPopupMenu::exec(pos);

	// Did we click on any of the main menu items?
	if (m_idx - result <= 4) {
		switch (m_idx - result) {
			case 4: m_parent->slotUpdateNews(); break;
			case 3: m_parent->setOfflineMode(!m_parent->m_cfg->offlineMode()); break;
			case 2: m_parent->help(); break;
			case 1: m_parent->about(); break;
			case 0: m_parent->preferences(); break;
		}
		return result;
	}

	unsigned int res = result;
	NewsSourceBase::List sources = m_parent->m_newsSources;
	NewsSourceBase::List::ConstIterator it = sources.begin();
	NewsSourceBase::List::ConstIterator end = sources.end();
	for (; it != end; ++it) {
		NewsSourceBase::Ptr ns = *it;
		
		/*
		 * The number of entries is either the number of articles or "1" if there
		 * are not articles available (because the "No articles available" item
		 * got inserted then).
		 */
		unsigned int entries;
		if (m_parent->m_pendingNewsUpdates.contains(ns->newsSourceName()) ||
				ns->articles().count() == 0)
			entries = 1;
		else
			entries = ns->articles().count();
		
		/*
		 * Add one for the spacer which separates the "Check news" item from the
		 * headlines in each submenu.
		 */
		entries++;
		if (res <= entries) {
			if (res == 1)
				ns->retrieveNews();
			else if (!ns->articles().isEmpty()) {
				Article::Ptr a = ns->articles()[res - 2];
				ASSERT(a);
				if (a)
					a->open();
			}
			break;
		} else
			res -= entries + 1;
	}

	return result;
}

extern "C"
{
	KPanelApplet* init(QWidget *parent, const QString &configFile)
	{
		KGlobal::locale()->insertCatalogue(QString::fromLatin1("knewsticker"));
		return new KNewsTicker(configFile, KPanelApplet::Stretch,
				KPanelApplet::Preferences | KPanelApplet::About | KPanelApplet::ReportBug,
			  parent, "knewsticker");
	}
}

#include "knewsticker.moc"
