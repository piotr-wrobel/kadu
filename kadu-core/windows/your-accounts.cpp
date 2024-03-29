/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009, 2010, 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2010 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2010, 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "your-accounts.h"
#include "moc_your-accounts.cpp"

#include "accounts/account-manager.h"
#include "accounts/filter/have-protocol-filter.h"
#include "accounts/model/accounts-model.h"
#include "accounts/model/accounts-proxy-model.h"
#include "activate.h"
#include "configuration/config-file-variant-wrapper.h"
#include "configuration/configuration-manager.h"
#include "core/injected-factory.h"
#include "core/myself.h"
#include "icons/icons-manager.h"
#include "icons/kadu-icon.h"
#include "model/action-filter-proxy-model.h"
#include "model/action-list-model.h"
#include "model/merged-proxy-model-factory.h"
#include "model/model-chain.h"
#include "model/roles.h"
#include "os/generic/window-geometry-manager.h"
#include "protocols/filter/can-register-protocol-filter.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocol.h"
#include "protocols/protocols-manager.h"
#include "widgets/account-add-widget.h"
#include "widgets/account-create-widget.h"
#include "widgets/account-edit-widget.h"
#include "widgets/modal-configuration-widget.h"
#include "widgets/protocols-combo-box.h"
#include "windows/message-dialog.h"

#include <QtGui/QKeyEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>

YourAccounts::YourAccounts(QWidget *parent)
        : QWidget(parent), DesktopAwareObject(this), CurrentWidget(0), IsCurrentWidgetEditAccount(false),
          ForceWidgetChange(false), LastProtocol(0), CanRegisterFilter(new CanRegisterProtocolFilter())
{
}

YourAccounts::~YourAccounts()
{
}

void YourAccounts::setAccountManager(AccountManager *accountManager)
{
    m_accountManager = accountManager;
}

void YourAccounts::setConfigurationManager(ConfigurationManager *configurationManager)
{
    m_configurationManager = configurationManager;
}

void YourAccounts::setConfiguration(Configuration *configuration)
{
    m_configuration = configuration;
}

void YourAccounts::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void YourAccounts::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

void YourAccounts::setMyself(Myself *myself)
{
    m_myself = myself;
}

void YourAccounts::init()
{
    setWindowRole("kadu-your-accounts");

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Your accounts"));

    createGui();
    AccountsView->selectionModel()->select(AccountsView->model()->index(0, 0), QItemSelectionModel::ClearAndSelect);

    new WindowGeometryManager(
        new ConfigFileVariantWrapper(m_configuration, "General", "YourAccountsWindowGeometry"), QRect(0, 50, 700, 500),
        this);
}

void YourAccounts::createGui()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    mainLayout->addItem(contentLayout);

    AccountsView = new QListView(this);
    AccountsView->setMinimumWidth(150);
    contentLayout->addWidget(AccountsView);
    MyAccountsModel = m_injectedFactory->makeInjected<AccountsModel>(m_accountManager, AccountsView);
    auto *chain = new ModelChain{this};
    auto accountProxyModel = new AccountsProxyModel(chain);
    chain->setBaseModel(MyAccountsModel);
    accountProxyModel->addFilter(new HaveProtocolFilter(accountProxyModel));
    chain->addProxyModel(accountProxyModel);

    QAction *separator = new QAction(this);
    separator->setSeparator(true);

    AddExistingAccountAction =
        new QAction(m_iconsManager->iconByPath(KaduIcon("contact-new")), tr("Add existing account"), this);
    CreateNewAccountAction =
        new QAction(m_iconsManager->iconByPath(KaduIcon("system-users")), tr("Create new account"), this);

    ActionListModel *actionsModel = new ActionListModel(this);
    actionsModel->appendAction(separator);
    actionsModel->appendAction(AddExistingAccountAction);
    actionsModel->appendAction(CreateNewAccountAction);

    QList<QAbstractItemModel *> models;
    models.append(chain->lastModel());
    models.append(actionsModel);

    ActionFilterProxyModel *proxyModel = new ActionFilterProxyModel(this);
    proxyModel->setSourceModel(MergedProxyModelFactory::createInstance(models, this));
    proxyModel->setModel(chain->lastModel());
    proxyModel->addHideWhenModelEmpty(separator);

    AccountsView->setModel(proxyModel);
    AccountsView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    AccountsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    AccountsView->setIconSize(QSize(32, 32));
    connect(
        AccountsView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this,
        SLOT(accountSelectionChanged(const QItemSelection &, const QItemSelection &)));

    QDialogButtonBox *buttons = new QDialogButtonBox(Qt::Horizontal, this);
    mainLayout->addWidget(buttons);

    QPushButton *cancelButton =
        new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogCloseButton), tr("Close"), this);

    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    buttons->addButton(cancelButton, QDialogButtonBox::RejectRole);

    MainStack = new QStackedWidget(this);

    contentLayout->addWidget(MainStack, 100);

    createAccountWidget();
    createEditAccountWidget();
}

