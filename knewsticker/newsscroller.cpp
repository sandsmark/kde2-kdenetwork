/*
 * newsscroller.cpp
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 * Copyright (c) 2001 Malte Starostik <malte.starostik@t-online.de>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include <qpainter.h>

#include <kcursor.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kurldrag.h>

#include "configaccess.h"
#include "newsscroller.h"
#include "newsengine.h"

class Headline
{
public:
	Headline(NewsScroller *scroller, const Article::Ptr &article)
		: m_scroller(scroller),
		 m_article(article),
		 m_normal(0),
		 m_highlighted(0)
	{
	};
	
	virtual ~Headline()
	{
		reset();
	}

	Article::Ptr article() const { return m_article; }	
	
	int width() { return pixmap()->width(); }
	int height() { return pixmap()->height(); }
	
	QPixmap *pixmap(bool highlighted = false, bool underlineHighlighted = true)
	{
		QPixmap *result = highlighted ? m_highlighted : m_normal;
		if (!result) {
			const QFontMetrics &metrics = m_scroller->fontMetrics();

			int w, h;
			if (m_scroller->m_cfg->showIcons()) {
				w = m_article->newsSource()->icon().width() + 4 + metrics.width(m_article->headline());
				h = QMAX(metrics.height(), m_article->newsSource()->icon().height());
			} else {
				w = metrics.width(m_article->headline());
				h = metrics.height();
			}

			if (ConfigAccess::rotated(static_cast<ConfigAccess::Direction>(m_scroller->m_cfg->scrollingDirection())))
				result = new QPixmap(h, w);
			else
				result = new QPixmap(w, h);
			
			result->fill(m_scroller->m_cfg->backgroundColor());
			QPainter p(result);
			QFont f = m_scroller->font();
			
			if (highlighted)
				f.setUnderline(underlineHighlighted);
			
			p.setFont(f);
			p.setPen(highlighted ? m_scroller->m_cfg->highlightedColor() : m_scroller->m_cfg->foregroundColor());

			if (ConfigAccess::rotated(static_cast<ConfigAccess::Direction>(m_scroller->m_cfg->scrollingDirection()))) {
				if (m_scroller->m_cfg->scrollingDirection() == ConfigAccess::UpRotated) {
					// Note that rotation also
					// changes the coordinate space
					//
					p.rotate(90.0);
					if (m_scroller->m_cfg->showIcons()) {
						p.drawPixmap(0, -m_article->newsSource()->icon().height(), m_article->newsSource()->icon());
						p.drawText(m_article->newsSource()->icon().width() + 4, -metrics.descent(), m_article->headline());
					} else
						p.drawText(0, -metrics.descent(), m_article->headline());
				} else {
					p.rotate(-90.0);
					if (m_scroller->m_cfg->showIcons()) {
						p.drawPixmap(-w, h - m_article->newsSource()->icon().height(), m_article->newsSource()->icon());
						p.drawText(-w + m_article->newsSource()->icon().width() + 4, h - metrics.descent(), m_article->headline());
					} else
						p.drawText(-w, h - metrics.descent(), m_article->headline());
				}
			} else {
				if (m_scroller->m_cfg->showIcons()) {
					p.drawPixmap(0,
						(result->height() - m_article->newsSource()->icon().height()) / 2,
						m_article->newsSource()->icon());
					p.drawText(m_article->newsSource()->icon().width() + 4, result->height() - metrics.descent(), m_article->headline());
				} else
					p.drawText(0, result->height() - metrics.descent(), m_article->headline());
			}
			
			if (highlighted)
				m_highlighted = result;
			else
				m_normal = result;
		}
		return result;
	}

	void reset()
	{
		delete m_normal;
		m_normal = 0;
		delete m_highlighted;
		m_highlighted = 0;
	}

private:
	NewsScroller *m_scroller;
	Article::Ptr m_article;
	QPixmap      *m_normal;
	QPixmap      *m_highlighted;
};

NewsScroller::NewsScroller(QWidget *parent, ConfigAccess *cfg, const char *name)
	: QFrame(parent, name),
	m_cfg(cfg),
	m_scrollTimer(new QTimer(this)),
	m_dragTimer(new QTimer(this))
{
	setFrameStyle(StyledPanel | Sunken);
	
	m_headlines.setAutoDelete(true);

	connect(m_scrollTimer, SIGNAL(timeout()), SLOT(slotTimeout()));

	connect(m_dragTimer, SIGNAL(timeout()), SLOT(slotStartDrag()));

	reset();
}

QSizePolicy NewsScroller::sizePolicy() const
{
	return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void NewsScroller::clear()
{
	m_headlines.clear();
	reset();
}

void NewsScroller::addHeadline(Article::Ptr article)
{
	for (unsigned int i = 0; i < m_cfg->filters().count(); i++)
		if (m_cfg->filter(i).matches(article))
			return;
			
	m_headlines.append(new Headline(this, article));
}

void NewsScroller::slotScrollText(int distance)
{
	switch (m_cfg->scrollingDirection()) {
		case ConfigAccess::Left:
			m_offset -= distance;
			if (m_offset <= - scrollWidth())
				m_offset = m_cfg->endlessScrolling() ? m_offset + scrollWidth() - m_separator.width() : contentsRect().width();
			break;
		case ConfigAccess::Right:
			m_offset += distance;
			if (m_offset >= contentsRect().width())
				m_offset = (m_cfg->endlessScrolling() ? m_offset + m_separator.width() : 0) - scrollWidth();
			break;
		case ConfigAccess::Up:
		case ConfigAccess::UpRotated:
			m_offset -= distance;
			if (m_offset <= - scrollHeight())
				m_offset = m_cfg->endlessScrolling() ? m_offset + scrollHeight() - m_separator.height() : contentsRect().height() ;
			break;
		case ConfigAccess::Down:
		case ConfigAccess::DownRotated:
			m_offset += distance;
			if (m_offset >= contentsRect().height())
				m_offset = (m_cfg->endlessScrolling() ? m_offset + m_separator.height() : 0) - scrollHeight();
	}
	
	QPoint pt = mapFromGlobal(QCursor::pos());
	
	if (contentsRect().contains(pt))
		updateActive(pt);
	
	repaint(false);
}

void NewsScroller::enterEvent(QEvent *)
{
	/*
	 * The method name scrollingSpeed is a bit misleading here, this part should
	 * make the scrolling slower, but still the scrollingSpeed() has to be
	 * multiplied by two, as it's actually an interval. :-]
	 */
	if (m_cfg->slowedScrolling())
		m_scrollTimer->changeInterval(QMAX(10, m_cfg->scrollingSpeed() * 2));
}	

