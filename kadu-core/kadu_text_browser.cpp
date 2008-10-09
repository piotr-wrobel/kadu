/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QFile>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QFileDialog>
#include <QtGui/QMenu>
#include <QtGui/QToolTip>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebHitTestResult>

#include "config_file.h"
#include "debug.h"
#include "html_document.h"
#include "message_box.h"
#include "misc.h"

#include "kadu_text_browser.h"

KaduTextBrowser::KaduTextBrowser(QWidget *parent)
	: QWebView(parent), refreshTimer()
{
	kdebugf();

	setAttribute(Qt::WA_NoBackground);
	setAcceptDrops(false);

 	page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

	connect(page(), SIGNAL(linkClicked(const QUrl &)), this, SLOT(hyperlinkClicked(const QUrl &)));
	connect(page(), SIGNAL(linkHovered(const QString&,  const QString&, const QString&)), this, SLOT(linkHighlighted(const QString &)));

	connect(pageAction(QWebPage::DownloadImageToDisk), SIGNAL(triggered()), this, SLOT(saveImage()));
	connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(reload()));

	kdebugf2();
}

void KaduTextBrowser::refreshLater()
{
	refreshTimer.start(10, true);
}

void KaduTextBrowser::linkHighlighted(const QString & link)
{
	QToolTip::showText(QCursor::pos(), link);
}

void KaduTextBrowser::setSource(const QString &/*name*/)
{
}

void KaduTextBrowser::setMargin(int width)
{
	setContentsMargins(width, width, width, width);
}

void KaduTextBrowser::contextMenuEvent(QContextMenuEvent *event)
{
	contextMenuPos = event->pos();

	QMenu *popupmenu = new QMenu();

	popupmenu->addAction(pageAction(QWebPage::Copy));
// 	popupmenu->addSeparator();
	popupmenu->addAction(pageAction(QWebPage::CopyLinkToClipboard));
// 	popupmenu->addAction(pageAction(QWebPage::DownloadLinkToDisk));
	popupmenu->addSeparator();
	popupmenu->addAction(pageAction(QWebPage::CopyImageToClipboard));
	popupmenu->addAction(pageAction(QWebPage::DownloadImageToDisk));

 	popupmenu->popup(event->globalPos());
 	kdebugf2();
}

void KaduTextBrowser::hyperlinkClicked(const QUrl &anchor) const
{
	const QString &link = anchor.toString();
	if (link.find(HtmlDocument::urlRegExp()) != -1)
	{
		if (link.startsWith("www."))
			openWebBrowser("http://" + link);
		else
			openWebBrowser(link);
	}
	else if (link.find(HtmlDocument::mailRegExp()) != -1)
		openMailClient(link);
	else if (link.find(HtmlDocument::ggRegExp()) != -1)
		openGGChat(link);
}

void KaduTextBrowser::mouseReleaseEvent(QMouseEvent *e)
{
	kdebugf();
	emit mouseReleased(e);
	QWebView::mouseReleaseEvent(e);
}

void KaduTextBrowser::wheelEvent(QWheelEvent *e)
{
	kdebugf();
	emit wheel(e);
	QWebView::wheelEvent(e);
}

void KaduTextBrowser::saveImage()
{
	kdebugf();

	QString image = page()->currentFrame()->hitTestContent(contextMenuPos).imageUrl().toLocalFile();
	if (image.isEmpty())
		return;

	int fdResult;
	QString fileExt = '.' + image.section('.', -1);

	QFileDialog fd(this);
	fd.setMode(QFileDialog::AnyFile);
	fd.setDir(config_file.readEntry("Chat", "LastImagePath"));
	fd.setFilter(QString("%1 (*%2)").arg(qApp->translate("ImageDialog", "Images"), fileExt));
	fd.setLabelText(QFileDialog::FileName, image.section('/', -1));
	fd.setWindowTitle(tr("Save image"));

	while (true)
	{
		if (fd.exec() != QFileDialog::Accepted)
			break;

		if (QFile::exists(fd.selectedFile()))
			if (MessageBox::ask(tr("File already exists. Overwrite?")))
			{
				QFile removeMe(fd.selectedFile());
				if (!removeMe.remove())
				{
					MessageBox::msg(tr("Cannot save image: %1").arg(removeMe.errorString()), false, "Warning");
					continue;
				}
			}
			else
				continue;

		QString dst = fd.selectedFile();
		if (!dst.endsWith(fileExt))
			dst.append(fileExt);

		QFile src(image);
		if (!src.copy(dst))
		{
			MessageBox::msg(tr("Cannot save image: %1").arg(src.errorString()), false, "Warning");
			continue;
		}

		config_file.writeEntry("Chat", "LastImagePath", fd.directory().absolutePath());
		break;
	}
}
