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

#include "ssl-certificate-repository.h"
#include "moc_ssl-certificate-repository.cpp"

#include "ssl/ssl-certificate.h"

#include <QtNetwork/QSslCertificate>

SslCertificateRepository::SslCertificateRepository(QObject *parent) : QObject{parent}
{
}

SslCertificateRepository::~SslCertificateRepository()
{
}

SslCertificateRepository::Iterator SslCertificateRepository::begin()
{
    return m_certificates.begin();
}

SslCertificateRepository::Iterator SslCertificateRepository::end()
{
    return m_certificates.end();
}

QSet<SslCertificate> SslCertificateRepository::certificates() const
{
    return m_certificates;
}

void SslCertificateRepository::setPersistentCertificates(const QSet<SslCertificate> &certificates)
{
    m_certificates = certificates;
    m_persistentCertificates = certificates;
}

QSet<SslCertificate> SslCertificateRepository::persistentCertificates() const
{
    return m_persistentCertificates;
}

bool SslCertificateRepository::containsCertificate(const SslCertificate &certificate) const
{
    return m_certificates.contains(certificate);
}


bool SslCertificateRepository::containsCertificateFor(const QString &hostName, QList<QString> hostNames) const
{
    qSort(hostNames);
    for (auto const& c : m_certificates)
    {
        if (c.hostName() != hostName)
            continue;

        auto ssl = QSslCertificate::fromData(QByteArray::fromHex(c.pemHexEncodedCertificate()));
        if (ssl.isEmpty())
            continue;

        auto sslHostnames = ssl[0].subjectAlternativeNames().values(QSsl::DnsEntry);
        qSort(sslHostnames);

        if (hostNames == sslHostnames)
            return true;
    }

    return false;
}

void SslCertificateRepository::addCertificate(SslCertificate certificate)
{
    m_certificates.insert(certificate);
}

void SslCertificateRepository::addPersistentCertificate(SslCertificate certificate)
{
    m_certificates.insert(certificate);
    m_persistentCertificates.insert(certificate);
}

void SslCertificateRepository::removeCertificate(SslCertificate certificate)
{
    m_certificates.remove(certificate);
    m_persistentCertificates.remove(certificate);
}
