/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2011 Sławomir Stępień (s.stepien@interia.pl)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "parser.h"
#include "moc_parser.cpp"

#include "accounts/account-manager.h"
#include "buddies/group.h"
#include "chat/model/chat-data-extractor.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "contacts/contact.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "misc/misc.h"
#include "misc/paths-provider.h"
#include "parser/parser-token.h"
#include "status/status-container-manager.h"
#include "status/status-container.h"
#include "status/status-type-data.h"
#include "status/status-type-manager.h"
#include "status/status-type.h"
#include "talkable/talkable-converter.h"

#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QStack>
#include <QtCore/QVariant>
#include <QtGui/QTextDocument>
#include <QtNetwork/QHostAddress>
#include <QtWidgets/QApplication>

#define SEARCH_CHARS "%[{\\$@#}]"
#define EXEC_SEARCH_CHARS "`\'"

// PT_CHECK_FILE_EXISTS and PT_CHECK_FILE_NOT_EXISTS checks need space to be encoded,
// and encoding search chars shouldn't hurt also
#define ENCODE_INCLUDE_CHARS " " SEARCH_CHARS EXEC_SEARCH_CHARS

Q_GLOBAL_STATIC(QSet<QChar>, searchChars)

static void prepareSearchChars(Configuration *configuration, bool forceExecSeachChars = false)
{
    QSet<QChar> &chars = *searchChars();
    if (chars.isEmpty())
        for (auto c : QString(SEARCH_CHARS))
            chars.insert(c);

    bool allowExec = forceExecSeachChars ||
                     configuration->deprecatedApi()->readBoolEntry("General", "AllowExecutingFromParser", false);
    for (auto c : QString(EXEC_SEARCH_CHARS))
        if (allowExec)
            chars.insert(c);
        else
            chars.remove(c);
}

Parser::Parser(QObject *parent) : QObject{parent}
{
}

Parser::~Parser()
{
}

void Parser::setChatDataExtractor(ChatDataExtractor *chatDataExtractor)
{
    m_chatDataExtractor = chatDataExtractor;
}

void Parser::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void Parser::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void Parser::setStatusContainerManager(StatusContainerManager *statusContainerManager)
{
    m_statusContainerManager = statusContainerManager;
}

void Parser::setStatusTypeManager(StatusTypeManager *statusTypeManager)
{
    m_statusTypeManager = statusTypeManager;
}

void Parser::setTalkableConverter(TalkableConverter *talkableConverter)
{
    m_talkableConverter = talkableConverter;
}

QString Parser::escape(const QString &string)
{
    prepareSearchChars(m_configuration, true);

    QString escaped;
    escaped.reserve(string.size() * 2);
    QSet<QChar> &chars = *searchChars();
    for (auto c : string)
    {
        if (chars.contains(c))
            escaped.append('\'');
        escaped.append(c);
    }

    return escaped;
}

bool Parser::registerTag(const QString &name, TalkableTagCallback func)
{
    if (m_registeredTalkableTags.contains(name))
    {
        return false;
    }

    if (m_registeredObjectTags.contains(name))
    {
        return false;
    }

    m_registeredTalkableTags.insert(name, func);
    return true;
}

bool Parser::unregisterTag(const QString &name)
{
    if (!m_registeredTalkableTags.contains(name))
        return false;

    m_registeredTalkableTags.remove(name);
    return true;
}

bool Parser::registerObjectTag(const QString &name, ObjectTagCallback func)
{
    if (m_registeredObjectTags.contains(name))
        return false;

    if (m_registeredTalkableTags.contains(name))
        return false;

    m_registeredObjectTags.insert(name, func);
    return true;
}

bool Parser::unregisterObjectTag(const QString &name)
{
    if (!m_registeredObjectTags.contains(name))
        return false;

    m_registeredObjectTags.remove(name);
    return true;
}

QString Parser::executeCmd(const QString &cmd)
{
    QString s(cmd);
    // TODO: check if Qt escapes these
    s.remove(QRegExp("`|>|<"));

    QProcess executor;
    executor.start(s);
    executor.closeWriteChannel();

    QString ret;
    if (executor.waitForFinished())
        ret = executor.readAll();

    return ret;
}

