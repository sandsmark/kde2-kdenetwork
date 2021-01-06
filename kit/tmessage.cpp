// tmessage.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include <kaction.h>
#include <kcolorbtn.h>
#include <kcolordlg.h>
#include <kdialog.h>
#include <kfile.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kmenubar.h>
#include <knotifyclient.h>
#include <kstddirs.h>
#include <kwin.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qclipboard.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qvbox.h>

#include "aim.h"
#include "kitapp.h"
#include "kithtmlpart.h"
#include "tmessage.h"

// helper functions
inline bool tagIsOpen(const QString &html, const QString &tag)
{
	return html.contains("<" + tag, false) > html.contains("</" + tag + ">", false);
}

#define SKIP_TO_GT {while(newstr[++i] != '>' && i < newstr.length());}

#define SKIP_TO_GT_CHECK_QUOTE {int quotecount = 0;\
					while(newstr[++i] != '>' && i < newstr.length()){\
						if(newstr[i] == '"') quotecount++;}\
					if(quotecount % 2 == 1){\
						newstr.replace(i, 1, "\">");\
						i++;}}

#define KILL_LT { newstr.replace(i, 1, "&lt;"); i += 3; }

void parseAndProcess(QString &newstr)
{
	bool bodyUsed = false;
	bool divUsed = false;
	bool pUsed = false;

	for(unsigned i = 0; i < newstr.length(); i++)
	{
		if(newstr[i] == '>')
		{
			newstr.replace(i, 1, "&gt;");
			i += 3;
		}
		else if(newstr[i] == '<')
		{
			if(newstr.mid(i, 3).upper() == "<A ") SKIP_TO_GT_CHECK_QUOTE
			else if(newstr.mid(i, 3).upper() == "<B>") i += 2;
			else if(newstr.mid(i, 5).upper() == "<BIG>") i += 4;
			else if(newstr.mid(i, 4).upper() == "<BR>") i += 3;
			else if(newstr.mid(i, 6).upper() == "<CODE>") i += 5;
			else if(newstr.mid(i, 3).upper() == "<EM") SKIP_TO_GT_CHECK_QUOTE
			else if(newstr.mid(i, 5).upper() == "<FONT") SKIP_TO_GT_CHECK_QUOTE
			else if(newstr.mid(i, 3).upper() == "<I>") i += 2;
			else if(newstr.mid(i, 7).upper() == "<SMALL>") i += 6;
			else if(newstr.mid(i, 4).upper() == "<TT>") i += 3;
			else if(newstr.mid(i, 3).upper() == "<U>") i += 2;
			else if(!divUsed && newstr.mid(i, 4).upper() == "<DIV")
			{
				divUsed = true;
				SKIP_TO_GT_CHECK_QUOTE
			}
			else if(!pUsed && newstr.mid(i, 2).upper() == "<P")
			{
				pUsed = true;
				SKIP_TO_GT_CHECK_QUOTE
			}
			else if(!bodyUsed && newstr.mid(i, 5).upper() == "<BODY")
			{
				bodyUsed = true;
				SKIP_TO_GT_CHECK_QUOTE
			}
			else if(newstr.mid(i, 7).upper() == "</HTML>") KILL_LT
			else if(newstr.mid(i, 7).upper() == "</DIV>") KILL_LT
			else if(newstr.mid(i, 7).upper() == "</BODY>") KILL_LT
			else if(newstr.mid(i, 2).upper() == "</") SKIP_TO_GT
			else KILL_LT;
		}
	}
}
QString stripTags(const QString &oldstr)
{
	QString newstr = oldstr;
	int bodyPos;

	// 1) cut first <html> , last </html> tags
	bodyPos = newstr.find( QRegExp("<html>", false) );
	if(bodyPos != -1)
	{
		newstr.replace(bodyPos, 6, "");
		bodyPos = newstr.findRev( QRegExp("</html>", false) );
		if(bodyPos != -1) newstr.replace(bodyPos, 7, "");
	}
	// 2) cut <p> tags
	newstr.replace( QRegExp("<p>", false), "" );
	// 2b) replace </p> with <br>
	newstr.replace( QRegExp("</p>", false), "<br>" );
	// 3) replace <body> with <div>, inserting a STYLE= value to apply text and
	// background colors
	bodyPos = newstr.find("<BODY BGCOLOR=");
	if(bodyPos != -1) newstr.replace(bodyPos, 15, "<tnof STYLE=\"font-style: normal; background: ");
	int bodyEndPos = newstr.find(">", bodyPos);
	int fontPos = newstr.find("<FONT");
	if(fontPos != -1)
	{
		newstr.replace(fontPos, 5, "");
		newstr.replace(bodyEndPos, 1, "");
	}
	if(bodyPos != -1) newstr.replace(bodyPos, 5, "<FONT");

	// 3b) remove </body>
	newstr.replace( QRegExp("</body>", false), "" );
	// 4) make sure the allowed tags are balanced, inserting a </tag> at the end of
	// the message to balance if needed.
#define CLOSE_TAG(a) if(tagIsOpen(newstr, a)) newstr += "</" a ">";
	CLOSE_TAG("a");
	CLOSE_TAG("b");
	CLOSE_TAG("big");
	CLOSE_TAG("code");
	CLOSE_TAG("em");
	CLOSE_TAG("font");
	CLOSE_TAG("i");
	CLOSE_TAG("small");
	CLOSE_TAG("tt");
	CLOSE_TAG("u");
#undef CLOSE_TAG
	// 4b) check allowed tags, make sure quotes are balanced within them
	// 5) replace any "<" or ">" that's not part of an allowed tag with &lt; or &gt;
	parseAndProcess(newstr);
	return newstr;
}

