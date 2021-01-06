/*
 * newssourcedlgimpl.h
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef NEWSSOURCEDLGIMPL_H
#define NEWSSOURCEDLGIMPL_H
#include "newssourcedlg.h"
#include "newsengine.h"

#include <kurl.h>

#include <qpixmap.h>

class XMLNewsSource;
class NewsIconMgr;

class NewsSourceDlgImpl : public NewsSourceDlg
{ 
	Q_OBJECT

	public:
		NewsSourceDlgImpl(QWidget * = 0, const char * = 0, bool = FALSE, WFlags = 0);
		~NewsSourceDlgImpl();

		void setup(const NewsSourceBase::Data &);

	signals:
		void newsSource(const NewsSourceBase::Data &);

	protected slots:
		void slotCancelClicked();
		void slotOkClicked();
		void slotSourceFileChanged();
		void slotSuggestClicked();
		void slotModified();
		void slotLoadComplete(XMLNewsSource *, bool);
		void slotGotIcon(const KURL &, const QPixmap &);
		void showSuggestedValues();
		KURL polishedURL(const KURL &) const;
		bool validateURL(const KURL &);
	
	private:
		bool           m_modified;
		bool           m_gotSourceFile;
		bool           m_gotIcon;
		bool           m_succeeded;
		QPixmap        m_icon;
		KURL           m_iconURL;
		QString        m_origCaption;
		XMLNewsSource  *m_xmlSrc;
		NewsIconMgr    *m_newsIconMgr;
};

#endif // NEWSSOURCEDLGIMPL_H
