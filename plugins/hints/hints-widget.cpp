/*
 * %kadu copyright begin%
 * Copyright 2016 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "hints-widget.h"
#include "moc_hints-widget.cpp"

#include "hint.h"
#include "hints-configuration.h"

#include "misc/memory.h"
#include "plugin/plugin-injected-factory.h"

#include <QtCore/QDateTime>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QLayoutItem>
#include <QtWidgets/QVBoxLayout>

HintsWidget::HintsWidget(QWidget *parent)
        : QFrame{parent,
                 Qt::FramelessWindowHint | Qt::Tool | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint |
                     Qt::MSWindowsOwnDC}
{
    setObjectName(QStringLiteral("hints"));
    setStyleSheet(QStringLiteral("#hints { border: 1px solid %1; }").arg(palette().window().color().darker().name()));

    m_layout = make_owned<QVBoxLayout>(this);
    m_layout->setSpacing(0);
    m_layout->setMargin(0);

    connect(&m_timer, &QTimer::timeout, this, &HintsWidget::removeExpiredHints);
}

HintsWidget::~HintsWidget()
{
    m_layout = nullptr;
}

void HintsWidget::setHintsConfiguration(HintsConfiguration *hintsConfiguration)
{
    m_hintsConfiguration = hintsConfiguration;
}

void HintsWidget::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void HintsWidget::addNotification(const Notification &notification)
{
    auto hint = m_pluginInjectedFactory->makeOwned<Hint>(notification, m_hintsConfiguration, this);
    hint->setProperty("expiration-time", QDateTime::currentDateTime().addSecs(10));
    m_layout->addWidget(hint);

    connect(hint, &Hint::leftButtonClicked, this, &HintsWidget::acceptHint);
    connect(hint, &Hint::rightButtonClicked, this, &HintsWidget::discardHint);
    connect(hint, &Hint::midButtonClicked, this, &HintsWidget::discardAllHints);
    connect(hint, &Hint::hintDestroyed, this, &HintsWidget::hintDestroyed);

    auto count = m_layout->count();
    auto height = hint->height();
    hint->adjustSize();

    for (auto i = 0; i < count; i++)
    {
        auto w = m_layout->itemAt(i)->widget();
        w->adjustSize();
        height += w->height();
    }

    auto maximumHeight = QApplication::desktop()->availableGeometry(this).height() / 2;
    while (height > maximumHeight)
    {
        auto hintToRemove = static_cast<Hint *>(m_layout->itemAt(0)->widget());
        height -= hintToRemove->height();
        m_layout->removeWidget(hintToRemove);
        hintToRemove->deleteLater();
    }

    updateHints();
}

void HintsWidget::removeHint(Hint *hint)
{
    m_layout->removeWidget(hint);
    hint->deleteLater();
    updateHints();
}

void HintsWidget::updateTimer()
{
    if (m_layout->count() == 0)
    {
        m_timer.stop();
        return;
    }

    auto hint = static_cast<Hint *>(m_layout->itemAt(0)->widget());
    auto expirationTime = hint->property("expiration-time").toDateTime();
    if (expirationTime < QDateTime::currentDateTime())
    {
        removeExpiredHints();
        return;
    }

    m_timer.start(QDateTime::currentDateTime().msecsTo(expirationTime) + 1000);
}

void HintsWidget::removeExpiredHints()
{
    while (m_layout->count() > 0)
    {
        auto hint = static_cast<Hint *>(m_layout->itemAt(0)->widget());
        auto expirationTime = hint->property("expiration-time").toDateTime();

        if (expirationTime < QDateTime::currentDateTime())
            removeHint(hint);
        else
            break;
    }

    updateHints();
}

void HintsWidget::resizeEvent(QResizeEvent *re)
{
    QWidget::resizeEvent(re);
    emit sizeChanged();
}

void HintsWidget::showEvent(QShowEvent *se)
{
    QWidget::showEvent(se);
    emit shown();
}

void HintsWidget::acceptHint(Hint *hint)
{
    hint->acceptNotification();
    removeHint(hint);
}

void HintsWidget::discardHint(Hint *hint)
{
    hint->discardNotification();
    removeHint(hint);
}

void HintsWidget::discardAllHints()
{
    while (m_layout->count())
    {
        auto item = m_layout->itemAt(0);
        auto hint = static_cast<Hint *>(item->widget());
        hint->deleteLater();
        m_layout->removeItem(item);
    }

    hide();
}

void HintsWidget::hintDestroyed(Hint *hint)
{
    if (m_layout)
    {
        m_layout->removeWidget(hint);
        updateHints();
    }
}

void HintsWidget::updateHints()
{
    if (m_layout->count() > 0)
    {
        adjustSize();
        show();
    }
    else
        hide();

    updateTimer();
}