// ****************************
// * class TMessage -- public *
// ****************************
TMessage::TMessage(const QString &display, const QString &internal,
	const QString &mine, bool log, bool tms, bool icq,
	const QColor &fg, const QColor &bg, bool Bold,
	const char *b)
	: KMainWindow(0, b)
{
	displayName = display;
	name = internal;
	myName = mine;
	setCaption(displayName);
	
	// away messages
	isAway = false;
	awayMessage = QString::null;
	awayMessageSent = false;
	useSound = true;

	// logging
	logFile = 0;
	isLogging = log;
	isTimestamping = tms;

	// colors
	fore = fg;
	back = bg;
	bold = Bold;

	isICQ = icq;
	
	// menu & toolbar
	initActions();
	
	// the stuff in the middle
	centralWidget = new QVBox(this);
	centralWidget->setMargin(KDialog::marginHint());
	centralWidget->setSpacing(KDialog::spacingHint());
	setCentralWidget(centralWidget);

	htmlpart = new KitHTMLPart(centralWidget, 0, this);
	connect(htmlpart, SIGNAL(urlClicked(const QString &)), this, SLOT(urlClicked(const QString &)));
	// Will this fix the cyrillics?
	htmlpart->setEncoding(QTextCodec::codecForLocale()->name(), true);
	// htmlpart->setCharset(something or other)
	htmlpart->setStandardFont(KGlobalSettings::generalFont().rawName());
	htmlpart->setFixedFont(KGlobalSettings::fixedFont().rawName());
	htmlpart->autoloadImages(false);
	htmlpart->enableJScript(false);
	htmlpart->enableJava(false);
	htmlpart->enablePlugins(false);
	htmlpart->show();
	initializeHTML();
	
	QHBox *bottomRow = new QHBox(centralWidget);
	editFore = new KColorButton(bottomRow);
	editBack = new KColorButton(bottomRow);
	edit = new KLineEdit(bottomRow);
	bottomRow->setMinimumHeight(edit->sizeHint().height());
	bottomRow->setMaximumHeight(edit->sizeHint().height());
	editFore->setMaximumWidth(edit->sizeHint().height());
	editBack->setMaximumWidth(edit->sizeHint().height());
	connect(edit, SIGNAL(returnPressed()), this, SLOT(send()) );
	connect( editFore, SIGNAL(changed(const QColor &)), this, SLOT(forePressed2(const QColor &)) );
	connect( editBack, SIGNAL(changed(const QColor &)), this, SLOT(backPressed2(const QColor &)) );

	centralWidget->setMinimumHeight(KDialog::marginHint() * 2 + KDialog::spacingHint() + htmlpart->view()->minimumHeight() + bottomRow->minimumHeight());
	resize(400,200 >= centralWidget->minimumHeight() ? 200 : centralWidget->minimumHeight());
	setColors(fore, back, bold);
	setIcon(DesktopIcon("kit"));

	setFocusProxy(edit);
	htmlpart->view()->viewport()->setFocusProxy(edit);
}
TMessage::~TMessage()
{
	if(logFile != 0) delete logFile;
}
// TMessage -- public slots
void TMessage::messageIn(const QString &message, bool isAuto)
{
	if(useSound)
		KNotifyClient::event("Message Received");
	updateHTML(message, displayName, "0000FF", isAuto);
	sendAuto();
}
void TMessage::setLogging(bool log)
{
	isLogging = log;
	if(!isLogging && logFile)
	{
		delete logFile;
		logFile = 0;
	}
	aLogging->setChecked(isLogging);
}
void TMessage::setTimestamping(bool stamp)
{
	isTimestamping = stamp;
	aTimestamping->setChecked(isTimestamping);
}
void TMessage::setAway(bool away, const QString &message)
{
	if( !(isAway) || (awayMessage != message) ) awayMessageSent = false;
	isAway = away;
	awayMessage = message;
}
void TMessage::setMyName(const QString &newname)
{
	myName = newname;
}
void TMessage::setColors(const QColor &fg, const QColor &bg, bool Bold)
{
	back = bg;
	fore = fg;
	bold = Bold;
	editFore->setColor(fore);
	editBack->setColor(back);
}
void TMessage::setICQ(bool icq)
{
	isICQ = icq;
	aICQ->setChecked(isICQ);
}
// * class TMessage -- protected *
void TMessage::closeEvent(QCloseEvent *)
{
	emit closing(name);
}
void TMessage::resizeEvent(QResizeEvent *)
{
	if(htmlpart && htmlpart->view())
		htmlpart->view()->ensureVisible(0, htmlpart->view()->contentsHeight());
}
// * class TMessage -- protected slots
void TMessage::send(void)
{
	// avoid doubles
	disconnect(edit, SIGNAL(returnPressed()), this, SLOT(send()) );
	QString message = edit->text();
	edit->clear();
	connect(edit, SIGNAL(returnPressed()), this, SLOT(send()) );
	// don't send empty messages
	if(message == "") return;
	// turn into AOL-HTML
	if(isICQ)
	{
		emit messageOut(message, name, false);
		updateHTML(message, myName, "FF0000", false);
	}
	else
	{
		QString aolHTML;
		aolHTML.sprintf("<HTML><BODY BGCOLOR=\"#%02X%02X%02X\"><FONT COLOR=\"#%02X%02X%02X\">",
			back.red(), back.green(), back.blue(), fore.red(), fore.green(), fore.blue());
		if(bold) aolHTML += "<B>";
		aolHTML += message;
		if(bold) aolHTML += "</B>";
		aolHTML += "</FONT></BODY></HTML>";
		emit messageOut(aolHTML, name, false);
		updateHTML(aolHTML, myName, "FF0000", false);
	}
	// press on
	if(useSound)
		KNotifyClient::event("Message Sent");
}
void TMessage::sendAuto(void)
{
	if(awayMessageSent) return;
	if(!isAway) return;
	awayMessageSent = true;
	if(useSound)
		KNotifyClient::event("Message Sent");
	emit messageOut(awayMessage, name, true);
	updateHTML(awayMessage, myName, "FF0000", true);
}
void TMessage::closePressed(void)
{
	QApplication::sendEvent(this, new QCloseEvent());
}
void TMessage::clearPressed(void)
{
	initializeHTML();
}
void TMessage::infoPressed(void)
{
	emit getInfo(name);
}
void TMessage::loggingPressed(void)
{
	setLogging(!isLogging);
}
void TMessage::timestampingPressed(void)
{
	setTimestamping(!isTimestamping);
}
void TMessage::soundPressed(void)
{
	useSound = !useSound;
}
void TMessage::warningPressed(void)
{
	emit warningOut(name, false);
}
void TMessage::anonWarningPressed(void)
{
	emit warningOut(name, true);
}
void TMessage::forePressed(void)
{
	QColor newColor = fore;
	if(KColorDialog::Accepted == KColorDialog::getColor(newColor, this))
		setColors(newColor, back, bold);
}
void TMessage::backPressed(void)
{
	QColor newColor = back;
	if(KColorDialog::Accepted == KColorDialog::getColor(newColor, this))
		setColors(fore, newColor, bold);
}
void TMessage::forePressed2(const QColor &a)
{
	setColors(a, back, bold);
}
void TMessage::backPressed2(const QColor &b)
{
	setColors(fore, b, bold);
}
void TMessage::boldPressed(void)
{
	setColors(fore, back, !bold);
}
void TMessage::icqPressed(void)
{
	setICQ(!isICQ);
}
// *****************************
// * class TMessage -- private *
// *****************************
void TMessage::initActions(void)
{
	// Define Actions
#define NEWACTION(NAME, ICON, TEXT, SSLOT) NAME = new KAction(TEXT, SmallIconSet(ICON), 0, this, SLOT(SSLOT(void)), this); NAME->setIcon(ICON);
#define NEWTOGGLEACTION(NAME, ICON, TEXT, SSLOT) NAME = new KToggleAction(TEXT, SmallIconSet(ICON), 0, this, SLOT(SSLOT(void)), this); NAME->setIcon(ICON);
	NEWACTION(aClear, "eraser", i18n("C&lear"), clearPressed);
	NEWACTION(aClose, "exit", i18n("&Close"), closePressed);
	NEWACTION(aInfo, "kit_info", i18n("&Information"), infoPressed);
	NEWACTION(aWarning, "kit_warning", i18n("&Warn"), warningPressed);
	NEWACTION(aAnonWarning, "kit_anonwarning", i18n("&Anonymous Warn"), anonWarningPressed);
	NEWTOGGLEACTION(aLogging, "kit_logging", i18n("&Logging"), loggingPressed);
	NEWTOGGLEACTION(aTimestamping, "kit_timestamping", i18n("&Timestamping"), timestampingPressed);
	NEWTOGGLEACTION(aSound, "kit_sound", i18n("&Sound"), soundPressed);
	NEWTOGGLEACTION(aICQ, "kit_icq", i18n("ICQ &Mode"), icqPressed);
	NEWACTION(aFore, "pencil", i18n("&Text color..."), forePressed);
	NEWACTION(aBack, "fill", i18n("Bac&kground color..."), backPressed);
	NEWTOGGLEACTION(aBold, "kit_bold", i18n("&Bold text"), boldPressed);
#undef NEW_ACTION
	aLogging->setChecked(isLogging);
	aTimestamping->setChecked(isTimestamping);
	aSound->setChecked(useSound);
	aICQ->setChecked(isICQ);

	// Place actions into menus
	// User (replaces file menu)
	KPopupMenu *user = new KPopupMenu(menuBar());
	aInfo->plug(user);
		user->insertSeparator();
	aWarning->plug(user);
	aAnonWarning->plug(user);
		user->insertSeparator();
	aClear->plug(user);
		user->insertSeparator();
	aClose->plug(user);

	// Settings
	KPopupMenu *settings = new KPopupMenu(menuBar());
	aFore->plug(settings);
	aBack->plug(settings);
	aBold->plug(settings);
		settings->insertSeparator();
	aLogging->plug(settings);
	aTimestamping->plug(settings);
	aSound->plug(settings);
	aICQ->plug(settings);

	// place menus into menubar
	menuBar()->insertItem(i18n("&User"), user);
	menuBar()->insertItem(i18n("&Settings"), settings);
	
	// place actions into toolbar
	aInfo->plug(toolBar(0));
		toolBar(0)->insertSeparator();
	aWarning->plug(toolBar(0));
	aAnonWarning->plug(toolBar(0));
		toolBar(0)->insertSeparator();
	aClear->plug(toolBar(0));
		toolBar(0)->insertSeparator();
	aLogging->plug(toolBar(0));
	aTimestamping->plug(toolBar(0));
	aSound->plug(toolBar(0));
		toolBar(0)->insertSeparator();
	aICQ->plug(toolBar(0));

	aFore->plug(toolBar("style"));
	aBack->plug(toolBar("style"));
	aBold->plug(toolBar("style"));
}
void TMessage::openLogFile(void)
{
	if(logFile != 0) delete logFile;
	logFile = 0;

	QString logPath = KGlobal::dirs()->saveLocation("data", KIT_NAME + "/" + myName + "/", true);
	if(logPath == QString::null)
	{
		KMessageBox::error(0, i18n("Unable to open logfile directory"));
		return;
	}
	QString logFilename = logPath + name + ".html";

	// open file for appending, to add the new message
	logFile = new QFile(logFilename);
	bool exists = logFile->exists();
	if(logFile->open(IO_WriteOnly | IO_Append))
	{
		if(!exists)
		{
			QString header = i18n("<HTML>\n<HEAD>\n<TITLE>Conversations with %1</TITLE>\n<BODY>\n").arg(displayName);
			QTextStream stream(logFile);
			stream << header;
		}
	}
	else
	{
		QString msg = i18n("Unable to open %1 for appending.");
		KMessageBox::error(0, msg.arg(logFilename));
		delete logFile;
		logFile = 0;
	}
}
void TMessage::initializeHTML()
{
	conversationBuffer = QString("<HTML><HEAD><TITLE>") + displayName + QString("</TITLE></HEAD><BODY BGCOLOR=\"#FFFFFF\">");
	conversationLength = 0;
	htmlpart->begin();
	// do this so khtml picks up the default encoding
	htmlpart->write(conversationBuffer);
}
void TMessage::updateHTML(QString message, QString sender, QString HTMLColor, bool isAuto)
{
	// add HTML wrapping paper to the message
	QString labelledMessage("<b><font color=\"#");
	labelledMessage += HTMLColor;
	labelledMessage += "\"> ";
	labelledMessage += sender;
	labelledMessage += " </font></b>";
	if(isAuto) labelledMessage += i18n("<font color=\"#FF0000\"> [AUTO] </font>");
	labelledMessage += ": ";
	QString stripped = stripTags(message);
	labelledMessage += stripped;
	// timestamp
	QString timeStr;
	if(isTimestamping)
	{
		QDateTime time = QDateTime::currentDateTime();
		timeStr = QString("<b>[%1 %2 - %3]</b>")
		           .arg(time.date().day())
		           .arg(time.date().monthName(time.date().month()))
		           .arg(time.time().toString());
	}
	// HTMLize
	QString HTMLMessage("<p>");
	HTMLMessage += timeStr;
	HTMLMessage += labelledMessage;
	HTMLMessage += QString("</p>\n");
	// add the message to the logfile
	if(isLogging)
	{
		if(!logFile) openLogFile();
		if(logFile)
		{
			QTextStream stream(logFile);
			stream << HTMLMessage;
		}
	}
	htmlpart->write(HTMLMessage);
	// scroll to the new text, and beat khtmlpart until it cooperates
	htmlpart->view()->layout();
	htmlpart->view()->ensureVisible(0, htmlpart->view()->contentsHeight());
	htmlpart->view()->updateContents(htmlpart->view()->contentsX(), htmlpart->view()->contentsY(),
		htmlpart->view()->contentsWidth(), htmlpart->view()->contentsHeight());
}

void TMessage::urlClicked(const QString &url)
{
	kapp->invokeBrowser(url);
}

#include "tmessage.moc"
