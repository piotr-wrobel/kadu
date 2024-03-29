/*
 * %kadu copyright begin%
 * Copyright 2011, 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "jabber-edit-account-widget.h"
#include "moc_jabber-edit-account-widget.cpp"

#include "gui/windows/jabber-change-password-window.h"
#include "jabber-protocol.h"

#include "accounts/account-manager.h"
#include "accounts/account.h"
#include "avatars/account-avatar-service.h"
#include "configuration/configuration-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "icons/icons-manager.h"
#include "identities/identity-manager.h"
#include "os/generic/system-info.h"
#include "plugin/plugin-injected-factory.h"
#include "widgets/account-avatar-widget.h"
#include "widgets/account-configuration-widget-tab-adapter.h"
#include "widgets/proxy-combo-box.h"
#include "widgets/simple-configuration-value-state-notifier.h"
#include "windows/message-dialog.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>

JabberEditAccountWidget::JabberEditAccountWidget(Account account, QWidget *parent) : AccountEditWidget(account, parent)
{
}

JabberEditAccountWidget::~JabberEditAccountWidget()
{
}

void JabberEditAccountWidget::setAccountManager(AccountManager *accountManager)
{
    m_accountManager = accountManager;
}

void JabberEditAccountWidget::setConfigurationManager(ConfigurationManager *configurationManager)
{
    m_configurationManager = configurationManager;
}

void JabberEditAccountWidget::setIconsManager(IconsManager *iconsManager)
{
    m_iconsManager = iconsManager;
}

void JabberEditAccountWidget::setIdentityManager(IdentityManager *identityManager)
{
    m_identityManager = identityManager;
}

void JabberEditAccountWidget::setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory)
{
    m_pluginInjectedFactory = pluginInjectedFactory;
}

void JabberEditAccountWidget::setSystemInfo(SystemInfo *systemInfo)
{
    m_systemInfo = systemInfo;
}

void JabberEditAccountWidget::init()
{
    createGui();
    loadAccountData();
    loadAccountDetailsData();
    simpleStateNotifier()->setState(StateNotChanged);
    stateChangedSlot(stateNotifier()->state());
}

void JabberEditAccountWidget::createGui()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QTabWidget *tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    createGeneralTab(tabWidget);
    createPersonalDataTab(tabWidget);
    createConnectionTab(tabWidget);
    createOptionsTab(tabWidget);

    new AccountConfigurationWidgetTabAdapter(this, tabWidget, this);

    QDialogButtonBox *buttons = new QDialogButtonBox(Qt::Horizontal, this);

    ApplyButton = new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogApplyButton), tr("Apply"), this);
    connect(ApplyButton, SIGNAL(clicked(bool)), this, SLOT(apply()));

    CancelButton = new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Cancel"), this);
    connect(CancelButton, SIGNAL(clicked(bool)), this, SLOT(cancel()));

    QPushButton *removeAccount =
        new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Delete account"), this);
    connect(removeAccount, SIGNAL(clicked(bool)), this, SLOT(removeAccount()));

    buttons->addButton(ApplyButton, QDialogButtonBox::ApplyRole);
    buttons->addButton(CancelButton, QDialogButtonBox::RejectRole);
    buttons->addButton(removeAccount, QDialogButtonBox::DestructiveRole);

    mainLayout->addWidget(buttons);

    connect(
        stateNotifier(), SIGNAL(stateChanged(ConfigurationValueState)), this,
        SLOT(stateChangedSlot(ConfigurationValueState)));
}

void JabberEditAccountWidget::createGeneralTab(QTabWidget *tabWidget)
{
    QWidget *generalTab = new QWidget(this);

    QGridLayout *layout = new QGridLayout(generalTab);
    QWidget *form = new QWidget(generalTab);
    layout->addWidget(form, 0, 0);

    QFormLayout *formLayout = new QFormLayout(form);

    AccountId = new QLineEdit(this);
    connect(AccountId, SIGNAL(textEdited(QString)), this, SLOT(dataChanged()));
    formLayout->addRow(tr("Username") + ':', AccountId);

    AccountPassword = new QLineEdit(this);
    AccountPassword->setEchoMode(QLineEdit::Password);
    connect(AccountPassword, SIGNAL(textEdited(QString)), this, SLOT(dataChanged()));
    formLayout->addRow(tr("Password") + ':', AccountPassword);

    RememberPassword = new QCheckBox(tr("Remember password"), this);
    RememberPassword->setChecked(true);
    connect(RememberPassword, SIGNAL(clicked()), this, SLOT(dataChanged()));
    formLayout->addRow(0, RememberPassword);

    QLabel *changePasswordLabel = new QLabel(QString("<a href='change'>%1</a>").arg(tr("Change your password")));
    changePasswordLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);
    formLayout->addRow(0, changePasswordLabel);
    connect(changePasswordLabel, SIGNAL(linkActivated(QString)), this, SLOT(changePasssword()));

    Identities = m_pluginInjectedFactory->makeInjected<IdentitiesComboBox>(this);
    connect(Identities, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged()));
    formLayout->addRow(tr("Account Identity") + ':', Identities);

    QLabel *infoLabel = new QLabel(
        tr("<font size='-1'><i>Select or enter the identity that will be associated with this account.</i></font>"),
        this);
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    infoLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    formLayout->addRow(0, infoLabel);

    AccountAvatarWidget *avatarWidget = m_pluginInjectedFactory->makeInjected<AccountAvatarWidget>(account(), this);
    layout->addWidget(avatarWidget, 0, 1, Qt::AlignTop);

    tabWidget->addTab(generalTab, tr("General"));
}

void JabberEditAccountWidget::createPersonalDataTab(QTabWidget *tabWidget)
{
    PersonalInfoWidget = m_pluginInjectedFactory->makeInjected<JabberPersonalInfoWidget>(account(), tabWidget);
    connect(PersonalInfoWidget, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
    tabWidget->addTab(PersonalInfoWidget, tr("Personal Information"));
}

void JabberEditAccountWidget::createConnectionTab(QTabWidget *tabWidget)
{
    QWidget *conenctionTab = new QWidget(this);
    tabWidget->addTab(conenctionTab, tr("Connection"));

    QVBoxLayout *layout = new QVBoxLayout(conenctionTab);
    createGeneralGroupBox(layout);

    layout->addStretch(100);
}

void JabberEditAccountWidget::createGeneralGroupBox(QVBoxLayout *layout)
{
    QGroupBox *general = new QGroupBox(this);
    general->setTitle(tr("XMPP Server"));
    layout->addWidget(general);

    QFormLayout *boxLayout = new QFormLayout(general);
    boxLayout->setSpacing(6);
    boxLayout->setMargin(9);

    CustomHostPort = new QCheckBox(general);
    CustomHostPort->setText(tr("Use custom server address/port"));
    boxLayout->addRow(CustomHostPort);

    CustomHostLabel = new QLabel(general);
    CustomHostLabel->setText(tr("Server address") + ':');

    CustomHost = new QLineEdit(general);
    connect(CustomHost, SIGNAL(textEdited(QString)), this, SLOT(dataChanged()));
    boxLayout->addRow(CustomHostLabel, CustomHost);

    CustomPortLabel = new QLabel(general);
    CustomPortLabel->setText(tr("Port") + ':');

    CustomPort = new QLineEdit(general);
    CustomPort->setMinimumSize(QSize(56, 0));
    CustomPort->setMaximumSize(QSize(56, 32767));
    CustomPort->setValidator(new QIntValidator(0, 9999999, CustomPort));
    connect(CustomPort, SIGNAL(textEdited(QString)), this, SLOT(dataChanged()));
    boxLayout->addRow(CustomPortLabel, CustomPort);

    // Manual Host/Port
    CustomHost->setEnabled(false);
    CustomHostLabel->setEnabled(false);
    CustomPort->setEnabled(false);
    CustomPortLabel->setEnabled(false);
    connect(CustomHostPort, SIGNAL(toggled(bool)), SLOT(hostToggled(bool)));
    connect(CustomHostPort, SIGNAL(clicked()), this, SLOT(dataChanged()));

    EncryptionModeLabel = new QLabel(general);
    EncryptionModeLabel->setText(tr("Use encrypted connection") + ':');

    EncryptionMode = new QComboBox(general);
    EncryptionMode->addItem(tr("Never"), JabberAccountData::Encryption_No);
    EncryptionMode->addItem(tr("Always"), JabberAccountData::Encryption_Yes);
    EncryptionMode->addItem(tr("When available"), JabberAccountData::Encryption_Auto);
    EncryptionMode->addItem(tr("Only in older version"), JabberAccountData::Encryption_Legacy);
    connect(EncryptionMode, SIGNAL(activated(int)), this, SLOT(dataChanged()));
    boxLayout->addRow(EncryptionModeLabel, EncryptionMode);

    QLabel *plainAuthLabel = new QLabel(general);
    plainAuthLabel->setText(tr("Allow plaintext authentication") + ':');

    PlainTextAuth = new QComboBox(general);
    PlainTextAuth->addItem(tr("Never"), JabberAccountData::NoAllowPlain);
    PlainTextAuth->addItem(tr("Always"), JabberAccountData::AllowPlain);
    PlainTextAuth->addItem(tr("Over encrypted connection"), JabberAccountData::AllowPlainOverTLS);
    connect(PlainTextAuth, SIGNAL(activated(int)), this, SLOT(dataChanged()));
    boxLayout->addRow(plainAuthLabel, PlainTextAuth);

    QGroupBox *connection = new QGroupBox(this);
    connection->setTitle(tr("Network"));
    layout->addWidget(connection);

    QFormLayout *connectionBoxLayout = new QFormLayout(connection);
    boxLayout->setSpacing(6);
    boxLayout->setMargin(9);

    QLabel *dataTransferProxyLabel = new QLabel(connection);
    dataTransferProxyLabel->setText(tr("Data transfer proxy") + ':');

    DataTransferProxy = new QLineEdit(connection);
    connect(DataTransferProxy, SIGNAL(textEdited(QString)), this, SLOT(dataChanged()));
    connectionBoxLayout->addRow(dataTransferProxyLabel, DataTransferProxy);

    RequireDataTransferProxy = new QCheckBox{tr("Require data transfer proxy:"), connection};
    connect(RequireDataTransferProxy, SIGNAL(toggled(bool)), this, SLOT(dataChanged()));
    connectionBoxLayout->addWidget(RequireDataTransferProxy);

    QLabel *proxyLabel = new QLabel(tr("Proxy configuration"), connection);
    ProxyCombo = m_pluginInjectedFactory->makeInjected<ProxyComboBox>(connection);
    ProxyCombo->enableDefaultProxyAction();
    connect(ProxyCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged()));

    connectionBoxLayout->addRow(proxyLabel, ProxyCombo);
}

void JabberEditAccountWidget::createOptionsTab(QTabWidget *tabWidget)
{
    QWidget *optionsTab = new QWidget(this);
    tabWidget->addTab(optionsTab, tr("Options"));

    QVBoxLayout *layout = new QVBoxLayout(optionsTab);
    layout->setSpacing(6);
    layout->setMargin(9);

    QGroupBox *resource = new QGroupBox(tr("Resource"), this);
    QFormLayout *resourceLayout = new QFormLayout(resource);

    AutoResource = new QCheckBox(tr("Use computer name as a resource"));
    connect(AutoResource, SIGNAL(clicked()), this, SLOT(dataChanged()));
    connect(AutoResource, SIGNAL(toggled(bool)), SLOT(autoResourceToggled(bool)));
    resourceLayout->addRow(AutoResource);

    ResourceLabel = new QLabel;
    ResourceLabel->setText(tr("Resource") + ':');

    ResourceName = new QLineEdit;
    connect(ResourceName, SIGNAL(textEdited(QString)), this, SLOT(dataChanged()));
    resourceLayout->addRow(ResourceLabel, ResourceName);

    PriorityLabel = new QLabel;
    PriorityLabel->setText(tr("Priority") + ':');

    Priority = new QLineEdit;
    connect(Priority, SIGNAL(textEdited(QString)), this, SLOT(dataChanged()));
    Priority->setValidator(new QIntValidator(Priority));
    resourceLayout->addRow(PriorityLabel, Priority);

    layout->addWidget(resource);

    QGroupBox *options = new QGroupBox(tr("Options"), this);
    QFormLayout *optionsLayout = new QFormLayout(options);

    SendTypingNotification = new QCheckBox(tr("Enable composing events"));
    SendTypingNotification->setToolTip(
        tr("Your interlocutor will be notified when you are typing a message, before it is sent. And vice versa."));
    connect(SendTypingNotification, SIGNAL(clicked()), this, SLOT(dataChanged()));
    optionsLayout->addRow(SendTypingNotification);

    SendGoneNotification = new QCheckBox(tr("Enable chat activity events"));
    SendGoneNotification->setToolTip(
        tr("Your interlocutor will be notified when you suspend or end conversation. And vice versa."));
    SendGoneNotification->setEnabled(false);
    connect(SendGoneNotification, SIGNAL(clicked()), this, SLOT(dataChanged()));
    connect(SendTypingNotification, SIGNAL(toggled(bool)), SendGoneNotification, SLOT(setEnabled(bool)));
    optionsLayout->addRow(SendGoneNotification);

    PublishSystemInfo = new QCheckBox(tr("Publish system information"));
    PublishSystemInfo->setToolTip(tr("Others can see your system name/version"));
    connect(PublishSystemInfo, SIGNAL(clicked()), this, SLOT(dataChanged()));
    optionsLayout->addRow(PublishSystemInfo);

    layout->addWidget(options);

    layout->addStretch(100);
}

void JabberEditAccountWidget::hostToggled(bool on)
{
    CustomHost->setEnabled(on);
    CustomPort->setEnabled(on);
    CustomHostLabel->setEnabled(on);
    CustomPortLabel->setEnabled(on);
    if (!on && EncryptionMode->currentIndex() == EncryptionMode->findData(JabberAccountData::Encryption_Legacy))
    {
        EncryptionMode->setCurrentIndex(JabberAccountData::Encryption_Auto);
    }
}

void JabberEditAccountWidget::autoResourceToggled(bool on)
{
    ResourceName->setEnabled(!on);
    ResourceLabel->setEnabled(!on);
}

void JabberEditAccountWidget::stateChangedSlot(ConfigurationValueState state)
{
    ApplyButton->setEnabled(state == StateChangedDataValid);
    CancelButton->setEnabled(state != StateNotChanged);
}

void JabberEditAccountWidget::dataChanged()
{
    auto accountData = JabberAccountData{account()};

    ConfigurationValueState widgetsState = stateNotifier()->state();

    if (account().accountIdentity() == Identities->currentIdentity() && account().id() == AccountId->text() &&
        account().rememberPassword() == RememberPassword->isChecked() &&
        account().password() == AccountPassword->text() && account().proxy() == ProxyCombo->currentProxy() &&
        account().useDefaultProxy() == ProxyCombo->isDefaultProxySelected() &&
        accountData.useCustomHostPort() == CustomHostPort->isChecked() &&
        accountData.customHost() == CustomHost->displayText() &&
        accountData.customPort() == CustomPort->displayText().toInt() &&
        accountData.encryptionMode() ==
            (JabberAccountData::EncryptionFlag)EncryptionMode->itemData(EncryptionMode->currentIndex()).toInt() &&
        accountData.plainAuthMode() ==
            (JabberAccountData::AllowPlainType)PlainTextAuth->itemData(PlainTextAuth->currentIndex()).toInt() &&
        accountData.autoResource() == AutoResource->isChecked() &&
        accountData.resource(*m_systemInfo) == ResourceName->text() &&
        accountData.priority() == Priority->text().toInt() &&
        accountData.dataTransferProxy() == DataTransferProxy->text() &&
        accountData.requireDataTransferProxy() == RequireDataTransferProxy->isChecked() &&
        accountData.sendGoneNotification() == SendGoneNotification->isChecked() &&
        accountData.sendTypingNotification() == SendTypingNotification->isChecked() &&
        accountData.publishSystemInfo() == PublishSystemInfo->isChecked() && !PersonalInfoWidget->isModified())
    {
        simpleStateNotifier()->setState(StateNotChanged);
        return;
    }

    bool sameIdExists = m_accountManager->byId(account().protocolName(), AccountId->text()) &&
                        m_accountManager->byId(account().protocolName(), AccountId->text()) != account();

    if (/*AccountName->text().isEmpty()
		|| sameNameExists
		|| */ AccountId->text()
            .isEmpty() ||
        sameIdExists || StateChangedDataInvalid == widgetsState)
        simpleStateNotifier()->setState(StateChangedDataInvalid);
    else
        simpleStateNotifier()->setState(StateChangedDataValid);
}

