/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2012 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2011, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QTextDocument>
#include <QtWidgets/QMenu>
#include <QtWidgets/QWidgetAction>

#include "accounts/account-manager.h"
#include "accounts/account.h"
#include "core/injected-factory.h"
#include "gui/status-icon.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "protocols/protocol.h"
#include "status/status-configuration-holder.h"
#include "status/status-container.h"
#include "status/status-type-data.h"
#include "status/status-type-manager.h"
#include "widgets/status-menu.h"

#include "status-button.h"
#include "moc_status-button.cpp"

StatusButton::StatusButton(StatusContainer *statusContainer, QWidget *parent)
        : QToolButton(parent), MyStatusContainer(statusContainer), DisplayStatusName(false), MenuTitleAction{nullptr}
{
}

StatusButton::~StatusButton()
{
}

void StatusButton::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void StatusButton::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

void StatusButton::setStatusConfigurationHolder(StatusConfigurationHolder *statusConfigurationHolder)
{
    m_statusConfigurationHolder = statusConfigurationHolder;
}

void StatusButton::setStatusTypeManager(StatusTypeManager *statusTypeManager)
{
    m_statusTypeManager = statusTypeManager;
}

void StatusButton::init()
{
    Icon = m_injectedFactory->makeInjected<StatusIcon>(MyStatusContainer, this);

    createGui();
    setPopupMode(InstantPopup);

    statusUpdated();
    connect(MyStatusContainer, SIGNAL(statusUpdated(StatusContainer *)), this, SLOT(statusUpdated(StatusContainer *)));
    connect(Icon, SIGNAL(iconUpdated(KaduIcon)), this, SLOT(iconUpdated(KaduIcon)));
}

void StatusButton::createGui()
{
    QMenu *menu = new QMenu(this);
    if (!MyStatusContainer->statusContainerName().isEmpty())
        addTitleToMenu(MyStatusContainer->statusContainerName(), menu);
    m_injectedFactory->makeInjected<StatusMenu>(MyStatusContainer, false, menu);

    setMenu(menu);
    setIcon(m_iconsManager->iconByPath(Icon->icon()));
}

void StatusButton::addTitleToMenu(const QString &title, QMenu *menu)
{
    MenuTitleAction = new QAction(menu);
    QFont font = MenuTitleAction->font();
    font.setBold(true);

    MenuTitleAction->setFont(font);
    MenuTitleAction->setText(title);
    MenuTitleAction->setIcon(m_iconsManager->iconByPath(MyStatusContainer->statusIcon()));

    QWidgetAction *action = new QWidgetAction(this);
    action->setObjectName("status_menu_title");
    QToolButton *titleButton = new QToolButton(this);
    titleButton->installEventFilter(this);   // prevent clicks on the title of the menu
    titleButton->setDefaultAction(MenuTitleAction);
    titleButton->setDown(true);   // prevent hover style changes in some styles
    titleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    action->setDefaultWidget(titleButton);

    menu->addAction(action);
}

bool StatusButton::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);

    if (event->type() == QEvent::ActionChanged || event->type() == QEvent::Paint || event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease)
        return false;

    event->accept();

    return true;
}

void StatusButton::updateStatus()
{
    QString tooltip("<table>");

    if (DisplayStatusName)
    {
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        setText(m_statusTypeManager->statusTypeData(MyStatusContainer->status().type()).displayName());
    }
    else
    {
        if (m_statusConfigurationHolder->isSetStatusPerIdentity())
        {
            setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            setText(MyStatusContainer->statusContainerName());
            tooltip += QString(
                           "<tr><td align='right' style='font-weight:bold; white-space:nowrap;'>%1:</td><td "
                           "style='white-space:nowrap;'>%2</td></tr>")
                           .arg(tr("Identity"), MyStatusContainer->statusContainerName());
        }
        else
        {
            setToolButtonStyle(Qt::ToolButtonIconOnly);
            setText(QString());
            tooltip += QString(
                           "<tr><td align='right' style='font-weight:bold; white-space:nowrap;'>%1:</td><td "
                           "style='white-space:nowrap;'>%2</td></tr>")
                           .arg(tr("Account"), MyStatusContainer->statusContainerName());
        }
    }

    tooltip +=
        QString(
            "<tr><td align='right' style='font-weight:bold; white-space:nowrap;'>%1:</td><td "
            "style='white-space:nowrap;'>%2</td></tr>")
            .arg(tr("Status"), m_statusTypeManager->statusTypeData(MyStatusContainer->status().type()).displayName());
    tooltip += QString("<tr><td align='right' style='font-weight:bold; white-space:nowrap;'>%1:</td><td>%2</td></tr>")
                   .arg(tr("Description"), prepareDescription(MyStatusContainer->status().description()));

    tooltip += QString("</table>");

    setToolTip(tooltip);
}

void StatusButton::statusUpdated(StatusContainer *container)
{
    Q_UNUSED(container)

    updateStatus();
}

void StatusButton::configurationUpdated()
{
    updateStatus();
}

void StatusButton::setDisplayStatusName(bool displayStatusName)
{
    if (DisplayStatusName != displayStatusName)
    {
        DisplayStatusName = displayStatusName;
        updateStatus();
    }
}

void StatusButton::iconUpdated(const KaduIcon &icon)
{
    setIcon(m_iconsManager->iconByPath(icon));
    if (MenuTitleAction)
        MenuTitleAction->setIcon(m_iconsManager->iconByPath(icon));
}

QString StatusButton::prepareDescription(const QString &description) const
{
    QColor color = palette().windowText().color();
    color.setAlpha(128);
    QString colorString =
        QString("rgba(%1,%2,%3,%4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());

    QString html = Qt::escape(description);
    html.replace(
        '\n',
        QString(QStringLiteral("<span style='color:%1;'> ") + QChar(0x21B5) + QStringLiteral("</span><br />"))
            .arg(colorString));

    return html;
}
