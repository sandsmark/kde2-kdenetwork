/*
 * kntsrcfilepropsdlg.h
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef KNTSRCFILEPROPSDLG_H
#define KNTSRCFILEPROPSDLG_H

#include <klibloader.h>
#include <kpropsdlg.h>

class XMLNewsSource;
class QLabel;
class KURLLabel;
class KntSrcFilePropsDlgWidget;
class NewsIconMgr;
class QListBoxItem;

class KntSrcFilePropsFactory : public KLibFactory
{
	Q_OBJECT
	
	public:
		virtual QObject *createObject(QObject * = 0, const char * = 0,
				const char * = "QObject", const QStringList & = QStringList());
};

class KntSrcFilePropsDlg : public KPropsDlgPlugin
{
	Q_OBJECT
	
	public:
		KntSrcFilePropsDlg(KPropertiesDialog *);
		virtual ~KntSrcFilePropsDlg();
	
	protected slots:
		void slotOpenURL(const QString &);
		void slotConstructUI(XMLNewsSource *, bool);
		void slotGotIcon(const KURL &, const QPixmap &);
		void slotClickedArticle(QListBoxItem *);
	
	private:
		XMLNewsSource            *m_xmlSrc;
		KntSrcFilePropsDlgWidget *m_child;
		NewsIconMgr              *m_newsIconMgr;
};

#endif // KNTSRCFILEPROPSDLG_H
