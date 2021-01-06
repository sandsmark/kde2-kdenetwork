/*
 * newsscroller.h
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 * Copyright (c) 2001 Malte Starostik <malte.starostik@t-online.de>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef NEWSSCROLLER_H
#define NEWSSCROLLER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "configaccess.h"
#include "newsengine.h"

#include <qcolor.h>
#include <qframe.h>
#include <qtimer.h>
#include <qlist.h>
#include <qpixmap.h>

#include <kurl.h>

class Headline;
typedef QList<Headline> HeadlineList;

class NewsScroller : public QFrame
{
	Q_OBJECT

	public:
		NewsScroller(QWidget *, ConfigAccess *, const char * = 0);

		virtual QSizePolicy sizePolicy() const;
	
		// Convenience stuff. Somehow ugly, no?	
		inline bool horizontal() const
		{
			return m_cfg->horizontal(static_cast<ConfigAccess::Direction>(m_cfg->scrollingDirection()));
		}

		inline bool vertical() const
		{
			return m_cfg->vertical(static_cast<ConfigAccess::Direction>(m_cfg->scrollingDirection()));
		}

		inline bool rotated() const
		{
			return m_cfg->rotated(static_cast<ConfigAccess::Direction>(m_cfg->scrollingDirection()));
		}

	public slots:
		void clear();
		void addHeadline(Article::Ptr);
		void reset(bool bSeparatorOnly = false);

	signals:
		void contextMenu();

	protected:
		virtual void enterEvent(QEvent *);
		virtual void mousePressEvent(QMouseEvent *);
		virtual void mouseReleaseEvent(QMouseEvent *);
		virtual void mouseMoveEvent(QMouseEvent *);
		virtual void wheelEvent(QWheelEvent *);
		virtual void leaveEvent(QEvent *);
		virtual void drawContents(QPainter *);

	protected slots:
		void slotScrollText(int = 1);
		void slotTimeout();
		void slotStartDrag();

	private:
		int scrollWidth() const;
		int scrollHeight() const;
		bool updateActive(const QPoint &);

	private:
		friend class Headline;
		ConfigAccess *m_cfg;
		QTimer       *m_scrollTimer;
		QTimer       *m_dragTimer;
		HeadlineList m_headlines;
		Headline     *m_activeHeadline;
		QPixmap      m_separator;
		int          m_offset;
		int          m_dragStart;
		bool         m_mouseDrag;
		QString      m_tempHeadline;
};

#endif // NEWSSCROLLER_H