void NewsScroller::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == QMouseEvent::LeftButton || e->button() == QMouseEvent::MidButton) {
		/*
		 * TODO: This should be made configurable somewhere, not in KCMNewsTicker
		 * but in some nifty system-wide control module (kcminput?).
		 */
		m_dragTimer->start(150, true);
		
		m_dragStart = m_cfg->horizontalScrolling() ? e->x() : e->y();
	
		if (m_activeHeadline)
			m_tempHeadline = m_activeHeadline->article()->headline();
	}
}

void NewsScroller::mouseReleaseEvent(QMouseEvent *e)
{
	m_dragTimer->stop();

	if ((e->button() == QMouseEvent::LeftButton || e->button() == QMouseEvent::MidButton)
			&& m_activeHeadline && m_activeHeadline->article()->headline() == m_tempHeadline
			&& !m_mouseDrag) {
		m_activeHeadline->article()->open();
		m_tempHeadline = QString::null;
	}
	
	if (e->button() == QMouseEvent::RightButton)
		emit(contextMenu());
	
	if (m_mouseDrag) {
		m_mouseDrag = false;
		if (m_cfg->scrollingSpeed())
			m_scrollTimer->start(QMAX(10, m_cfg->scrollingSpeed()));
	}
}

void NewsScroller::mouseMoveEvent(QMouseEvent *e)
{
	if (m_mouseDrag) {
		bool createDrag;
		if (m_cfg->horizontalScrolling()) {
			slotScrollText(m_dragStart - e->x());
			m_dragStart = e->x();
			createDrag = e->y() < 0 || e->y() > height();
		} else {
			slotScrollText(m_dragStart - e->y());
			m_dragStart = e->y();
			createDrag = e->x() < 0 || e->x() > width();
		}
		if (createDrag && m_activeHeadline) {
			KURL::List url;
			url.append(m_activeHeadline->article()->address());
			QDragObject *drag = KURLDrag::newDrag(url, this);
			drag->setPixmap(m_activeHeadline->article()->newsSource()->icon());
			drag->drag();
			m_mouseDrag = false;
			m_dragTimer->stop();
			if (m_cfg->scrollingSpeed())
				m_scrollTimer->start(QMAX(10, m_cfg->scrollingSpeed()));
		}
	}

	if (updateActive(e->pos()))
		repaint(false);
}