void YourAccounts::switchToCreateMode()
{
    MainAccountLabel->setText(tr("<font size='+2'><b>Create New Account</b></font>"));

    CanRegisterFilter->setEnabled(true);

    CurrentWidget = getAccountCreateWidget(Protocols->currentProtocol());
    if (CurrentWidget)
    {
        CreateAddStack->setCurrentWidget(CurrentWidget);
        CreateAddStack->show();
    }
    else
        CreateAddStack->hide();
}

void YourAccounts::switchToAddMode()
{
    MainAccountLabel->setText(tr("<font size='+2'><b>Add Existing Account</b></font>"));

    CanRegisterFilter->setEnabled(false);

    CurrentWidget = getAccountAddWidget(Protocols->currentProtocol());
    if (CurrentWidget)
    {
        CreateAddStack->setCurrentWidget(CurrentWidget);
        CreateAddStack->show();
    }
    else
        CreateAddStack->hide();
}

void YourAccounts::createAccountWidget()
{
    CreateAddAccountContainer = new QWidget(this);
    CreateAddAccountContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    MainStack->addWidget(CreateAddAccountContainer);
    MainStack->setCurrentWidget(CreateAddAccountContainer);

    QVBoxLayout *newAccountLayout = new QVBoxLayout(CreateAddAccountContainer);

    MainAccountLabel = new QLabel();
    newAccountLayout->addWidget(MainAccountLabel);

    QGroupBox *selectNetworkGroupbox = new QGroupBox(CreateAddAccountContainer);
    selectNetworkGroupbox->setFlat(true);

    QFormLayout *selectNetworkLayout = new QFormLayout(selectNetworkGroupbox);

    QLabel *imNetworkLabel = new QLabel(tr("IM Network") + ':', CreateAddAccountContainer);
    Protocols = m_injectedFactory->makeInjected<ProtocolsComboBox>(CreateAddAccountContainer);
    Protocols->addFilter(CanRegisterFilter);
    selectNetworkLayout->addRow(imNetworkLabel, Protocols);

    newAccountLayout->addWidget(selectNetworkGroupbox);

    MainAccountGroupBox = new QGroupBox(CreateAddAccountContainer);
    MainAccountGroupBox->setFlat(true);

    QGridLayout *createAccountLayout = new QGridLayout(MainAccountGroupBox);

    CreateAddStack = new QStackedWidget(MainAccountGroupBox);
    createAccountLayout->addWidget(CreateAddStack, 0, 1, 1, 1);

    newAccountLayout->addWidget(MainAccountGroupBox, Qt::AlignTop);

    connect(Protocols, SIGNAL(currentIndexChanged(int)), this, SLOT(protocolChanged()));

    switchToCreateMode();
}