bool Parser::isActionParserTokenAtTop(
    const QStack<ParserToken> &parseStack, const QVector<ParserTokenType> &acceptedTokens)
{
    bool found = false;
    QStack<ParserToken>::const_iterator begin = parseStack.constBegin();
    QStack<ParserToken>::const_iterator it = parseStack.constEnd();

    while (it != begin)
    {
        --it;
        ParserTokenType t = it->type();

        if (acceptedTokens.contains(t))
        {
            found = true;
            break;
        }

        if (PT_STRING == t)
            continue;

        break;
    }

    return found;
}

ParserToken Parser::parsePercentSyntax(const QString &s, int &idx, const Talkable &talkable, ParserEscape escape)
{
    ParserToken pe;
    pe.setType(PT_STRING);

    Chat chat = m_talkableConverter->toChat(talkable);
    Buddy buddy = m_talkableConverter->toBuddy(talkable);
    Contact contact = m_talkableConverter->toContact(talkable);

    switch (s.at(idx).toAscii())
    {
    // 'o' does not work so we should just ignore it
    // see bug #2199
    case 'o':
    // 't' was removed in commit 48d3cd65 during 0.9 (aka 0.6.6) release cycle
    case 't':
        ++idx;
        break;
    case 's':
        ++idx;

        if (buddy && buddy.isBlocked())
            pe.setContent(QCoreApplication::translate("@default", "Blocked"));
        else if (contact)
        {
            if (contact.isBlocking())
                pe.setContent(QCoreApplication::translate("@default", "Blocking"));
            else
            {
                const StatusTypeData &typeData = m_statusTypeManager->statusTypeData(contact.currentStatus().type());
                pe.setContent(typeData.displayName());
            }
        }
        else if (chat && chat.chatAccount().statusContainer())
        {
            const StatusTypeData &typeData =
                m_statusTypeManager->statusTypeData(chat.chatAccount().statusContainer()->status().type());
            pe.setContent(typeData.displayName());
        }

        break;
    case 'q':
        ++idx;

        if (contact)
        {
            StatusContainer *container = contact.contactAccount().statusContainer();
            if (container)
                pe.setContent(container->statusIcon(Status{contact.currentStatus().type()}).path());
            else
                pe.setContent(m_statusContainerManager->statusIcon(Status{contact.currentStatus().type()}).path());
        }
        else if (chat)
        {
            StatusContainer *container = chat.chatAccount().statusContainer();
            if (container)
                pe.setContent(container->statusIcon().path());
            else
                pe.setContent(m_statusContainerManager->statusIcon(Status()).path());
        }

        break;
    case 'd':
        ++idx;

        if (contact)
        {
            QString description = contact.currentStatus().description();
            if (escape == ParserEscape::HtmlEscape)
                description = Qt::escape(description);

            pe.setContent(description);

            if (m_configuration->deprecatedApi()->readBoolEntry("Look", "ShowMultilineDesc"))
            {
                QString content = pe.decodedContent();
                content.replace('\n', QStringLiteral("<br/>"));
                content.replace(QRegExp("\\s\\s"), QString(" &nbsp;"));
                pe.setContent(content);
            }
        }

        break;
    case 'i':
        ++idx;
        break;
    case 'v':
        ++idx;
        break;
    case 'p':
        ++idx;
        break;
    case 'u':
        ++idx;

        if (contact)
            pe.setContent(contact.id());
        else if (buddy)
            pe.setContent(buddy.mobile().isEmpty() ? buddy.email() : buddy.mobile());

        break;
    case 'h':
        ++idx;
        break;
    case 'n':
    {
        ++idx;

        QString nickName = chat ? m_chatDataExtractor->data(chat, Qt::DisplayRole).toString() : buddy.nickName();
        if (escape == ParserEscape::HtmlEscape)
            nickName = Qt::escape(nickName);

        pe.setContent(nickName);

        break;
    }
    case 'a':
    {
        ++idx;

        QString display = chat ? m_chatDataExtractor->data(chat, Qt::DisplayRole).toString() : buddy.display();
        if (escape == ParserEscape::HtmlEscape)
            display = Qt::escape(display);

        pe.setContent(display);

        break;
    }
    case 'f':
    {
        ++idx;

        QString firstName = buddy.firstName();
        if (escape == ParserEscape::HtmlEscape)
            firstName = Qt::escape(firstName);

        pe.setContent(firstName);

        break;
    }
    case 'r':
    {
        ++idx;

        QString lastName = buddy.lastName();
        if (escape == ParserEscape::HtmlEscape)
            lastName = Qt::escape(lastName);

        pe.setContent(lastName);

        break;
    }
    case 'm':
        ++idx;

        pe.setContent(buddy.mobile());

        break;
    case 'g':
    {
        ++idx;

        QStringList groups;
        if (chat)
            for (auto const &group : chat.groups())
                groups << group.name();
        else
            for (auto const &group : buddy.groups())
                groups << group.name();

        pe.setContent(groups.join(","));

        break;
    }
    case 'e':
        ++idx;

        pe.setContent(buddy.email());

        break;
    case 'x':
        ++idx;

        if (contact)
            pe.setContent(QString::number(contact.maximumImageSize()));

        break;
    case 'z':
        ++idx;

        if (buddy)
            pe.setContent(QString::number(buddy.gender()));

        break;
    case '%':
        ++idx;
    // fall through
    default:
        pe.setContent("%");

        break;
    }

    return pe;
}