void JabberEditAccountWidget::loadAccountData()
{
    Identities->setCurrentIdentity(account().accountIdentity());
    AccountId->setText(account().id());
    RememberPassword->setChecked(account().rememberPassword());
    AccountPassword->setText(account().password());

    if (account().useDefaultProxy())
        ProxyCombo->selectDefaultProxy();
    else
        ProxyCombo->setCurrentProxy(account().proxy());
}

void JabberEditAccountWidget::loadAccountDetailsData()
{
    auto accountData = JabberAccountData{account()};

    CustomHostPort->setChecked(accountData.useCustomHostPort());
    CustomHost->setText(accountData.customHost());
    CustomPort->setText(QString::number(accountData.customPort()));
    EncryptionMode->setCurrentIndex(EncryptionMode->findData(accountData.encryptionMode()));
    PlainTextAuth->setCurrentIndex(PlainTextAuth->findData(accountData.plainAuthMode()));

    AutoResource->setChecked(accountData.autoResource());
    ResourceName->setText(accountData.resource(*m_systemInfo));
    Priority->setText(QString::number(accountData.priority()));
    DataTransferProxy->setText(accountData.dataTransferProxy());
    RequireDataTransferProxy->setChecked(accountData.requireDataTransferProxy());

    SendGoneNotification->setChecked(accountData.sendGoneNotification());
    SendTypingNotification->setChecked(accountData.sendTypingNotification());

    PublishSystemInfo->setChecked(accountData.publishSystemInfo());
}

