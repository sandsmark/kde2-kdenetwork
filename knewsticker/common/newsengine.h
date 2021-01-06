/*
 * newsengine.h
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef NEWSENGINE_H
#define NEWSENGINE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "configiface.h"
#include "xmlnewsaccess.h"

#include <qbuffer.h>
#include <qlist.h>
#include <qmap.h>
#include <qpixmap.h>

#include <kio/job.h>
#include <klocale.h>
#include <ksharedptr.h>

class ConfigAccess;
class NewsIconMgr;
class NewsSourceBase;

class QDomDocument;

class KProcess;
class KShellProcess;
class KURL;

class Article : public XMLNewsArticle, public KShared
{
	public:
		typedef KSharedPtr<Article> Ptr;
		typedef QValueList<Ptr> List;

		Article(NewsSourceBase *, const QString & = QString::null,
				const KURL & = KURL());

		bool read() const { return m_read; }
		void setRead(bool read) { m_read = read; }

		NewsSourceBase *newsSource() const { return m_parent; }

		void open();

	private:
		NewsSourceBase *m_parent; // don't use KSharedPtr to avoid circular refs!
		bool m_read;
};

class NewsSourceBase : public XMLNewsSource, public KShared
{
	Q_OBJECT

	public:
		enum Subject {
			Arts = 0, Business, Computers, Games, Health, Home, Recreation,
			Reference, Science, Shopping, Society, Sports, Misc
		};
		struct Data {
			Data(const QString &_name = I18N_NOOP("Unknown"),
					const QString &_sourceFile = QString::null,
					const QString &_icon = QString::null,
					const Subject _subject = Computers,
					const unsigned int _maxArticles = 10,
					bool _enabled = true, bool _isProgram = false,
					const QString &_language = QString::fromLatin1("C"))
			{
				name = _name;
				sourceFile = _sourceFile;
				icon = _icon;
				maxArticles = _maxArticles;
				subject = _subject;
				enabled = _enabled;
				isProgram = _isProgram;
				language = _language;
			}
				
			QString name;
			QString sourceFile;
			QString icon;
			Subject subject;
			unsigned int maxArticles;
			bool enabled;
			bool isProgram;
			QString language;
		};
		typedef KSharedPtr<NewsSourceBase> Ptr;
		typedef QValueList<Ptr> List;

		NewsSourceBase(const Data &, ConfigIface *);

		virtual QString newsSourceName() const;
		QString sourceFile() const { return m_data.sourceFile; }
		unsigned int maxArticles() const { return m_data.maxArticles; }
		QPixmap icon() const { return m_icon; }

		Data data() const { return m_data; }

		Article::List articles() const { return m_articles; }
		Article::Ptr article(const QString &);

		static QString subjectText(const Subject);

	signals:
		void newNewsAvailable(const NewsSourceBase::Ptr &, bool);
		void invalidInput(const NewsSourceBase::Ptr &);

	public slots:
		virtual void retrieveNews() = 0;

	protected slots:
		void slotProcessArticles(XMLNewsSource *, bool);
		void slotGotIcon(const KURL &, const QPixmap &);

	protected:
		Data          m_data;
		QPixmap       m_icon;
		ConfigAccess *m_cfg;
		NewsIconMgr  *m_newsIconMgr;
		Article::List m_articles;
};

class SourceFileNewsSource : public NewsSourceBase
{
	Q_OBJECT
	
	public:
		SourceFileNewsSource(const NewsSourceBase::Data &, ConfigIface *);
	
	public slots:
		virtual void retrieveNews();
};

class ProgramNewsSource : public NewsSourceBase
{
	Q_OBJECT
	
	public:
		enum ErrorCode { NOERR = 0, EPERM, ENOENT, EIO = 5, E2BIG = 7,
			ENOEXEC, EACCESS = 13, ENODEV = 19, ENOSPC = 28, EROFS = 30,
			ENOSYS = 38, ENODATA = 61, ENONET = 64, EPROTO = 71, EDESTADDRREQ = 89,
			ESOCKTNOSUPPORT = 94, ENETUNREACH = 101, ENETRESET = 102,
			ECONNRESET = 104, ETIMEDOUT = 110, ECONNREFUSED, EHOSTDOWN, EHOSTUNREACH,
			ENOEXECBIT = 126, EBADREQ = 400, ENOAUTH, EMUSTPAY, EFORBIDDEN, ENOTFOUND,
			ETIMEOUT = 408, ESERVERE = 500, EHTTPNOSUP = 505 };
	
		ProgramNewsSource(const NewsSourceBase::Data &, ConfigIface *);
		virtual ~ProgramNewsSource();

	public slots:
		virtual void retrieveNews();

	protected slots:
		void slotGotProgramOutput(KProcess *, char *, int);
		void slotProgramExited(KProcess *);
	
	private:
		static QString errorMessage(const ErrorCode);
		
		KShellProcess *m_program;
		QBuffer        m_programOutput;
};

#endif // NEWSENGINE_H