void YourAccounts::createEditAccountWidget()
{
    EditStack = new QStackedWidget(this);
    EditStack->setContentsMargins(5, 5, 5, 5);
    MainStack->addWidget(EditStack);
}

AccountCreateWidget *YourAccounts::getAccountCreateWidget(ProtocolFactory *protocol)
{
    if (!protocol)
        return 0;

    if (!CreateWidgets.contains(protocol))
    {
        AccountCreateWidget *widget = protocol->newCreateAccountWidget(true, CreateAddStack);
        if (widget)
        {
            CreateWidgets.insert(protocol, widget);
            connect(widget, SIGNAL(accountCreated(Account)), this, SLOT(accountCreated(Account)));
            CreateAddStack->addWidget(widget);

            return widget;
        }
    }

    return CreateWidgets.value(protocol);
}

AccountAddWidget *YourAccounts::getAccountAddWidget(ProtocolFactory *protocol)
{
    if (!protocol)
        return 0;

    if (!AddWidgets.contains(protocol))
    {
        AccountAddWidget *widget = protocol->newAddAccountWidget(true, CreateAddStack);
        if (widget)
        {
            AddWidgets.insert(protocol, widget);
            connect(widget, SIGNAL(accountCreated(Account)), this, SLOT(accountCreated(Account)));
            CreateAddStack->addWidget(widget);

            return widget;
        }
    }

    return AddWidgets.value(protocol);
}

AccountEditWidget *YourAccounts::getAccountEditWidget(Account account)
{
    if (!account.protocolHandler() || !account.protocolHandler()->protocolFactory())
        return 0;

    if (!EditWidgets.contains(account))
    {
        AccountEditWidget *editWidget =
            account.protocolHandler()->protocolFactory()->newEditAccountWidget(account, this);
        EditWidgets.insert(account, editWidget);
        EditStack->addWidget(editWidget);

        return editWidget;
    }

    return EditWidgets.value(account);
}

void YourAccounts::protocolChanged()
{
    if (!canChangeWidget())
    {
        ForceWidgetChange = true;
        Protocols->setCurrentProtocol(LastProtocol);
        ForceWidgetChange = false;
        return;
    }

    updateCurrentWidget();
    LastProtocol = Protocols->currentProtocol();
}

void YourAccounts::updateCurrentWidget()
{
    QModelIndexList selection = AccountsView->selectionModel()->selectedIndexes();
    if (1 != selection.size())
        return;

    QAction *action = selection.at(0).data(ActionRole).value<QAction *>();
    if (!action)
    {
        MainStack->setCurrentWidget(EditStack);
        Account account = selection.at(0).data(AccountRole).value<Account>();
        CurrentWidget = getAccountEditWidget(account);
        if (CurrentWidget)
        {
            EditStack->setCurrentWidget(CurrentWidget);
            IsCurrentWidgetEditAccount = true;
        }

        return;
    }

    IsCurrentWidgetEditAccount = false;

    MainStack->setCurrentWidget(CreateAddAccountContainer);
    if (action == CreateNewAccountAction)
        switchToCreateMode();
    else if (action == AddExistingAccountAction)
        switchToAddMode();
}

