/*
 * %kadu copyright begin%
 * Copyright 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#pragma once

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <functional>
#include <injeqt/injeqt.h>
#include <QXmppQt5/QXmppClient.h>

class SslCertificateManager;
class QSslError;

class JabberSslHandler : public QObject
{
    Q_OBJECT

public:
    explicit JabberSslHandler(
        QXmppClient *parent, const std::function<void()> &onAccepted, const std::function<void()> &onRejected);
    virtual ~JabberSslHandler();

private:
    QPointer<SslCertificateManager> m_sslCertificateManager;

    std::function<void()> m_onAccepted;
    std::function<void()> m_onRejected;

private slots:
    INJEQT_SET void setSslCertificateManager(SslCertificateManager *sslCertificateManager);

    void sslErrors(const QList<QSslError> &errors);
};