void JabberEditAccountWidget::apply()
{
    auto accountData = JabberAccountData{account()};

    applyAccountConfigurationWidgets();

    account().setId(AccountId->text());
    account().setRememberPassword(RememberPassword->isChecked());
    account().setPassword(AccountPassword->text());
    account().setHasPassword(!AccountPassword->text().isEmpty());
    account().setUseDefaultProxy(ProxyCombo->isDefaultProxySelected());
    account().setProxy(ProxyCombo->currentProxy());
    // bad code: order of calls is important here
    // we have to set identity after password
    // so in cache of identity status container it already knows password and can do status change without asking user
    // for it
    account().setAccountIdentity(Identities->currentIdentity());
    accountData.setUseCustomHostPort(CustomHostPort->isChecked());
    accountData.setCustomHost(CustomHost->text());
    accountData.setCustomPort(CustomPort->text().toInt());
    accountData.setEncryptionMode(static_cast<JabberAccountData::EncryptionFlag>(
        EncryptionMode->itemData(EncryptionMode->currentIndex()).toInt()));
    accountData.setPlainAuthMode(
        static_cast<JabberAccountData::AllowPlainType>(PlainTextAuth->itemData(PlainTextAuth->currentIndex()).toInt()));
    accountData.setAutoResource(AutoResource->isChecked());
    accountData.setResource(ResourceName->text());
    accountData.setPriority(Priority->text().toInt());
    accountData.setDataTransferProxy(DataTransferProxy->text());
    accountData.setRequireDataTransferProxy(RequireDataTransferProxy->isChecked());
    accountData.setSendGoneNotification(SendGoneNotification->isChecked());
    accountData.setSendTypingNotification(SendTypingNotification->isChecked());
    accountData.setPublishSystemInfo(PublishSystemInfo->isChecked());

    if (PersonalInfoWidget->isModified())
        PersonalInfoWidget->apply();

    m_identityManager->removeUnused();
    m_configurationManager->flush();

    simpleStateNotifier()->setState(StateNotChanged);
}