void NewsScroller::wheelEvent(QWheelEvent *e)
{
	int delta = -e->delta() / 5;
	
	slotScrollText(delta);

	QFrame::wheelEvent(e);
}

void NewsScroller::leaveEvent(QEvent *)
{
	if (m_cfg->slowedScrolling())
		m_scrollTimer->changeInterval(QMAX(10, m_cfg->scrollingSpeed()));

	if (m_activeHeadline) {
		m_activeHeadline = 0;
		repaint(false);
	}
}

void NewsScroller::drawContents(QPainter *p)
{
	if (!scrollWidth() || // No news and no "No News Available": uninitialized
	    !m_headlines.count()) // Happens when we're currently fetching new news
		return;

	QPixmap buffer(contentsRect().width(), contentsRect().height());
	buffer.fill(m_cfg->backgroundColor());
	int pos = m_offset;

	// Paste in all the separator bitmaps (" +++ ")
	if (horizontal()) {
		while (m_cfg->endlessScrolling() && pos > 0)
			pos -= scrollWidth() - (m_headlines.count() ? 0 : m_separator.width());
		do {
			bitBlt(&buffer, pos, (contentsRect().height() - m_separator.height()) / 2, &m_separator);
			pos += m_separator.width();
		} while (!m_headlines.count() && pos < contentsRect().width());
	} else {
		while (m_cfg->endlessScrolling() && pos > 0)
			pos -= scrollHeight() - (m_headlines.count() ? m_separator.height() : 0);
		do {
			bitBlt(&buffer, (contentsRect().width() - m_separator.width()) / 2, pos, &m_separator);
			pos += m_separator.height();
		} while (!m_headlines.count() && pos < contentsRect().height());
	}
	
	// Now do the headlines themselves
	do {
		QListIterator<Headline> it(m_headlines);
		for(; *it; ++it) {
			if (horizontal()) {
				if (pos + (*it)->width() >= 0)
					bitBlt(&buffer, pos, (contentsRect().height() - (*it)->height()) / 2, (*it)->pixmap(*it == m_activeHeadline, m_cfg->underlineHighlighted()));
				pos += (*it)->width();
				
				if (pos + m_separator.width() >= 0)
					bitBlt(&buffer, pos, (contentsRect().height() - m_separator.height()) / 2, &m_separator);
				pos += m_separator.width();
				
				if (pos >= contentsRect().width())
					break;
			} else {
				if (pos + (*it)->height() >= 0)
					bitBlt(&buffer, (contentsRect().width() - (*it)->width()) / 2, pos, (*it)->pixmap(*it == m_activeHeadline, m_cfg->underlineHighlighted()));
				pos += (*it)->height();
				
				if (pos + m_separator.height() >= 0)
					bitBlt(&buffer, (contentsRect().width() - m_separator.width()) / 2, pos, &m_separator);
				pos += m_separator.height();
				
				if (pos > contentsRect().height())
					break;
			}
		}

		/*
		 * Break out if we reached the bottom of the window before the end of the
		 * list of headlines.
		 */
		if (*it)
			break;
	}
	while (m_cfg->endlessScrolling() && ((m_cfg->horizontalScrolling() && pos < contentsRect().width()) || pos < contentsRect().height()));

	p->drawPixmap(0, 0, buffer);
}