bool YourAccounts::canChangeWidget()
{
    if (ForceWidgetChange)
        return true;

    if (!CurrentWidget)
        return true;

    if (StateNotChanged == CurrentWidget->stateNotifier()->state())
        return true;

    if (!IsCurrentWidgetEditAccount)
    {
        MessageDialog *dialog = MessageDialog::create(
            m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Unsaved changes"),
            tr("You have unsaved changes in current account.<br />Do you want to return to edit or discard changes?"));
        dialog->addButton(QMessageBox::Yes, tr("Return to edit"));
        dialog->addButton(QMessageBox::Ignore, tr("Discard changes"));
        dialog->addButton(QMessageBox::Cancel, tr("Cancel"));

        QMessageBox::StandardButton result = (QMessageBox::StandardButton)dialog->exec();

        switch (result)
        {
        case QMessageBox::Yes:
            return false;

        case QMessageBox::Ignore:
            CurrentWidget->cancel();
            return true;

        default:
            return false;
        }
    }

    if (StateChangedDataValid == CurrentWidget->stateNotifier()->state())
    {
        MessageDialog *dialog = MessageDialog::create(
            m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Unsaved changes"),
            tr("You have unsaved changes in current account.<br />Do you want to save them?"));
        dialog->addButton(QMessageBox::Save, tr("Save changes"));
        dialog->addButton(QMessageBox::Ignore, tr("Discard"));
        dialog->addButton(QMessageBox::Cancel, tr("Cancel"));

        QMessageBox::StandardButton result = (QMessageBox::StandardButton)dialog->exec();

        switch (result)
        {
        case QMessageBox::Save:
            CurrentWidget->apply();
            return true;

        case QMessageBox::Ignore:
            CurrentWidget->cancel();
            return true;

        default:
            return false;
        }
    }

    if (StateChangedDataInvalid == CurrentWidget->stateNotifier()->state())
    {
        MessageDialog *dialog = MessageDialog::create(
            m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Invalid changes"),
            tr("You have invalid changes in current account, which cannot be saved.<br />Do you want to stay in edit "
               "or discard changes?"));
        dialog->addButton(QMessageBox::Yes, tr("Stay in edit"));
        dialog->addButton(QMessageBox::Ignore, tr("Discard changes"));
        dialog->addButton(QMessageBox::Cancel, tr("Cancel"));

        QMessageBox::StandardButton result = (QMessageBox::StandardButton)dialog->exec();

        switch (result)
        {
        case QMessageBox::Yes:
            return false;

        case QMessageBox::Ignore:
            CurrentWidget->cancel();
            return true;

        default:
            return false;
        }
    }

    return true;
}

void YourAccounts::accountCreated(Account account)
{
    if (!account)
        return;

    m_accountManager->addItem(account);
    account.accountContact().setOwnerBuddy(m_myself->buddy());

    m_configurationManager->flush();
    selectAccount(account);
}

void YourAccounts::selectAccount(Account account)
{
    AccountsView->selectionModel()->clearSelection();

    const QModelIndexList &indexes = MyAccountsModel->indexListForValue(account);
    if (indexes.isEmpty())
        return;

    Q_ASSERT(indexes.size() == 1);

    AccountsView->selectionModel()->select(indexes.at(0), QItemSelectionModel::Select);
}

void YourAccounts::accountSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    if (canChangeWidget())
    {
        updateCurrentWidget();
        return;
    }

    ForceWidgetChange = true;
    AccountsView->selectionModel()->select(deselected, QItemSelectionModel::ClearAndSelect);
    ForceWidgetChange = false;
}

void YourAccounts::accountAdded(Account account)
{
    connect(account, SIGNAL(protocolHandlerChanged(Account)), this, SLOT(protocolHandlerChanged(Account)));
    protocolHandlerChanged(account);
}

void YourAccounts::accountRemoved(Account account)
{
    QMap<Account, AccountEditWidget *>::iterator i = EditWidgets.find(account);
    if (i != EditWidgets.end())
    {
        if (i.value() == CurrentWidget)
            CurrentWidget = 0;

        EditStack->removeWidget(i.value());
        i.value()->deleteLater();
        EditWidgets.erase(i);
    }
}

void YourAccounts::protocolHandlerChanged(Account account)
{
    if (!account.protocolHandler())
        accountRemoved(account);
}

void YourAccounts::okClicked()
{
    for (auto editWidget : EditWidgets)
        editWidget->apply();
    close();
}

void YourAccounts::closeEvent(QCloseEvent *e)
{
    if (canChangeWidget())
        e->accept();
    else
        e->ignore();
}

void YourAccounts::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
    {
        e->accept();
        close();
    }
    else
        QWidget::keyPressEvent(e);
}
