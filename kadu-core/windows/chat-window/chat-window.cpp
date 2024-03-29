/*
 * %kadu copyright begin%
 * Copyright 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "chat-window.h"
#include "moc_chat-window.cpp"

#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QVBoxLayout>

#include "chat/chat-details.h"
#include "chat/type/chat-type.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "contacts/contact-set.h"
#include "core/core.h"
#include "core/injected-factory.h"
#include "gui/configuration/chat-configuration-holder.h"
#include "icons/icons-manager.h"
#include "os/generic/window-geometry-manager.h"
#include "storage/custom-properties-variant-wrapper.h"
#include "widgets/chat-widget/chat-widget-impl.h"
#include "widgets/chat-widget/chat-widget-manager.h"
#include "widgets/chat-widget/chat-widget-title.h"
#include "widgets/custom-input.h"
#include "windows/message-dialog.h"

#include "activate.h"

ChatWindow::ChatWindow(Chat chat, QWidget *parent) : QWidget(parent), DesktopAwareObject(this), m_chat{chat}
{
}

ChatWindow::~ChatWindow()
{
    emit windowDestroyed(this);
}

void ChatWindow::setChatConfigurationHolder(ChatConfigurationHolder *chatConfigurationHolder)
{
    m_chatConfigurationHolder = chatConfigurationHolder;
}

void ChatWindow::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void ChatWindow::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void ChatWindow::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

void ChatWindow::init()
{
    setWindowRole("kadu-chat");

    m_chatWidget = m_injectedFactory->makeInjected<ChatWidgetImpl>(m_chat, this);
    connect(m_chatWidget, SIGNAL(closeRequested(ChatWidget *)), this, SLOT(close()));

    if (m_chatWidget && m_chatWidget->chat().details() && m_chatWidget->chat().details()->type())
        setWindowRole(m_chatWidget->chat().details()->type()->windowRole());

    setAttribute(Qt::WA_DeleteOnClose);

    m_chatWidget->edit()->setFocus();
    m_chatWidget->kaduRestoreGeometry();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_chatWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    configurationUpdated();

    updateTitle();

    CustomPropertiesVariantWrapper *variantWrapper = new CustomPropertiesVariantWrapper(
        m_chatWidget->chat().data()->customProperties(), "chat-geometry:WindowGeometry", CustomProperties::Storable);
    new WindowGeometryManager(variantWrapper, defaultGeometry(), this);

    connect(m_chatWidget->title(), SIGNAL(titleChanged(ChatWidget *)), this, SLOT(updateTitle()));
}

void ChatWindow::configurationUpdated()
{
    triggerCompositingStateChanged();

    m_chatWidget->title()->setBlinkIconWhenUnreadMessages(false);
    // m_configuration->deprecatedApi()->readBoolEntry("Chat", "BlinkChatTitle", true));
    m_chatWidget->title()->setBlinkTitleWhenUnreadMessages(
        m_configuration->deprecatedApi()->readBoolEntry("Chat", "BlinkChatTitle", false));
    m_chatWidget->title()->setComposingStatePosition(m_chatConfigurationHolder->composingStatePosition());
    m_chatWidget->title()->setShowUnreadMessagesCount(
        m_configuration->deprecatedApi()->readBoolEntry("Chat", "NewMessagesInChatTitle", false));
}

void ChatWindow::compositingEnabled()
{
    if (m_configuration->deprecatedApi()->readBoolEntry("Chat", "UseTransparency", false))
    {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }
    else
        compositingDisabled();
}

void ChatWindow::compositingDisabled()
{
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAttribute(Qt::WA_NoSystemBackground, false);
    setAutoFillBackground(true);
}

QRect ChatWindow::defaultGeometry() const
{
    QSize size(0, 400);
    int x, y;
    x = pos().x();
    y = pos().y();
    if (m_chatWidget->chat().contacts().count() > 1)
        size.setWidth(550);
    else
        size.setWidth(400);

    QDesktopWidget *desk = qApp->desktop();

    if ((size.width() + x) > desk->width())
        x = desk->width() - size.width() - 50;
    if ((size.height() + y) > desk->height())
        y = desk->height() - size.height() - 50;

    if (x < 50)
        x = 50;
    if (y < 50)
        y = 50;

    return QRect(QPoint(x, y), size);
}

Chat ChatWindow::chat() const
{
    return m_chatWidget ? m_chatWidget->chat() : Chat::null;
}

void ChatWindow::closeEvent(QCloseEvent *e)
{
    if (m_configuration->deprecatedApi()->readBoolEntry("Chat", "ChatCloseTimer"))
    {
        int period = m_configuration->deprecatedApi()->readNumEntry("Chat", "ChatCloseTimerPeriod", 2);

        if (QDateTime::currentDateTime() < m_chatWidget->lastReceivedMessageTime().addSecs(period))
        {
            MessageDialog *dialog = MessageDialog::create(
                m_iconsManager->iconByPath(KaduIcon("dialog-question")), tr("Kadu"),
                tr("New message received, close window anyway?"));
            dialog->addButton(QMessageBox::Yes, tr("Close window"));
            dialog->addButton(QMessageBox::No, tr("Cancel"));

            if (!dialog->ask())
            {
                e->ignore();
                return;
            }
        }
    }

    QWidget::closeEvent(e);
}

void ChatWindow::updateTitle()
{
    setWindowTitle(m_chatWidget->title()->blinkingFullTitle());
    setWindowIcon(m_chatWidget->title()->blinkingIcon());
}

void ChatWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange && _isActiveWindow(this))
        emit activated(this);
}

void ChatWindow::setWindowTitle(QString title)
{
    // qt treats [*] as 'modified placeholder'
    // we escape each [*] with double [*][*] so it gets properly handled
    QWidget::setWindowTitle(title.replace(QStringLiteral("[*]"), QStringLiteral("[*][*]")));
}

bool ChatWindow::isChatWidgetActive(const ChatWidget *chatWidget)
{
    Q_UNUSED(chatWidget)
    Q_ASSERT(chatWidget == m_chatWidget);

    return _isWindowActiveOrFullyVisible(this);
}