void NewsScroller::reset(bool bSeparatorOnly)
{
	setFont(m_cfg->font());
	
	m_scrollTimer->stop();
	if (m_cfg->scrollingSpeed())
		m_scrollTimer->start(QMAX(10, m_cfg->scrollingSpeed()));
	
	QString sep = m_headlines.count() ? QString::fromLatin1(" +++ ") : i18n(" +++ No News Available +++");

	int w = fontMetrics().width(sep);
	int h = fontMetrics().height();

	if (rotated())
		m_separator.resize(h, w);
	else
		m_separator.resize(w, h);

	m_separator.fill(m_cfg->backgroundColor());
	
	QPainter p(&m_separator);
	p.setFont(font());
	p.setPen(m_cfg->foregroundColor());

	if(rotated()) {
		if (m_cfg->scrollingDirection() == ConfigAccess::UpRotated) {
			p.rotate(90.0);
			p.drawText(0, -fontMetrics().descent(),sep);
		} else {
			p.rotate(-90.0);
			p.drawText(-w, h-fontMetrics().descent(),sep);
		}
	} else
		p.drawText(0, m_separator.height() - fontMetrics().descent(), sep);
	p.end();
	
	if (!bSeparatorOnly)	
		for (QListIterator<Headline> it(m_headlines); *it; ++it)
			(*it)->reset();

	switch (m_cfg->scrollingDirection()) {
		case ConfigAccess::Left:
			m_offset = contentsRect().width();
			break;
		case ConfigAccess::Right:
			m_offset = - scrollWidth();
			break;
		case ConfigAccess::Up:
		case ConfigAccess::UpRotated:
			m_offset = contentsRect().height();
			break;
		case ConfigAccess::Down:
		case ConfigAccess::DownRotated:
			m_offset = - scrollHeight();
	}

	repaint(false);
}

int NewsScroller::scrollWidth() const
{
	int result = (m_headlines.count() + 1) * m_separator.width();
	
	for (QListIterator<Headline> it(m_headlines); *it; ++it)
		result += (*it)->width();
	
	return result;
}

int NewsScroller::scrollHeight() const
{
	int result = (m_headlines.count() + 1) * m_separator.height();
	
	for (QListIterator<Headline> it(m_headlines); *it; ++it)
		result += (*it)->height();
	
	return result;
}

bool NewsScroller::updateActive(const QPoint &pt)
{
	int pos = m_offset;
	
	Headline *headline = 0;
		
	if (m_headlines.count()) {
		if (m_cfg->endlessScrolling()) {
			while (pos > 0)
				if (horizontal())
					pos -= scrollWidth() - m_separator.width();
				else
					pos -= scrollHeight() - m_separator.height();
		}

		do {
			QListIterator<Headline> it(m_headlines);
			for (; (headline = *it); ++it) {
				QRect rect;
				if (horizontal()) {
					pos += m_separator.width();
					rect.moveTopLeft(QPoint(pos, (contentsRect().height() - (*it)->height()) / 2));
					pos += (*it)->width();
				} else {
					pos += m_separator.height();
					rect.moveTopLeft(QPoint((contentsRect().width() - (*it)->width()) / 2, pos));
					pos += (*it)->height();
				}
				rect.setSize(QSize((*it)->width(), (*it)->height()));
				
				if (m_mouseDrag)
					if (horizontal()) {
						rect.setTop(0);
						rect.setHeight(height());
					} else {
						rect.setLeft(0);
						rect.setWidth(width());
					}

				if (rect.contains(pt))
					break;
			}
			if (*it)
				break;
		}
		while (m_cfg->endlessScrolling() && ((m_cfg->horizontalScrolling() && pos < contentsRect().width()) || pos < contentsRect().height()));
	}
	
	if (m_activeHeadline == headline)
		return false;
	
	if ((m_activeHeadline = headline))
		setCursor(KCursor::handCursor());
	else
		unsetCursor();
	
	return true;
}

void NewsScroller::slotTimeout()
{
	if (m_cfg->scrollingSpeed() > 10)
		slotScrollText();
	else
		slotScrollText(11 - m_cfg->scrollingSpeed());
}

void NewsScroller::slotStartDrag()
{
	m_mouseDrag = true;
	m_scrollTimer->stop();
}

#include "newsscroller.moc"

