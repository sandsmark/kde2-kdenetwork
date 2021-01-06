/*
 * kcmnewsticker.h
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef KCMNEWSTICKER_H
#define KCMNEWSTICKER_H

#include "kcmnewstickerwidget.h"
#include "configaccess.h"
#include "newsengine.h"

#include <kcmodule.h>

#include <qevent.h>
#include <qfont.h>
#include <qlistview.h>

class KCMNewsTickerWidget;
class KConfig;
class NewsIconMgr;

class CategoryItem : public QListViewItem
{
	public:
		CategoryItem(QListView *, const QString &);

		void setOpen(bool);
};

class NewsSourceItem : public QObject, public QCheckListItem
{
	Q_OBJECT

	public:
		NewsSourceItem(CategoryItem *, NewsIconMgr *, const NewsSourceBase::Data &);

		NewsSourceBase::Data data() const;
		void setData(const NewsSourceBase::Data &);

		void setOn(bool);
	
	signals:
		void toggled();
	
	protected slots:
		void slotGotIcon(const KURL &, const QPixmap &);
	
	private:
		QString      m_icon;
		bool         m_isProgram;
		CategoryItem *m_parent;
		NewsIconMgr  *m_newsIconMgr;
};

class KCMNewsTicker : public KCModule
{
	Q_OBJECT

	public:
		KCMNewsTicker(QWidget * = 0, const char * = 0);
		~KCMNewsTicker();

		void load();
		void save();
		void defaults();

		int buttons();
		QString quickHelp() const;

		QSize minimumSizeHint() const { return m_child->minimumSizeHint(); }

	protected:
		void addNewsSource(const NewsSourceBase::Data &, bool = false);
		void modifyNewsSource(QListViewItem *);
		void removeNewsSource(QListViewItem *);
		void addFilter(const ArticleFilter &);
		void removeFilter(QListViewItem *);
		void resizeEvent(QResizeEvent *);
		void openModifyDialog();

	protected slots:
		void slotConfigChanged();
		void slotNewsSourceContextMenu(KListView *, QListViewItem *, const QPoint &);
		void slotChooseFont();
		void slotAddNewsSource();
		void slotAddFilter();
		void slotAddNewsSource(const NewsSourceBase::Data &);
		void slotRemoveNewsSource();
		void slotRemoveFilter();
		void slotModifyNewsSource();
		void slotModifyNewsSource(const NewsSourceBase::Data &);
		void slotModifyNewsSource(QListViewItem *, const QPoint &, int);
		void slotNewsSourceSelectionChanged(QListViewItem *);
		void slotFilterSelectionChanged(QListViewItem *);
		void slotFilterActionChanged(const QString &);
		void slotFilterNewsSourceChanged(const QString &);
		void slotFilterConditionChanged(const QString &);
		void slotFilterExpressionChanged(const QString &);

	private:
		KConfig             *m_config;
		ConfigAccess        *m_cfg;
		KCMNewsTickerWidget *m_child;
		QFont               m_font;
		NewsSourceItem      *m_modifyItem;
		NewsIconMgr         *m_newsIconMgr;
};

#endif // KCMNEWSTICKER_H
