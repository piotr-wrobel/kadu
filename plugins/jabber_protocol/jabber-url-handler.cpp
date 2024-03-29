/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2012, 2013 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * %kadu copyright end%
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "jabber-url-handler.h"
#include "moc_jabber-url-handler.cpp"

#include "accounts/account-manager.h"
#include "accounts/account.h"
#include "chat/chat-manager.h"
#include "chat/chat-storage.h"
#include "chat/type/chat-type-contact.h"
#include "contacts/contact-manager.h"
#include "contacts/contact-set.h"
#include "contacts/contact.h"
#include "dom/dom-processor-service.h"
#include "dom/dom-processor.h"
#include "dom/ignore-links-dom-visitor.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "misc/misc.h"
#include "status/status-container.h"
#include "url-handlers/simple-url-expander.h"
#include "widgets/chat-widget/chat-widget-manager.h"

#include <QtGui/QCursor>
#include <QtWidgets/QMenu>

JabberUrlHandler::JabberUrlHandler(QObject *parent) : QObject{parent}
{
    // Based on: http://mail.jabber.org/pipermail/xmppwg/2005-February/002261.html
    // (RFC5122 - 3.3, XEP-0147)
    // "(?:xmpp|jabber):" - if we ever need to handle jabber: links

    m_jabberRegExp = QRegExp(
        "\\b"
        "xmpp:"
        "(?://([^@ ]+)@([^/?# ]+)/?)?"                   // auth-xmpp
        "(?:(?:([^@ ]+)@)?([^/?# ]+)(?:/([^?# ]+))?)?"   // path-xmpp
        "(?:\\?([^&# ]+)"                                // querytype
        "(&[^# ]+)?)?"                                   // pair, will need to be reparsed, later
        "(?:#(\\S*))?"                                   // fragment
        "\\b");
    // Reparse pair with: "&([^=]+)=([^&]+)"
}

JabberUrlHandler::~JabberUrlHandler()
{
}

void JabberUrlHandler::setAccountManager(AccountManager *accountManager)
{
    m_accountManager = accountManager;
}

void JabberUrlHandler::setChatManager(ChatManager *chatManager)
{
    m_chatManager = chatManager;
}

void JabberUrlHandler::setChatStorage(ChatStorage *chatStorage)
{
    m_chatStorage = chatStorage;
}

void JabberUrlHandler::setChatWidgetManager(ChatWidgetManager *chatWidgetManager)
{
    m_chatWidgetManager = chatWidgetManager;
}

void JabberUrlHandler::setContactManager(ContactManager *contactManager)
{
    m_contactManager = contactManager;
}

void JabberUrlHandler::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

bool JabberUrlHandler::isUrlValid(const QByteArray &url)
{
    if (url == "xmpp:")
        return false;

    return m_jabberRegExp.exactMatch(QString::fromUtf8(url));
}

void JabberUrlHandler::openUrl(UrlOpener *urlOpener, const QByteArray &url, bool disableMenu)
{
    Q_UNUSED(urlOpener);

    QVector<Account> jabberAccounts = m_accountManager->byProtocolName("jabber");
    if (jabberAccounts.isEmpty())
        return;

    QString jabberId = QString::fromUtf8(url);
    if (jabberId.startsWith(QStringLiteral("jid:")))
    {
        jabberId.remove(0, 3);
        jabberId.remove(QRegExp("/*"));
    }

    if (jabberAccounts.count() == 1 || disableMenu)
    {
        const Contact &contact = m_contactManager->byId(jabberAccounts[0], jabberId, ActionCreateAndAdd);
        const Chat &chat = ChatTypeContact::findChat(m_chatManager, m_chatStorage, contact, ActionCreateAndAdd);
        if (chat)
        {
            m_chatWidgetManager->openChat(chat, OpenChatActivation::Activate);
            return;
        }
    }
    else
    {
        QMenu menu;

        QStringList ids;
        for (auto account : jabberAccounts)
        {
            ids.clear();
            ids.append(account.id());
            ids.append(jabberId);

            menu.addAction(m_iconsManager->iconByPath(account.statusContainer()->statusIcon()), account.id())
                ->setData(ids);
        }

        connect(&menu, SIGNAL(triggered(QAction *)), this, SLOT(accountSelected(QAction *)));

        menu.exec(QCursor::pos());
    }
}

void JabberUrlHandler::accountSelected(QAction *action)
{
    const QStringList &ids = action->data().toStringList();

    if (ids.count() != 2)
        return;

    const Account &account = m_accountManager->byId("jabber", ids[0]);
    if (!account)
        return;

    const Contact &contact = m_contactManager->byId(account, ids[1], ActionCreateAndAdd);
    const Chat &chat = ChatTypeContact::findChat(m_chatManager, m_chatStorage, contact, ActionCreateAndAdd);
    m_chatWidgetManager->openChat(chat, OpenChatActivation::Activate);
}