template <typename ContainerClass>
QString Parser::joinParserTokens(const ContainerClass &parseStack)
{
    QString joined;
    for (auto const &elem : parseStack)
    {
        switch (elem.type())
        {
        case PT_STRING:
            joined += elem.decodedContent();
            break;
        case PT_EXTERNAL_VARIABLE:
            joined += "#{";
            break;
        case PT_ICONPATH:
            joined += "@{";
            break;
        case PT_VARIABLE:
            joined += "${";
            break;
        case PT_CHECK_FILE_EXISTS:
            joined += '{';
            break;
        case PT_CHECK_FILE_NOT_EXISTS:
            joined += "{!";
            break;
        case PT_CHECK_ALL_NOT_NULL:
            joined += '[';
            break;
        case PT_CHECK_ANY_NULL:
            joined += "[!";
            break;
        case PT_EXECUTE:
            joined += '`';
            break;
        case PT_EXECUTE2:
            joined += "`{";
            break;
        }
    }

    return joined;
}

QString Parser::parse(
    const QString &s, Talkable talkable, const ParserData *const parserData,
    ParserEscape escape = ParserEscape::HtmlEscape)
{
    prepareSearchChars(m_configuration);

    QStack<ParserToken> parseStack;
    int idx = 0, len = s.length();
    while (idx < len)
    {
        ParserToken pe1, pe;

        int prevIdx = idx;
        for (; idx < len; ++idx)
            if (searchChars()->contains(s.at(idx)))
                break;

        if (idx != prevIdx)
        {
            pe1.setType(PT_STRING);
            pe1.setContent(s.mid(prevIdx, idx - prevIdx));
            parseStack.push(pe1);

            if (idx == len)
                break;
        }

        const QChar c(s.at(idx));
        if (c == '%')
        {
            ++idx;
            if (idx == len)
                break;

            pe = parsePercentSyntax(s, idx, talkable, escape);
            pe.encodeContent(QByteArray(), ENCODE_INCLUDE_CHARS);

            parseStack.push(pe);
        }
        else if (c == '[')
        {
            ++idx;
            if (idx == len)
                break;

            if (s.at(idx) == '!')
            {
                pe.setType(PT_CHECK_ANY_NULL);
                ++idx;
            }
            else
                pe.setType(PT_CHECK_ALL_NOT_NULL);

            parseStack.push(pe);
        }
        else if (c == ']')
        {
            ++idx;

            QVector<ParserTokenType> acceptedTokens;
            acceptedTokens << PT_CHECK_ALL_NOT_NULL << PT_CHECK_ANY_NULL;

            if (!isActionParserTokenAtTop(parseStack, acceptedTokens))
            {
                pe.setContent("]");
                pe.setType(PT_STRING);

                parseStack.push(pe);
            }
            else
            {
                bool anyNull = false;
                while (!parseStack.empty())
                {
                    ParserToken pe2 = parseStack.pop();

                    if (pe2.type() == PT_CHECK_ALL_NOT_NULL)
                    {
                        if (!anyNull)
                        {
                            pe.setType(PT_STRING);

                            parseStack.push(pe);
                        }

                        break;
                    }

                    if (pe2.type() == PT_CHECK_ANY_NULL)
                    {
                        if (anyNull)
                        {
                            pe.setType(PT_STRING);

                            parseStack.push(pe);
                        }

                        break;
                    }

                    // here we know for sure that pe2.type() == PT_STRING,
                    // as it is guaranteed by isActionParserTokenAtTop() call
                    anyNull = anyNull || pe2.decodedContent().isEmpty();
                    QString content = pe.decodedContent();
                    content.prepend(pe2.decodedContent());
                    pe.setContent(content);
                }
            }
        }
        else if (c == '{')
        {
            ++idx;
            if (idx == len)
                break;

            if (s.at(idx) == '!' || s.at(idx) == '~')
            {
                ++idx;
                pe.setType(PT_CHECK_FILE_NOT_EXISTS);
            }
            else
                pe.setType(PT_CHECK_FILE_EXISTS);

            parseStack.push(pe);
        }
        else if (c == '}')
        {
            ++idx;

            QVector<ParserTokenType> acceptedTokens;
            acceptedTokens << PT_CHECK_FILE_EXISTS << PT_CHECK_FILE_NOT_EXISTS << PT_VARIABLE << PT_ICONPATH
                           << PT_EXTERNAL_VARIABLE << PT_EXECUTE2;

            if (!isActionParserTokenAtTop(parseStack, acceptedTokens))
            {
                pe.setContent("}");
                pe.setType(PT_STRING);

                parseStack.push(pe);
            }
            else
            {
                QList<ParserToken> tokens;

                while (!parseStack.empty())
                {
                    ParserToken pe2 = parseStack.pop();

                    if (pe2.type() == PT_CHECK_FILE_EXISTS || pe2.type() == PT_CHECK_FILE_NOT_EXISTS)
                    {
                        int firstSpaceTokenIdx = 0, spacePos = -1;
                        for (auto const &token : tokens)
                        {
                            // encoded cannot contain space
                            if (!token.isEncoded())
                            {
                                spacePos = token.rawContent().indexOf(' ');
                                if (spacePos != -1)
                                    break;
                            }

                            ++firstSpaceTokenIdx;
                        }

                        QString filePath;
                        if (spacePos == -1)
                            filePath = joinParserTokens(tokens);
                        else
                            filePath = joinParserTokens(tokens.mid(0, firstSpaceTokenIdx)) +
                                       tokens.at(firstSpaceTokenIdx).rawContent().left(spacePos);

#ifdef Q_OS_WIN
                        if (filePath.startsWith(QStringLiteral("file:///")))
                            filePath = filePath.mid(static_cast<int>(qstrlen("file:///")));
#else
                        if (filePath.startsWith(QStringLiteral("file:///")))
                            filePath = filePath.mid(static_cast<int>(qstrlen("file://")));
#endif

                        bool checkFileExists = (pe2.type() == PT_CHECK_FILE_EXISTS);
                        if (QFileInfo::exists(filePath) == checkFileExists)
                        {
                            pe.setType(PT_STRING);

                            if (spacePos == -1)
                                pe.setContent(filePath);
                            else
                            {
                                QString content = tokens.at(firstSpaceTokenIdx).rawContent().mid(spacePos + 1) +
                                                  joinParserTokens(tokens.mid(firstSpaceTokenIdx + 1));

                                pe.setContent(content);
                            }

                            parseStack.push(pe);
                        }

                        break;
                    }

                    if (pe2.type() == PT_VARIABLE)
                    {
                        QString content = joinParserTokens(tokens);

                        pe.setType(PT_STRING);

                        if (GlobalVariables.contains(content))
                        {
                            pe.setContent(GlobalVariables[content]);
                            pe.encodeContent(QByteArray(), ENCODE_INCLUDE_CHARS);
                        }
                        else
                            pe.setContent(QString());

                        parseStack.push(pe);

                        break;
                    }

                    if (pe2.type() == PT_ICONPATH)
                    {
                        QString content = joinParserTokens(tokens);

                        pe.setType(PT_STRING);
                        if (content.contains(':'))
                        {
                            QStringList parts = content.split(':');
                            pe.setContent(PathsProvider::webKitPath(
                                m_iconsManager->iconPath(KaduIcon(parts.at(0), parts.at(1)))));
                        }
                        else
                            pe.setContent(PathsProvider::webKitPath(m_iconsManager->iconPath(KaduIcon(content))));

                        parseStack.push(pe);

                        break;
                    }

                    if (pe2.type() == PT_EXTERNAL_VARIABLE)
                    {
                        QString content = joinParserTokens(tokens);

                        pe.setType(PT_STRING);

                        if (m_registeredTalkableTags.contains(content))
                            pe.setContent(m_registeredTalkableTags[content](talkable));
                        else if (parserData && m_registeredObjectTags.contains(content))
                            pe.setContent(m_registeredObjectTags[content](parserData));
                        else
                        {
                            pe.setContent(QString());
                        }

                        pe.encodeContent(QByteArray(), ENCODE_INCLUDE_CHARS);
                        parseStack.push(pe);

                        break;
                    }

                    if (pe2.type() == PT_EXECUTE2)
                    {
                        pe.setType(PT_STRING);
                        pe.setContent(executeCmd(joinParserTokens(tokens)));

                        parseStack.push(pe);

                        break;
                    }

                    // here we know for sure that pe2.type() == PT_STRING,
                    // as it is guaranteed by isActionParserTokenAtTop() call
                    tokens.prepend(pe2);
                }
            }
        }
        else if (c == '`')
        {
            ++idx;

            if (idx == len || s.at(idx) != '{')
            {
                pe.setType(PT_EXECUTE);

                parseStack.push(pe);
            }
            else
            {
                ++idx;

                pe.setType(PT_EXECUTE2);

                parseStack.push(pe);
            }
        }
        else if (c == '\'')
        {
            ++idx;

            pe.setContent(QString());

            QVector<ParserTokenType> acceptedTokens(PT_EXECUTE);

            if (!isActionParserTokenAtTop(parseStack, acceptedTokens))
            {
                pe.setContent("\'");
                pe.setType(PT_STRING);

                parseStack.push(pe);
            }
            else
                while (!parseStack.empty())
                {
                    ParserToken pe2 = parseStack.pop();

                    if (pe2.type() == PT_EXECUTE)
                    {
                        pe.setType(PT_STRING);
                        pe.setContent(executeCmd(pe.decodedContent()));

                        parseStack.push(pe);

                        break;
                    }

                    // here we know for sure that pe2.type() == PT_STRING,
                    // as it is guaranteed by isActionParserTokenAtTop() call
                    QString content = pe.decodedContent();
                    content.prepend(pe2.decodedContent());
                    pe.setContent(content);
                }
        }
        else if (c == '\\')
        {
            ++idx;
            if (idx == len)
                break;

            pe.setType(PT_STRING);
            pe.setContent(s.at(idx));

            ++idx;

            parseStack.push(pe);
        }
        else if (c == '$')
        {
            ++idx;

            if (idx == len || s.at(idx) != '{')
            {
                pe.setType(PT_STRING);
                pe.setContent("$");

                parseStack.push(pe);
            }
            else
            {
                ++idx;

                pe.setType(PT_VARIABLE);

                parseStack.push(pe);
            }
        }
        else if (c == '@')
        {
            ++idx;

            if (idx == len || s.at(idx) != '{')
            {
                pe.setType(PT_STRING);
                pe.setContent("@");

                parseStack.push(pe);
            }
            else
            {
                ++idx;

                pe.setType(PT_ICONPATH);

                parseStack.push(pe);
            }
        }
        else if (c == '#')
        {
            ++idx;

            if (idx == len || s.at(idx) != '{')
            {
                pe.setType(PT_STRING);
                pe.setContent("#");

                parseStack.push(pe);
            }
            else
            {
                ++idx;

                pe.setType(PT_EXTERNAL_VARIABLE);

                parseStack.push(pe);
            }
        }
    }

    QString ret = joinParserTokens(parseStack);

    return ret;
}
