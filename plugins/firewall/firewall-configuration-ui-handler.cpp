/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "firewall-configuration-ui-handler.h"
#include "moc_firewall-configuration-ui-handler.cpp"

#include "firewall-message-filter.h"

#include "buddies/buddy-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "configuration/gui/configuration-ui-handler-repository.h"
#include "misc/paths-provider.h"
#include "widgets/configuration/config-group-box.h"
#include "widgets/configuration/configuration-widget.h"
#include "windows/main-configuration-window.h"

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>

FirewallConfigurationUiHandler::FirewallConfigurationUiHandler(QObject *parent)
        : QObject{parent}, AllList{}, SecureList{}, QuestionEdit{}, AnswerEdit{}
{
}

FirewallConfigurationUiHandler::~FirewallConfigurationUiHandler()
{
}

void FirewallConfigurationUiHandler::setBuddyManager(BuddyManager *buddyManager)
{
    m_buddyManager = buddyManager;
}

void FirewallConfigurationUiHandler::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void FirewallConfigurationUiHandler::mainConfigurationWindowCreated(MainConfigurationWindow *mainConfigurationWindow)
{
    ConfigGroupBox *secureGroupBox =
        mainConfigurationWindow->widget()->configGroupBox("Firewall", "Safe sending", "Safe sending");

    QWidget *secure = new QWidget(secureGroupBox->widget());
    QGridLayout *secureLayout = new QGridLayout(secure);
    secureLayout->setSpacing(5);
    secureLayout->setMargin(5);

    AllList = new QListWidget(secure);
    QPushButton *moveToSecureList = new QPushButton(tr("Move to 'Secured'"), secure);

    secureLayout->addWidget(new QLabel(tr("All"), secure), 0, 0);
    secureLayout->addWidget(AllList, 1, 0);
    secureLayout->addWidget(moveToSecureList, 2, 0);

    SecureList = new QListWidget(secure);
    QPushButton *moveToAllList = new QPushButton(tr("Move to 'All'"), secure);

    secureLayout->addWidget(new QLabel(tr("Secured"), secure), 0, 1);
    secureLayout->addWidget(SecureList, 1, 1);
    secureLayout->addWidget(moveToAllList, 2, 1);

    connect(moveToSecureList, SIGNAL(clicked()), this, SLOT(allRight()));
    connect(moveToAllList, SIGNAL(clicked()), this, SLOT(allLeft()));
    connect(SecureList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(left(QListWidgetItem *)));
    connect(AllList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(right(QListWidgetItem *)));

    secureGroupBox->addWidgets(0, secure);

    for (auto const &buddy : m_buddyManager->items())
        if (!buddy.isAnonymous())
        {
            if (!buddy.property("firewall-secured-sending:FirewallSecuredSending", false).toBool())
                AllList->addItem(buddy.display());
            else
                SecureList->addItem(buddy.display());
        }

    AllList->sortItems();
    SecureList->sortItems();

    AllList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    SecureList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    /*
    Automatic question GUI
    */
    ConfigGroupBox *questionGroupBox =
        mainConfigurationWindow->widget()->configGroupBox("Firewall", "Unknown chats protection", "Automatic question");

    QWidget *question = new QWidget(questionGroupBox->widget());
    QFormLayout *questionLayout = new QFormLayout(question);
    questionLayout->setSpacing(5);
    questionLayout->setMargin(5);

    QuestionEdit = new QTextEdit(question);
    QuestionEdit->setAcceptRichText(false);
    QuestionEdit->setText(m_configuration->deprecatedApi()->readEntry("Firewall", "question"));
    QuestionEdit->setToolTip(tr("This message will be send to unknown person."));

    AnswerEdit = new QLineEdit(question);
    AnswerEdit->setText(m_configuration->deprecatedApi()->readEntry("Firewall", "answer"));
    AnswerEdit->setToolTip(tr("Right answer for question above - you can use regexp."));
    QLabel *label = new QLabel(tr("Answer:"), question);
    label->setToolTip(tr("Right answer for question above - you can use regexp."));

    questionLayout->addRow(new QLabel(tr("Message:"), question), QuestionEdit);
    questionLayout->addRow(label, AnswerEdit);

    questionGroupBox->addWidgets(0, question);
    /*
    End creating Gui
    */
    connect(
        mainConfigurationWindow->widget()->widgetById("firewall/chats"), SIGNAL(toggled(bool)),
        mainConfigurationWindow->widget()
            ->configGroupBox("Firewall", "Unknown chats protection", "Automatic question")
            ->widget(),
        SLOT(setEnabled(bool)));
    connect(
        mainConfigurationWindow->widget()->widgetById("firewall/chats"), SIGNAL(toggled(bool)),
        mainConfigurationWindow->widget()
            ->configGroupBox("Firewall", "Unknown chats protection", "Reaction on right answer")
            ->widget(),
        SLOT(setEnabled(bool)));

    QWidget *safeSending = mainConfigurationWindow->widget()->widgetById("firewall/safe_sending");
    connect(safeSending, SIGNAL(toggled(bool)), AllList, SLOT(setEnabled(bool)));
    connect(safeSending, SIGNAL(toggled(bool)), SecureList, SLOT(setEnabled(bool)));
    connect(safeSending, SIGNAL(toggled(bool)), moveToSecureList, SLOT(setEnabled(bool)));
    connect(safeSending, SIGNAL(toggled(bool)), moveToAllList, SLOT(setEnabled(bool)));

    connect(mainConfigurationWindow, SIGNAL(configurationWindowApplied()), this, SLOT(configurationApplied()));
}

