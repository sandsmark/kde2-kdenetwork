/*
 * xmlnewsaccess.cpp
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "xmlnewsaccess.h"

#include <kcharsets.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kio/job.h>

#include <qbuffer.h>
#include <qdom.h>
#include <qregexp.h>

XMLNewsArticle::XMLNewsArticle(const QString &headline, const KURL &address)
	: m_headline(headline),
	m_address(address)
{
}

XMLNewsArticle &XMLNewsArticle::operator=(const XMLNewsArticle &other)
{
	m_headline = other.m_headline;
	m_address = other.m_address;
	return *this;
}

bool XMLNewsArticle::operator==(const XMLNewsArticle &a)
{
	return m_headline == a.headline() && m_address == a.address();
}

XMLNewsSource::XMLNewsSource() : QObject(),
 	m_name(QString::null),
	m_link(QString::null),
	m_description(QString::null)
{
	m_downloadData.open(IO_WriteOnly);
}

void XMLNewsSource::loadFrom(const KURL &url)
{
	m_downloadData.reset();
	
	KIO::Job *job = KIO::get(url.url(), true, false);
	connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
			SLOT(slotData(KIO::Job *, const QByteArray &)));
	connect(job, SIGNAL(result(KIO::Job *)), SLOT(slotResult(KIO::Job *)));
}

void XMLNewsSource::slotData(KIO::Job *, const QByteArray &data)
{
	m_downloadData.writeBlock(data.data(), data.size());
}

void XMLNewsSource::slotResult(KIO::Job *job)
{
	processData(m_downloadData.buffer(), !job->error());
}

void XMLNewsSource::processData(const QByteArray &data, bool okSoFar)
{
	bool validContent = okSoFar;
	
	if (okSoFar) {
		QString xmlData(data);
		xmlData.replace(QRegExp(QString::fromLatin1("&")), QString::fromLatin1("&amp;"));
	
		QDomDocument domDoc;
		if (validContent = domDoc.setContent(xmlData)) {
			QDomNode channelNode = domDoc.documentElement().namedItem(QString::fromLatin1("channel"));
	
			m_name = channelNode.namedItem(QString::fromLatin1("title")).toElement().text().simplifyWhiteSpace();
			m_link = channelNode.namedItem(QString::fromLatin1("link")).toElement().text().simplifyWhiteSpace();
			m_description = channelNode.namedItem(QString::fromLatin1("description")).toElement().text().simplifyWhiteSpace();

			QDomNodeList items = domDoc.elementsByTagName(QString::fromLatin1("item"));
			m_articles.clear();
			QDomNode itemNode;
			QString headline, address;
			for (unsigned int i = 0; i < items.count(); i++) {
				itemNode = items.item(i);
				headline = decodeEntities(itemNode.namedItem(QString::fromLatin1("title")).toElement().text());
				address = decodeEntities(itemNode.namedItem(QString::fromLatin1("link")).toElement().text());
				m_articles.append(XMLNewsArticle(headline, address));
			}
		}
	}

	emit loadComplete(this, validContent);
}

QString XMLNewsSource::decodeEntities(const QString &s) const
{
	QString result = s;
	result.replace(QRegExp(QString::fromLatin1("&amp;")), QString::fromLatin1("&"));
	for (int p = result.find(QString::fromLatin1("&")); p >= 0; p = result.find(QString::fromLatin1("&"), p)) {
		int q = result.find(QString::fromLatin1(";"), p++);
		if (q != -1)
			result.replace(p - 1, q - p + 2, KGlobal::charsets()->fromEntity(result.mid(p, q - p)));
	}
	return result;
}

#include "xmlnewsaccess.moc"
