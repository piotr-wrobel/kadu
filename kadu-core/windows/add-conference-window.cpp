/*
 * %kadu copyright begin%
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

#include "add-conference-window.h"
#include "moc_add-conference-window.cpp"

#include "accounts/filter/protocol-filter.h"
#include "buddies/model/buddy-list-model.h"
#include "buddies/model/buddy-manager-adapter.h"
#include "chat/chat-manager.h"
#include "chat/chat-storage.h"
#include "chat/type/chat-type-contact-set.h"
#include "configuration/config-file-variant-wrapper.h"
#include "core/injected-factory.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "model/model-chain.h"
#include "os/generic/window-geometry-manager.h"
#include "protocols/protocol.h"
#include "talkable/filter/account-talkable-filter.h"
#include "talkable/filter/hide-anonymous-talkable-filter.h"
#include "talkable/filter/name-talkable-filter.h"
#include "talkable/model/talkable-proxy-model.h"
#include "widgets/accounts-combo-box.h"
#include "widgets/chat-widget/chat-widget-manager.h"
#include "widgets/filtered-tree-view.h"
#include "widgets/talkable-tree-view.h"
// will be used when Qt 4.8 .is required
// #include "buddies/model/checkable-buddies-proxy-model.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

AddConferenceWindow::AddConferenceWindow(QWidget *parent) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowRole("kadu-add-conference");
    setWindowTitle(tr("Add Conference"));
}

AddConferenceWindow::~AddConferenceWindow()
{
}

void AddConferenceWindow::setChatManager(ChatManager *chatManager)
{
    m_chatManager = chatManager;
}

void AddConferenceWindow::setChatStorage(ChatStorage *chatStorage)
{
    m_chatStorage = chatStorage;
}

void AddConferenceWindow::setChatWidgetManager(ChatWidgetManager *chatWidgetManager)
{
    m_chatWidgetManager = chatWidgetManager;
}

void AddConferenceWindow::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void AddConferenceWindow::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void AddConferenceWindow::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

void AddConferenceWindow::init()
{
    createGui();

    accountChanged();
    validateData();

    new WindowGeometryManager(
        new ConfigFileVariantWrapper(m_configuration, "General", "AddConferenceWindowGeometry"), QRect(0, 50, 430, 400),
        this);
}

void AddConferenceWindow::createGui()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QWidget *mainWidget = new QWidget(this);
    mainLayout->addWidget(mainWidget);

    QFormLayout *layout = new QFormLayout(mainWidget);

    AccountCombo = m_injectedFactory->makeInjected<AccountsComboBox>(
        true, AccountsComboBox::NotVisibleWithOneRowSourceModel, this);
    AccountCombo->setIncludeIdInDisplay(true);

    // only gadu supports conferences for now
    // we need to add something like Protocol::supporterChatTypes()
    ProtocolFilter *protocolFilter = new ProtocolFilter(AccountCombo);
    protocolFilter->setProtocolName("gadu");
    AccountCombo->addFilter(protocolFilter);
    connect(AccountCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(accountChanged()));
    connect(AccountCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(validateData()));

    layout->addRow(tr("Account:"), AccountCombo);

    DisplayNameEdit = new QLineEdit(this);
    connect(DisplayNameEdit, SIGNAL(textChanged(QString)), this, SLOT(validateData()));

    layout->addRow(tr("Visible name:"), DisplayNameEdit);

    QLabel *hintLabel = new QLabel(tr("Enter a name for this conference if you want to have it on roster"));
    QFont hintLabelFont = hintLabel->font();
    hintLabelFont.setItalic(true);
    hintLabelFont.setPointSize(hintLabelFont.pointSize() - 2);
    hintLabel->setFont(hintLabelFont);
    layout->addRow(0, hintLabel);

    ModelChain *chain = new ModelChain(this);

    Model = m_injectedFactory->makeInjected<BuddyListModel>(chain);
    m_injectedFactory->makeInjected<BuddyManagerAdapter>(Model);

    // will be removed when Qt 4.8 .is required
    Model->setCheckable(true);
    connect(Model, SIGNAL(checkedBuddiesChanged(BuddySet)), this, SLOT(validateData()));

    chain->setBaseModel(Model);
    TalkableProxyModel *proxyModel = m_injectedFactory->makeInjected<TalkableProxyModel>(chain);
    proxyModel->setSortByStatusAndUnreadMessages(false);

    AccountFilter = new AccountTalkableFilter(proxyModel);
    proxyModel->addFilter(AccountFilter);

    proxyModel->addFilter(new HideAnonymousTalkableFilter(proxyModel));
    chain->addProxyModel(proxyModel);

    // will be used when Qt 4.8 .is required
    // CheckableProxy = new CheckableBuddiesProxyModel(chain);
    // connect(CheckableProxy, SIGNAL(checkedBuddiesChanged(BuddySet)), this, SLOT(validateData()));

    // chain->addProxyModel(CheckableProxy);

    FilteredTreeView *buddiesWidget =
        m_injectedFactory->makeInjected<FilteredTreeView>(FilteredTreeView::FilterAtBottom, this);

    NameTalkableFilter *nameFilter = new NameTalkableFilter(NameTalkableFilter::UndecidedMatching, proxyModel);
    connect(buddiesWidget, SIGNAL(filterChanged(QString)), nameFilter, SLOT(setName(QString)));
    proxyModel->addFilter(nameFilter);

    TalkableTreeView *view = m_injectedFactory->makeInjected<TalkableTreeView>(buddiesWidget);
    view->setModel(chain->lastModel());
    view->setRootIsDecorated(false);
    view->setShowIdentityNameIfMany(false);
    view->setContextMenuEnabled(true);

    buddiesWidget->setView(view);

    mainLayout->addWidget(buddiesWidget);

    ErrorLabel = new QLabel(this);
    QFont labelFont = ErrorLabel->font();
    labelFont.setBold(true);
    ErrorLabel->setFont(labelFont);
    mainLayout->addWidget(ErrorLabel);

    QDialogButtonBox *buttons = new QDialogButtonBox(mainWidget);

    AddButton = new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogOkButton), tr("Add Conference"), this);
    AddButton->setDefault(true);
    connect(AddButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    StartButton =
        new QPushButton(m_iconsManager->iconByPath(KaduIcon("internet-group-chat")), tr("Start Conference"), this);
    connect(StartButton, SIGNAL(clicked(bool)), this, SLOT(start()));

    QPushButton *cancel =
        new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Cancel"), this);
    connect(cancel, SIGNAL(clicked(bool)), this, SLOT(reject()));

    buttons->addButton(AddButton, QDialogButtonBox::AcceptRole);
    buttons->addButton(StartButton, QDialogButtonBox::ActionRole);
    buttons->addButton(cancel, QDialogButtonBox::DestructiveRole);

    mainLayout->addSpacing(32);
    mainLayout->addWidget(buttons);

    if (AccountCombo->currentAccount())
        DisplayNameEdit->setFocus();
    else
        AccountCombo->setFocus();
}

void AddConferenceWindow::displayErrorMessage(const QString &message)
{
    ErrorLabel->setText(message);
}

void AddConferenceWindow::validateData()
{
    AddButton->setEnabled(false);
    StartButton->setEnabled(false);

    Account account = AccountCombo->currentAccount();
    if (account.isNull() || !account.protocolHandler() || !account.protocolHandler()->protocolFactory())
    {
        displayErrorMessage(tr("Account is not selected"));
        return;
    }

    const BuddySet &conferenceBuddies = filterByAccount(account, Model->checkedBuddies());
    if (conferenceBuddies.size() < 2)
    {
        displayErrorMessage(tr("Select at least two buddies"));
        return;
    }

    StartButton->setEnabled(true);

    const QString &display = DisplayNameEdit->text();
    if (!display.isEmpty() && m_chatManager->byDisplay(display))
    {
        displayErrorMessage(tr("Visible name is already used for another chat"));
        return;
    }

    const Chat &chat = computeChat();
    Q_ASSERT(!chat.isNull());

    if (!chat.display().isEmpty())
    {
        displayErrorMessage(tr("This conference is already available as <i>%1</i>").arg(chat.display()));
        return;
    }

    if (display.isEmpty())
        displayErrorMessage(tr("Enter visible name to add this conference to roster"));
    else
        displayErrorMessage(QString());

    AddButton->setEnabled(!display.isEmpty());
}

void AddConferenceWindow::accountChanged()
{
    AccountFilter->setAccount(AccountCombo->currentAccount());
}

Chat AddConferenceWindow::computeChat() const
{
    const Account &account = AccountCombo->currentAccount();
    Q_ASSERT(account);

    const BuddySet &conferenceBuddies = filterByAccount(account, Model->checkedBuddies());
    ContactSet conferenceContacts;
    for (auto const &buddy : conferenceBuddies)
        conferenceContacts.insert(buddy.contacts(account).first());

    return ChatTypeContactSet::findChat(m_chatManager, m_chatStorage, conferenceContacts, ActionCreateAndAdd);
}

// we need to filter by account here because CheckableBuddiesProxyModel does not do filtering at all
BuddySet AddConferenceWindow::filterByAccount(const Account &account, const BuddySet &buddies) const
{
    BuddySet result;
    if (!account)
        return result;

    for (auto const &buddy : buddies)
        if (buddy.hasContact(account))
            result.insert(buddy);

    return result;
}

void AddConferenceWindow::accept()
{
    const Chat &chat = computeChat();
    Q_ASSERT(!chat.isNull());

    chat.setDisplay(DisplayNameEdit->text());

    QDialog::accept();
}

void AddConferenceWindow::start()
{
    const Chat &chat = computeChat();
    Q_ASSERT(!chat.isNull());

    // only update display chat name when not empty
    // this method can be called even when uer did not enter display name
    if (!DisplayNameEdit->text().isEmpty())
        chat.setDisplay(DisplayNameEdit->text());

    m_chatWidgetManager->openChat(computeChat(), OpenChatActivation::Activate);
    QDialog::accept();
}