void FirewallConfigurationUiHandler::mainConfigurationWindowDestroyed()
{
}

void FirewallConfigurationUiHandler::mainConfigurationWindowApplied()
{
}

void FirewallConfigurationUiHandler::left(QListWidgetItem *item)
{
    AllList->addItem(new QListWidgetItem(*item));
    SecureList->removeItemWidget(item);
    delete item;

    AllList->sortItems();
}

void FirewallConfigurationUiHandler::allLeft()
{
    int count = SecureList->count();

    for (int i = count - 1; i >= 0; i--)
        if (SecureList->item(i)->isSelected())
        {
            AllList->addItem(SecureList->item(i)->text());
            delete SecureList->takeItem(i);
        }

    AllList->sortItems();
}

void FirewallConfigurationUiHandler::right(QListWidgetItem *item)
{
    SecureList->addItem(new QListWidgetItem(*item));
    AllList->removeItemWidget(item);
    delete item;

    SecureList->sortItems();
}

void FirewallConfigurationUiHandler::allRight()
{
    int count = AllList->count();

    for (int i = count - 1; i >= 0; i--)
        if (AllList->item(i)->isSelected())
        {
            SecureList->addItem(AllList->item(i)->text());
            delete AllList->takeItem(i);
        }

    SecureList->sortItems();
}

void FirewallConfigurationUiHandler::configurationApplied()
{
    int count = SecureList->count();
    for (int i = 0; i < count; i++)
    {
        Buddy buddy = m_buddyManager->byDisplay(SecureList->item(i)->text(), ActionReturnNull);
        if (buddy.isNull() || buddy.isAnonymous())
            continue;

        buddy.addProperty("firewall-secured-sending:FirewallSecuredSending", true, CustomProperties::Storable);
    }

    count = AllList->count();
    for (int i = 0; i < count; i++)
    {
        Buddy buddy = m_buddyManager->byDisplay(AllList->item(i)->text(), ActionReturnNull);
        if (buddy.isNull() || buddy.isAnonymous())
            continue;

        buddy.removeProperty("firewall-secured-sending:FirewallSecuredSending");
    }

    m_configuration->deprecatedApi()->writeEntry("Firewall", "question", QuestionEdit->toPlainText());
    m_configuration->deprecatedApi()->writeEntry("Firewall", "answer", AnswerEdit->text());
}