void JabberEditAccountWidget::cancel()
{
    cancelAccountConfigurationWidgets();

    loadAccountData();
    loadAccountDetailsData();
    PersonalInfoWidget->cancel();

    m_identityManager->removeUnused();

    simpleStateNotifier()->setState(StateNotChanged);
}

void JabberEditAccountWidget::removeAccount()
{
    MessageDialog *dialog = MessageDialog::create(
        m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Confrim Account Removal"),
        tr("Are you sure do you want to remove account %1 (%2)?")
            .arg(account().accountIdentity().name())
            .arg(account().id()));
    dialog->addButton(QMessageBox::Yes, tr("Remove account"));
    dialog->addButton(QMessageBox::Cancel, tr("Cancel"));
    dialog->setDefaultButton(QMessageBox::Cancel);
    int decision = dialog->exec();

    if (decision == QMessageBox::Yes)
    {
        m_accountManager->removeAccountAndBuddies(account());
        deleteLater();
    }
}

void JabberEditAccountWidget::changePasssword()
{
    auto protocol = static_cast<JabberProtocol *>(account().protocolHandler());
    if (!protocol->isConnected())
    {
        MessageDialog::show(
            m_iconsManager->iconByPath(KaduIcon("dialog-warning")), tr("Kadu"), tr("Log in before changing password."),
            QMessageBox::Ok, this);
        return;
    }

    auto changePasswordWindow =
        m_pluginInjectedFactory->makeInjected<JabberChangePasswordWindow>(protocol->changePasswordService(), account());
    connect(
        changePasswordWindow, SIGNAL(passwordChanged(const QString &)), this, SLOT(passwordChanged(const QString &)));
    changePasswordWindow->show();
}

void JabberEditAccountWidget::passwordChanged(const QString &newPassword)
{
    AccountPassword->setText(newPassword);
}
