/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qradiobutton.h>
#include <qevent.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qstring.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qpoint.h>
#include <qlayout.h>
#include <qslider.h>
#include <qapplication.h>
#include <qhbuttongroup.h>
#include <qtextview.h>
#include <qtooltip.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qfont.h>
#include <qcursor.h>
#include <qmessagebox.h>
#include <qhgroupbox.h>
#include <qprocess.h>
#include <qsocketnotifier.h>
#include <qprogressdialog.h>
#include <qregexp.h>
#include <qlistview.h>
#include <qtabdialog.h>
#include <qstrlist.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qtextcodec.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qmenubar.h>
#include <qnetworkprotocol.h>
#include <qstringlist.h>
#include <qsplitter.h>
#include <qdatetime.h>
#include <qframe.h>
#include <qtoolbar.h>

//#include <arpa/inet.h>
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
//#include <iostream>

#include <ctype.h>
#include <errno.h>

#include <pwd.h>
#include <sys/stat.h>

#include <sys/wait.h>
//#include <sys/socket.h>
//#include <sys/ioctl.h>
//#include <net/if.h>
#include <netinet/in.h>

#include "libgadu.h"

//
#include "config_dialog.h"
#include "config_file.h"
#include "misc.h"
#include "kadu.h"
#include "userbox.h"
#include "events.h"
#include "chat.h"
#include "search.h"
#include "dcc.h"
#include "expimp.h"
#include "userinfo.h"
#include "personal_info.h"
#include "register.h"
#include "sms.h"
#include "adduser.h"
#include "about.h"
#include "ignore.h"
#include "history.h"
#include "pending_msgs.h"
#include "dock_widget.h"
#include "updates.h"
#include "password.h"
#include "tabbar.h"
#include "debug.h"
#include "sound.h"
#include "../config.h"
#ifdef HAVE_OPENSSL
#include "simlite.h"
#endif
//

#define GG_USER_OFFLINE	0x01
#define	GG_USER_NORMAL	0x03
#define GG_USER_BLOCKED	0x04

bool userlist_sent = FALSE;
bool socket_active = FALSE;
bool disconnect_planned = FALSE;
int userlist_count = 0;
int last_ping;
int last_read_event = -1;
int server_nr = 0;
int muteitem;
bool timeout_connected = true;
bool i_wanna_be_invisible = true;
struct gg_event *e;

QTime closestatusppmtime;
QTimer *blinktimer;
QTimer *pingtimer;
//QTimer *readevent; - patrz plik events.cpp
QPopupMenu *statusppm;
QPopupMenu *dockppm;
QLabel *statuslabel;
QLabel *statuslabeltxt;
QPopupMenu *grpmenu;

QValueList<struct chats> chats;
struct gg_session *sess = NULL;

QArray<struct acks> acks(0);
struct gg_dcc * dccsock;
struct gg_login_params loginparams;
QSocketNotifier *kadusnr = NULL;
QSocketNotifier *kadusnw = NULL;
QSocketNotifier *dccsnr = NULL;
QSocketNotifier *dccsnw = NULL;
UpdatesClass *uc;

QValueList<QHostAddress> gg_servers;
const char *gg_servers_ip[7] = {"217.17.41.82", "217.17.41.83", "217.17.41.84", "217.17.41.85",
	"217.17.41.86", "217.17.41.87", "217.17.41.88"};

/* sends the userlist. ripped off EKG, actually, but works */
void sendUserlist() {
	uin_t *uins;
	char *types;
	int i, j;

	for (i = 0, j = 0; i < userlist.count(); i++)
		if (userlist[i].uin)
			j++;

	uins = (uin_t *) malloc(j * sizeof(uin_t));
	types = (char *) malloc(j * sizeof(char));

	for (i = 0, j = 0; i < userlist.count(); i++)
		if (userlist[i].uin && !userlist[i].anonymous) {
			uins[j] = userlist[i].uin;
			if (userlist[i].offline_to_user)
				types[j] = GG_USER_OFFLINE;
			else
				if (userlist[i].blocking)
					types[j] = GG_USER_BLOCKED;
				else
					types[j] = GG_USER_NORMAL;
			j++;
			}

	/* we were popping up sometimes, so let's keep the server informed */
	if (i_wanna_be_invisible)
		gg_change_status(sess, GG_STATUS_INVISIBLE);

	gg_notify_ex(sess, uins, types, j);
	kdebug("send_userlist(): Userlist sent\n");

	free(uins);
}

void Kadu::gotUpdatesInfo(const QByteArray &data, QNetworkOperation *op) {
	char buf[32];
	int i;
	QString newestversion;

	if (data.size() > 31) {
		kdebug("Kadu::gotUpdatesInfo(): cannot obtain update info\n");		
		delete uc;
		return;
		}
	for (i = 0; i < data.size(); i++)
		buf[i] = data[i];
	buf[data.size()] = 0;
	newestversion = buf;

	kdebug("Kadu::gotUpdatesInfo(): %s\n", buf);

	if (uc->ifNewerVersion(newestversion)) {
		QMessageBox::information(this, i18n("Update information"),
			i18n("The newest Kadu version is %1").arg(newestversion), QMessageBox::Ok);
		}
	delete uc;
}

void Kadu::keyPressEvent(QKeyEvent *e) {
	config_file.setGroup("ShortCuts");
	if (e->key() == Key_Escape && trayicon) {
		kdebug("Kadu::keyPressEvent(Key_Escape): Kadu hide\n");
		hide();
	}
	else if (HotKey::shortCut(e,"kadu_deleteuser"))
	{
	if (userbox->isSelected(userbox->currentItem()))
		deleteUsers();
	}
	else if (HotKey::shortCut(e,"kadu_persinfo"))
	{
	if (userbox->isSelected(userbox->currentItem()))
	        showUserInfo();
	}	
	else if (HotKey::shortCut(e,"kadu_sendsms"))
	{
		sendSmsToUser();
	}
	else if (HotKey::shortCut(e,"kadu_viewhistory"))
	{
		viewHistory();
	}	
	else if (HotKey::shortCut(e,"kadu_searchuser"))
	{
		lookupInDirectory();
	}	
	else if (HotKey::shortCut(e,"kadu_showinactive"))
	{
		showHideInactive();
	}	
	else if (HotKey::shortCut(e,"kadu_sendfile"))
	{
		sendFile();
	}		
	else if (HotKey::shortCut(e,"kadu_configure"))
	{
		configure();
	}	

	
	QWidget::keyPressEvent(e);
}

/* a monstrous constructor so Kadu would take longer to start up */
Kadu::Kadu(QWidget *parent, const char *name) : QMainWindow(parent, name)
{
	UserBox::initModule();
	Chat::initModule();
	
	ConfigDialog::registerHotKeyEdit(i18n("Define keys"),i18n("Remove from userlist"),  "ShortCuts","kadu_deleteuser","Del");
	ConfigDialog::registerHotKeyEdit(i18n("Define keys"),i18n("View/edit user info"),"ShortCuts","kadu_persinfo","Ins");
	ConfigDialog::registerHotKeyEdit(i18n("Define keys"),i18n("Send SMS"),"ShortCuts","kadu_sendsms","Ctrl+S");
	ConfigDialog::registerHotKeyEdit(i18n("Define keys"),i18n("View history"),"ShortCuts","kadu_viewhistory","F11");
	ConfigDialog::registerHotKeyEdit(i18n("Define keys"),i18n("Lookup in directory"),"ShortCuts","kadu_searchuser","F10");
	ConfigDialog::registerHotKeyEdit(i18n("Define keys"), i18n("Show / hide inactive users"),"ShortCuts","kadu_showinactive","F9");
	ConfigDialog::registerHotKeyEdit(i18n("Define keys"), i18n("Send file"),"ShortCuts","kadu_sendfile","F8");
	ConfigDialog::registerHotKeyEdit(i18n("Define keys"), i18n("Configuration"),"ShortCuts","kadu_configure","F2");
	ConfigDialog::registerHotKeyEdit(i18n("Define keys"), i18n("Add user"),"ShortCuts","kadu_adduser","F3");
	Sms::initModule();
	closestatusppmtime.start();
	lastsoundtime.start();

	gg_proxy_host = NULL;

	/* timers, cause event loops and QSocketNotifiers suck. */
	
	//pingtimer = blinktimer = readevent = NULL; zamieniamy na(powod: patrz plik events.cpp)
	pingtimer = blinktimer = NULL;

	/* blinker */
	blinkOn = false;
	doBlink = false;

	/* another API change, another hack */
	memset(&loginparams, 0, sizeof(loginparams));
	loginparams.async = 1;

	/* active group, 600 is all groups */
	activegrpno = 600;

	int g;

	for (g = 0; g < acks.size(); g++)
		acks[g].seq = 0;

	loadKaduConfig();
        
	kdebug("Setting size: width=%d, height=%d and setting position: x=%d, y=%d\n",
		config.geometry.width(),config.geometry.height(),
		config.geometry.x(), config.geometry.y());
	resize(config.geometry.width(),config.geometry.height());
	move(config.geometry.x(),config.geometry.y());

	if (config.dock) {
		trayicon = new TrayIcon(this);
		trayicon->show();
		}

	/* read the userlist */
	userlist.readFromFile();

	readIgnored();

/*	DCC
Po jakiego czorta to ?
	QString buf;
	buf.append("Kadu: ");
	buf.append(QString::number(config.uin));
*/

	/* a newbie? */
	if (config.uin)
		setCaption(i18n("Kadu: %1").arg(config.uin));

	pending.loadFromFile();

	/* use dock icon? */
	if (config.dock) {
		trayicon->changeIcon();
		}

	centralFrame = new QFrame(this);
	setCentralWidget(centralFrame);

	QSplitter *split = new QSplitter(Qt::Vertical, centralFrame);

	QHBox *hbox1 = new QHBox(split);
	/* initialize group tabbar */
//	QVBox *vbox1 = new QVBox(hbox1);
//	vbox1->setMinimumWidth(10);
	group_bar = new KaduTabBar(hbox1, "groupbar");
	group_bar->setShape(QTabBar::RoundedBelow);
	group_bar->addTab(new QTab(i18n("All")));
	group_bar->setFont(QFont(config.fonts.userbox.family(),config.fonts.userbox.pointSize(),75));
//	group_bar->setMinimumWidth(group_bar->sizeHint().width());
	connect(group_bar, SIGNAL(selected(int)), this, SLOT(groupTabSelected(int)));

//	QHBox *hbox2 = new QHBox(vbox1);
//	hbox2->setMinimumWidth(10);

//	vbox1->setStretchFactor(group_bar, 1);
//	vbox1->setStretchFactor(hbox2, 100);

	/* connect userlist modified signal */
	connect(&userlist, SIGNAL(modified()), this, SLOT(userListModified()));
	connect(&userlist, SIGNAL(statusModified(UserListElement *)), this, SLOT(userListStatusModified(UserListElement *)));

	/* initialize and configure userbox */
	userbox = new UserBox(hbox1, "userbox");
	if (config.multicoluserbox)
		userbox->setColumnMode(QListBox::FitToWidth);
	userbox->setPaletteBackgroundColor(config.colors.userboxBg);
	userbox->setPaletteForegroundColor(config.colors.userboxFg);
	userbox->QListBox::setFont(config.fonts.userbox);
	userbox->setMinimumWidth(20);

	hbox1->setStretchFactor(group_bar, 1);
	hbox1->setStretchFactor(userbox, 100);

	/* add all users to userbox */
	setActiveGroup("");

	connect(userbox, SIGNAL(doubleClicked(QListBoxItem *)), this, SLOT(sendMessage(QListBoxItem *)));
	connect(userbox, SIGNAL(returnPressed(QListBoxItem *)), this, SLOT(sendMessage(QListBoxItem *)));
	connect(userbox, SIGNAL(rightButtonClicked(QListBoxItem *, const QPoint &)),
		this, SLOT(listPopupMenu(QListBoxItem *)));
	connect(userbox, SIGNAL(mouseButtonClicked(int, QListBoxItem *, const QPoint &)),
		this, SLOT(mouseButtonClicked(int, QListBoxItem *)));
	connect(userbox, SIGNAL(currentChanged(QListBoxItem *)), this, SLOT(currentChanged(QListBoxItem *)));

	statuslabeltxt = new MyLabel(centralFrame, "statuslabeltxt");
	statuslabeltxt->setText(i18n("Offline"));
	statuslabeltxt->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	statuslabeltxt->setFont(QFont("Verdana", 9));
	statuslabeltxt->setFixedWidth((width() - 45) > 50 ? width() - 45 : 50);

	/* a bit darker than the rest */
	statuslabeltxt->setPaletteBackgroundColor(QColor(
		qRed(statuslabeltxt->paletteBackgroundColor().rgb()) - 20,
		qGreen(statuslabeltxt->paletteBackgroundColor().rgb()) - 20,
		qBlue(statuslabeltxt->paletteBackgroundColor().rgb()) - 20));

	statuslabel = new MyLabel(centralFrame, "statuslabel");
	QPixmap *pix = icons->loadIcon("offline");
	statuslabel->setPixmap(*pix);
	statuslabel->setFixedWidth(pix->width());

	/* guess what */
	createMenu();
	createStatusPopupMenu();

	connect(statusppm, SIGNAL(aboutToHide()), this, SLOT(statusMenuAboutToHide()));

	dockppm->insertSeparator();
	dockppm->insertItem(loadIcon("exit.png"), i18n("&Exit Kadu"), 9);
	if (config.dock)
		trayicon->connectSignals();
//		connect(dockppm, SIGNAL(activated(int)), trayicon, SLOT(dockletChange(int)));

	descrtb = new QTextBrowser(split, "descrtb");
	descrtb->setFrameStyle(QFrame::NoFrame);
	descrtb->setMinimumHeight(int(1.5 * QFontMetrics(descrtb->font()).height()));
//	descrtb->resize(descrtb->size().width(), int(1.5 * QFontMetrics(descrtb->font()).height()));
	descrtb->setTextFormat(Qt::RichText);
	descrtb->setAlignment(Qt::AlignVCenter | Qt::WordBreak | Qt::DontClip);
	descrtb->setVScrollBarMode(QScrollView::AlwaysOff);
	descrtb->setPaletteBackgroundColor(config.colors.userboxDescBg);
	descrtb->setPaletteForegroundColor(config.colors.userboxDescText);
	descrtb->setFont(config.fonts.userboxDesc);
	if (!config.showdesc)
		descrtb->hide();
	QObject::connect(&userlist, SIGNAL(dnsNameReady(uin_t)), this, SLOT(infopanelUpdate(uin_t)));

	QValueList<int> splitsizes;
	
	splitsizes.append(config.splitsize.width());
	splitsizes.append(config.splitsize.height());
	split->setSizes(splitsizes);

//	tworzymy pasek narzedziowy
	QToolBar *toolbar = new QToolBar(this, "main toolbar");
	setRightJustification(true);
	setDockEnabled(Qt::DockBottom, false);
	setAppropriate(toolbar, true);
	toolbar->setCloseMode(QDockWindow::Undocked);
	toolbar->setLabel(i18n("Main toolbar"));

	config_file.setGroup("Other");

	QToolButton *inactivebtn = new QToolButton(*icons->loadIcon("offline"), i18n("Show / hide inactive users"),
	QString::null, this, SLOT(showHideInactive()), toolbar, "ShowHideInactive");

	
	QIconSet *mu;
	if (mute)
	{mu= new QIconSet(loadIcon("mute.png"));}
	else
	{mu= new QIconSet(loadIcon("unmute.png"));}
	mutebtn = new QToolButton((*mu), i18n("Mute sounds"),
	QString::null, this, SLOT(muteUnmuteSounds()), toolbar, "mute");

	QToolButton *configbtn = new QToolButton(loadIcon("configure.png"), i18n("Configuration"),
		QString::null, this, SLOT(configure()), toolbar, "configure");
		
	toolbar->addSeparator();
	QToolButton *viewhistorybtn = new QToolButton(loadIcon("history.png"), i18n("View history"),
		QString::null, this, SLOT(viewHistory()), toolbar, "viewhistory");
	QToolButton *userinfobtn = new QToolButton(loadIcon("identity.png"), i18n("View/edit user info"),
		QString::null, this, SLOT(showUserInfo()), toolbar, "userinfo");
	QToolButton *lookupbtn = new QToolButton(loadIcon("viewmag.png"),i18n("Lookup in directory"),
		QString::null, this, SLOT(lookupInDirectory()), toolbar, "lookup");
	toolbar->addSeparator();
	QToolButton *adduserbtn = new QToolButton(*icons->loadIcon("online"), i18n("Add user"),
		QString::null, this, SLOT(addUserAction()), toolbar, "adduser");
	QFrame *toolbarfiller = new QFrame(toolbar);
	toolbar->setStretchableWidget(toolbarfiller);
	toolbar->setVerticallyStretchable(true);

	if (config.dockwindows != QString::null) {
		config.dockwindows.replace(QRegExp("\\\\n"), "\n");
		QTextStream stream(&config.dockwindows, IO_ReadOnly);
		stream >> *this;
		}

//	tworzymy liste serverow domyslnych gg
	QHostAddress ip;
	for (int i = 0; i < 7; i++) {
		ip.setAddress(QString(gg_servers_ip[i]));
		gg_servers.append(ip);
		}

//	tworzymy gridlayout
	grid = new QGridLayout(centralFrame, 2, 6);
	grid->addMultiCellWidget(split, 0, 0, 0, 5);
	grid->addWidget(statuslabeltxt, 1, 0, Qt::AlignLeft);
	grid->addWidget(statuslabel, 1, 5, Qt::AlignRight);
	grid->setColStretch(0, 5);
	grid->setColStretch(1, 1);
	grid->setColStretch(2, 1);
	grid->setColStretch(3, 1);
	grid->setColStretch(4, 1);
	grid->setColStretch(5, 1);
	grid->setRowStretch(0, 40);
	grid->setRowStretch(1, 1);
	grid->activate();

	centralFrame->setMinimumSize(50, 100);

	refreshGroupTabBar();

	dccsock = NULL;
	/* dirty workaround for multiple showEvents */
	commencing_startup = true;

	/* pokaz okno jesli RunDocked jest wylaczone lub dock wylaczone */
	if ((!config.rundocked) || (!config.dock))
		show();

	autostatus_timer = new AutoStatusTimer(this);
	if (config.addtodescription)
		autostatus_timer->start(1000,TRUE);

	if (config.uin) {
		uc = new UpdatesClass(config.uin);
		if (config.checkupdates)
			QObject::connect(uc->op, SIGNAL(data(const QByteArray &, QNetworkOperation *)),
				this, SLOT(gotUpdatesInfo(const QByteArray &, QNetworkOperation *)));
		uc->run();
		}
}

void Kadu::showHideInactive() 
{
	userbox->showHideInactive();
}

void Kadu::muteUnmuteSounds()
{
	mute = !mute;
	if (mute) {
		mutebtn->setIconSet(loadIcon("mute.png"));
		mutebtn->setTextLabel(i18n("Unmute sounds"));
		mmb->changeItem(muteitem, loadIcon("mute.png"), i18n("Unmute sounds"));
		}
	else {
		mmb->changeItem(muteitem, loadIcon("unmute.png"), i18n("Mute sounds"));
		mutebtn->setTextLabel(i18n("Mute sounds"));
		mutebtn->setIconSet(loadIcon("unmute.png"));
		}
}

void Kadu::configure() 
{
	ConfigDialog::showConfigDialog();	
}

void Kadu::sendSms()
{
	Sms *sms = new Sms("", 0);
	sms->show();
}

void Kadu::sendSmsToUser()
{
	if ((userbox->isSelected(userbox->currentItem())) &&
	(userlist.byAltNick(kadu->userbox->currentText()).mobile.length()!=0)) {
		Sms *sms = new Sms(userbox->currentText(), 0);
		sms->show();
		}	
}


void Kadu::viewHistory() {
	UinsList uins;
	for (int i = 0; i < userbox->count(); i++)
		if (userbox->isSelected(i))
			uins.append(userlist.byAltNick(userbox->text(i)).uin);
		if (uins.count()) {
			History *hb = new History(uins);
			hb->show();
			}
}

void Kadu::sendFile()
{
	struct gg_dcc *dcc_new;
	if (userbox->currentText()=="")
		return;
	UserListElement user = userlist.byAltNick(userbox->currentText());
	if (user.port >= 10) {
		if ((dcc_new = gg_dcc_send_file(htonl(user.ip.ip4Addr()), user.port, config.uin, user.uin)) != NULL) {
			dccSocketClass *dcc = new dccSocketClass(dcc_new);
			connect(dcc, SIGNAL(dccFinished(dccSocketClass *)), this, SLOT(dccFinished(dccSocketClass *)));
			dcc->initializeNotifiers();
			}
		}
	else {
		acks.resize(acks.size() + 1);
		int i = acks.size() - 1;
		acks[i].ack = 0;
		acks[i].seq = gg_dcc_request(sess, user.uin);
		acks[i].type = 0;
		acks[i].ptr = NULL;
		}
}


void Kadu::lookupInDirectory() {
	if (userbox->currentItem() != -1) {
		SearchDialog *sd = new SearchDialog(0, i18n("User info"),
			userlist.byAltNick(userbox->currentText()).uin);
		sd->init();
		sd->show();
		sd->firstSearch();
		}
}

void Kadu::showUserInfo() {
	if (userbox->currentItem() != -1) 
		{
		UserInfo *ui = new UserInfo("user info", 0, userbox->currentText());
		ui->show();
		}
}

void Kadu::deleteUsers() 
{
	QStringList users;
	for (int i = 0; i < userbox->count(); i++)
		if (userbox->isSelected(i))
			users.append(userbox->text(i));
	removeUser(users,false);
}

void Kadu::personalInfo()
{
	PersonalInfoDialog *pid = new PersonalInfoDialog();
	pid->show();
}

void Kadu::addUserAction() {
	Adduser *au = new Adduser(0, "add_user");
	au->show();
}

void Kadu::sendKey()
{
#ifdef HAVE_OPENSSL
	UserListElement user = userlist.byAltNick(userbox->currentText());
	QString keyfile_path;
	QString mykey;
	QFile keyfile;

	keyfile_path.append(ggPath("keys/"));
	keyfile_path.append(QString::number(config.uin));
	keyfile_path.append(".pem");

	keyfile.setName(keyfile_path);

	if (keyfile.open(IO_ReadOnly)) {
		QTextStream t(&keyfile);
		mykey = t.read();
		keyfile.close();
		QCString tmp(mykey.local8Bit());
		gg_send_message(sess, GG_CLASS_MSG, user.uin, (unsigned char *)tmp.data());
		QMessageBox::information(this, "Kadu",
			i18n("Your public key has been sent"), i18n("OK"), QString::null, 0);
		}
#endif
}

void Kadu::deleteHistory()
{
	UinsList uins;
	for (int i = 0; i < userbox->count(); i++)
		if (userbox->isSelected(i))
			uins.append(userlist.byAltNick(userbox->text(i)).uin);
	history.removeHistory(uins);
}

void Kadu::manageIgnored()
{
	Ignored *ign = new Ignored(0, "ignored");
	ign->show();
}

void Kadu::openChat()
{
	PendingMsgs::Element elem;
	int l,k;
	bool stop = false;
	UinsList uins = getSelectedUins();
	QString toadd;
	for (int i = 0; i < pending.count(); i++) {
		elem = pending[i];
		if (elem.uins.equals(uins))
			if ((elem.msgclass & GG_CLASS_CHAT) == GG_CLASS_CHAT
				|| (elem.msgclass & GG_CLASS_MSG) == GG_CLASS_MSG
				|| !elem.msgclass) {
				l = chats.count();
				k = openChat(elem.uins);
				QValueList<UinsList>::iterator it = wasFirstMsgs.begin();
				while (it != wasFirstMsgs.end() && !elem.uins.equals(*it))
					it++;
				if (it != wasFirstMsgs.end())
					wasFirstMsgs.remove(*it);
				if (l < chats.count())
					chats[k].ptr->writeMessagesFromHistory(elem.uins, elem.time);
				chats[k].ptr->formatMessage(false,
					userlist.byUin(elem.uins[0]).altnick, elem.msg,
					timestamp(elem.time), toadd);
				pending.deleteMsg(i);
				i--;
				stop = true;
				}
		}
	if (!stop) {
		k = openChat(uins);
		chats[k].ptr->writeMessagesFromHistory(uins, 0);
		}
}

void Kadu::searchInDirectory()
{
	SearchDialog *sd = new SearchDialog();
	sd->init();
	sd->show();
}

void Kadu::help()
{
	QProcess *help = new QProcess();
	help->addArgument("konqueror");
	if (QFile::exists(QString(DOCDIR)+"/index_doc.html"))
		help->addArgument(QString(DOCDIR)+"/index_doc.html");
	else
		help->addArgument("http://kadu.net/index_doc.html");
	help->start();
	delete help;
}

void Kadu::about()
{
	About *about = new About;
	about->show();
}

void Kadu::remindPassword1()
{
	remindPassword *rp = new remindPassword();
	rp->start();
}

void Kadu::changePassword1()
{
	changePassword *cp = new changePassword();
	cp->show();
}

void Kadu::registerUser()
{
	Register *reg = new Register;
	reg->show();
}
void Kadu::unregisterUser()
{
	Unregister *ureg = new Unregister;
	ureg->show();
}

void Kadu::quit()
{
	close(true);
}

void Kadu::exportUserlist()
{
	UserlistExport *ule = new UserlistExport();
	ule->show();
}

void Kadu::importUserlist()
{
	UserlistImport *uli = new UserlistImport();
	uli->show();
}


void Kadu::hideKadu()
{
	if (trayicon)
		close();
}

void Kadu::sendUserlist1()
{
	sendUserlist();
}

void Kadu::ignoreUser()
{
	UinsList uins = getSelectedUins();
	if (isIgnored(uins))
		delIgnored(uins);
	else
		addIgnored(uins);
	writeIgnored();
}

void Kadu::blockUser()
{
	UserListElement *puser = &userlist.byAltNick(userbox->currentText());
	puser->blocking = !puser->blocking;
	gg_remove_notify_ex(sess, puser->uin, puser->blocking ? GG_USER_NORMAL : GG_USER_BLOCKED);
	gg_add_notify_ex(sess, puser->uin, puser->blocking ? GG_USER_BLOCKED : GG_USER_NORMAL);
	userlist.writeToFile();
}

void Kadu::notifyUser()
{
	UserListElement *puser = &userlist.byAltNick(userbox->currentText());
	puser->notify = !puser->notify;
	userlist.writeToFile();
}

void Kadu::offlineToUser()
{
	UserListElement *puser = &userlist.byAltNick(userbox->currentText());
	puser->offline_to_user = !puser->offline_to_user;
	gg_remove_notify_ex(sess, puser->uin, puser->offline_to_user ? GG_USER_NORMAL : GG_USER_OFFLINE);
	gg_add_notify_ex(sess, puser->uin, puser->offline_to_user ? GG_USER_OFFLINE : GG_USER_NORMAL);
	userlist.writeToFile();
}

void Kadu::resizeEvent(QResizeEvent *e) {
	statuslabeltxt->setFixedWidth((width() - 45) > 50 ? width() - 45 : 50);
}

void Kadu::changeAppearance() {
	kdebug("kadu::changeAppearance()\n");

	userbox->setPaletteBackgroundColor(config.colors.userboxBg);
	userbox->setPaletteForegroundColor(config.colors.userboxFg);
	userbox->QListBox::setFont(config.fonts.userbox);

	group_bar->setFont(QFont(config.fonts.userbox.family(),config.fonts.userbox.pointSize(),75));

	descrtb->setPaletteBackgroundColor(config.colors.userboxDescBg);
	descrtb->setPaletteForegroundColor(config.colors.userboxDescText);
	descrtb->setFont(config.fonts.userboxDesc);
}

void Kadu::currentChanged(QListBoxItem *item) {
	if (!item || !item->isSelected())
		return;

	kdebug("Kadu::currentChanged(): %s\n", (const char *)item->text().local8Bit());

	if (config.showdesc)
		descrtb->setText(parse(config.panelsyntax, userlist.byAltNick(item->text())));
}

void Kadu::refreshGroupTabBar()
{
	if (!config.grouptabs)
		{
		group_bar->hide();
		return;
		}
	/* budujemy list� grup */
	QValueList<QString> group_list;
	for (int i = 0; i < userlist.count(); i++)
		{
		QString groups = userlist[i].group();
		QString group;
		for (int g = 0; (group = groups.section(',' ,g ,g)) != ""; g++)
			if(!group_list.contains(group))
				group_list.append(group);
		}
	kdebug("%i groups found\n",group_list.count());
	//
	if (group_list.count() == 0)
		{
		group_bar->hide();
		setActiveGroup("");
		return;
		}
	/* usuwamy wszystkie niepotrzebne zakladki - od tylu,
	   bo indeksy sie przesuwaja po usunieciu */
	for (int i = group_bar->count() - 1; i >= 1; i--)
		if(!group_list.contains(group_bar->tabAt(i)->text()))
			group_bar->removeTab(group_bar->tabAt(i));
	/* dodajemy nowe zakladki */
	for (int i = 0; i < group_list.count(); i++)
		{
		bool createNewTab = true;
		for (int j = 0; j < group_bar->count(); j++)
			if (group_bar->tabAt(j)->text() == group_list[i])
				createNewTab = false;
		if(createNewTab)
			group_bar->addTab(new QTab(group_list[i]));
		}
	kdebug("%i group tabs\n", group_bar->count());
	group_bar->show();
	/* odswiezamy - dziala tylko jesli jest widoczny */
	group_bar->update();
};

void Kadu::setActiveGroup(const QString& group)
{
	userbox->clearUsers();
	for (int i = 0; i < userlist.count(); i++)
		{
		bool belongsToGroup;
		if (group == "")
			belongsToGroup = true;
		else
			{
			belongsToGroup = false;
			QString user_groups = userlist[i].group();
			QString user_group;
			for (int g = 0; (user_group = user_groups.section(',',g,g)) != ""; g++)
				if (user_group == group)
					belongsToGroup = true;
			}
		if (belongsToGroup && (!userlist[i].anonymous || !trayicon))
			userbox->addUser(userlist[i].altnick);
		}
	UserBox::all_refresh();
};

void Kadu::groupTabSelected(int id)
{
	if (id == 0)
		setActiveGroup("");
	else
		setActiveGroup(group_bar->tab(id)->text());
};

void Kadu::userListModified()
{
	refreshGroupTabBar();
};

void Kadu::userListStatusModified(UserListElement *user)
{
	kdebug("Kadu::userListStatusModified(): %d\n", user->uin);

//	int index = userbox->currentItem();
//	if (index >= 0) {
//		QListBoxItem *lbi = userbox->item(index);
//		UserListElement &u = userlist.byAltNick(lbi->text());
//		if (u.uin == user->uin)
//			currentChanged(lbi);
//		}
	for (int i = 0; i < chats.count(); i++)
		if (chats[i].uins.contains(user->uin))
			chats[i].ptr->setTitle();
};

void Kadu::removeUser(QStringList &users, bool permanently = false)
{
	if(QMessageBox::warning(kadu, "Kadu",
		i18n("Selected users will be deleted. Are you sure?"),
		i18n("&Yes"),i18n("&No"))!=0)
		return;

	int i;

	for (i = 0; i < users.count(); i++)
		UserBox::all_removeUser(users[i]);
	UserBox::all_refresh();

	for (i = 0; i < users.count(); i++) {
		UserListElement user = userlist.byAltNick(users[i]);
		if (!user.anonymous && user.uin)
			gg_remove_notify(sess, user.uin);
		userlist.removeUser(user.altnick);
		}

	userlist.writeToFile();
	refreshGroupTabBar();
}

void Kadu::blink() {
	int i;
	QPixmap *pix;

	if (!doBlink && socket_active) {
		setCurrentStatus(loginparams.status & (~GG_STATUS_FRIENDS_MASK));
		return;
		}
	else
		if (!doBlink && !socket_active) {
			pix = icons->loadIcon("offline");
	    		statuslabel->setPixmap(*pix);
	    		if (trayicon)
				trayicon->setType(*pix);
	    		return;
	    		}

	if (blinkOn) {
		pix = icons->loadIcon("offline");
		statuslabel->setPixmap(*pix);
		if (trayicon)
			trayicon->setType(*pix);
		blinkOn = false;
		}
	else {
		i = statusGGToStatusNr(loginparams.status & (~GG_STATUS_FRIENDS_MASK));
		pix = icons->loadIcon(gg_icons[i]);
		statuslabel->setPixmap(*pix);
		if (trayicon)
			trayicon->setType(*pix);
		blinkOn = true;
		}

	blinktimer->start(1000, TRUE);
}

/* dcc initials */
void Kadu::prepareDcc(void) {
	QHostAddress dccip;

	if (!config.dccip.ip4Addr())
		dccip.setAddress("255.255.255.255");
	else
		dccip = config.dccip;

	dccsock = gg_dcc_socket_create(config.uin, 0);

	if (!dccsock) {
		kdebug("Kadu::prepareDcc(): Couldn't bind DCC socket.\n");
		gg_dcc_free(dccsock);
		QMessageBox::warning(kadu, "",
			i18n("Couldn't create DCC socket.\nDirect connections disabled."));
		return;
		}

	gg_dcc_ip = htonl(dccip.ip4Addr());
	gg_dcc_port = dccsock->port;

	kdebug("Kadu::prepareDcc() DCC_IP=%s DCC_PORT=%d\n", dccip.toString().latin1(), dccsock->port);

	dccsnr = new QSocketNotifier(dccsock->fd, QSocketNotifier::Read, kadu);
	QObject::connect(dccsnr, SIGNAL(activated(int)), kadu, SLOT(dccReceived()));

	dccsnw = new QSocketNotifier(dccsock->fd, QSocketNotifier::Write, kadu);
	QObject::connect(dccsnw, SIGNAL(activated(int)), kadu, SLOT(dccSent()));
}

// code for addUser has been moved from adduser.cpp
// for sharing with search.cpp
void Kadu::addUser(const QString &FirstName, const QString &LastName,
	const QString &NickName, const QString &AltNick,
	const QString &Mobile, const QString &Uin, const int Status,
	const int Image_size,
	const QString &Group, const QString &Description, const QString &Email,
	const bool Anonymous)
{
	uint32_t uin = Uin.toUInt();

	if (!userlist.containsUin(uin) || (!uin && !userlist.containsAltNick(AltNick)))
		userlist.addUser(FirstName, LastName, NickName, AltNick, Mobile, Uin, Status, 
			Image_size, false, false, true, Group, Description, Email, Anonymous);
	else {
		UserListElement &ule = userlist.byUin(uin);
		if (!uin)
			ule = userlist.byAltNick(AltNick);
		userlist.changeUserInfo(ule.altnick,
			FirstName, LastName, NickName, AltNick, Mobile, Uin, ule.status,
			ule.image_size,
			ule.blocking, ule.offline_to_user, ule.notify, Group, Email);
		}
	userlist.writeToFile();

	userbox->addUser(AltNick);
	UserBox::all_refresh();

	refreshGroupTabBar();

	if (!Anonymous && uin)
		gg_add_notify(sess, uin);
};

int Kadu::openChat(UinsList senders) {
	int i;
	UinsList uins;

	i = 0;
	while (i < chats.count() && !chats[i].uins.equals(senders))
		i++;

	if (i == chats.count()) {
		Chat *chat;
		uins = senders;
		chat = new Chat(uins, 0);
		chat->setTitle();
		chat->show();
		}
	else {
		chats[i].ptr->raise();
		chats[i].ptr->setActiveWindow();
		return i;
		}

	i = 0;
	while (i < chats.count() && !chats[i].uins.equals(senders))
		i++;

	kdebug("Kadu::openChat(): return %d\n", i);

	return i;
}

UinsList Kadu::getSelectedUins() {
	UinsList uins;
	UserListElement user;
	for (int i = 0; i < userbox->count(); i++)
		if (userbox->isSelected(i)) {
			user = userlist.byAltNick(userbox->text(i));
			if (user.uin)
				uins.append(user.uin);
			}
	return uins;
}

/* changes the active group */
void Kadu::changeGroup(int group) {
	activegrpno = group;
	grpmenu->setItemChecked(true, group - 600);
	grpmenu->updateItem(group - 600);
	grpmenu->repaint();
	kdebug("Kadu::changeGroup(): group = %d\n", group - 600);
}

void Kadu::mouseButtonClicked(int button, QListBoxItem *item) {
	kdebug("Kadu::mouseButtonClicked(): button=%d\n", button);
	if (button !=4 || !item)
		return;
	UserListElement user;
	user = userlist.byAltNick(item->text());
	if (user.mobile.length())	
		sendSmsToUser();
}

/* the list that pops up if we right-click one someone */
void Kadu::listPopupMenu(QListBoxItem *item) {
	if (item == NULL)
		return;

	QPopupMenu * pm;
	pm = new QPopupMenu(this);

	QPixmap msg;
	msg = loadIcon("mail_generic.png");
	int smsitem;
	int sendfile;
	int deletehistoryitem;
	int historyitem;
	int searchuser;
	int openchatitem;
	int ignoreuseritem;
	int blockuseritem;
	int notifyuseritem;
	int offlinetouseritem;
	UserListElement user;
	user = userlist.byAltNick(item->text());

//	pm->insertItem(msg, i18n("Send message"), KADU_CMD_SEND_MESSAGE);
	openchatitem= pm->insertItem(i18n("Open chat window") ,this, SLOT(openChat()));
	smsitem= pm->insertItem(i18n("Send SMS"),this,SLOT(sendSmsToUser()),HotKey::shortCutFromFile("kadu_sendsms"));
	if (!user.mobile.length())
		pm->setItemEnabled(smsitem,false);

	sendfile= pm->insertItem(loadIcon("filesave.png"), i18n("Send file"), this, SLOT(sendFile()),HotKey::shortCutFromFile("kadu_sendfile"));
	if (dccSocketClass::count >= 8)
		pm->setItemEnabled(sendfile, false);
	if (user.status == GG_STATUS_AVAIL || user.status == GG_STATUS_AVAIL_DESCR ||
		user.status == GG_STATUS_BUSY || user.status == GG_STATUS_BUSY_DESCR)
		pm->setItemEnabled(sendfile, true);
	else
		pm->setItemEnabled(sendfile, false);

#ifdef HAVE_OPENSSL
	int sendkeyitem;
	sendkeyitem= pm->insertItem(loadIcon("encrypted.png"), i18n("Send my public key"), this, SLOT(sendKey()));

	QString keyfile_path;

	keyfile_path.append(ggPath("keys/"));
	keyfile_path.append(QString::number(config.uin));
	keyfile_path.append(".pem");

	QFileInfo keyfile(keyfile_path);
	if (keyfile.permission(QFileInfo::ReadUser) && user.uin)
		pm->setItemEnabled(sendkeyitem, true);
	else
		pm->setItemEnabled(sendkeyitem, false);

#endif

	pm->insertSeparator();
	ignoreuseritem= pm->insertItem(i18n("Ignore user"), this, SLOT(ignoreUser()));
	blockuseritem= pm->insertItem(i18n("Block user"), this, SLOT(blockUser()));
	notifyuseritem= pm->insertItem(i18n("Notify about user"), this, SLOT(notifyUser()));
	offlinetouseritem= pm->insertItem(i18n("Offline to user"), this, SLOT(offlineToUser()));
	if (!user.uin) {
		pm->setItemEnabled(ignoreuseritem, false);
		pm->setItemEnabled(blockuseritem, false);
		pm->setItemEnabled(notifyuseritem, false);
		pm->setItemEnabled(offlinetouseritem, false);
		}
	else {
		UinsList uins;
		uins = getSelectedUins();
		if (isIgnored(uins))
			pm->setItemChecked(ignoreuseritem, true);
		if (user.blocking)
			pm->setItemChecked(blockuseritem, true);
		pm->setItemEnabled(offlinetouseritem, config.privatestatus);
		if (user.offline_to_user)
			pm->setItemChecked(offlinetouseritem, true);
		pm->setItemEnabled(notifyuseritem, config.notifyglobal && !config.notifyall);
		if (user.notify)
			pm->setItemChecked(notifyuseritem, true);
		}

	pm->insertSeparator();
	pm->insertItem(loadIcon("remove.png"), i18n("Remove from userlist"), this, SLOT(deleteUsers()),HotKey::shortCutFromFile("kadu_deleteuser"));
	deletehistoryitem= pm->insertItem(loadIcon("eraser.png"), i18n("Clear history"));
	QPixmap history;
	history = loadIcon("history.png");
	historyitem= pm->insertItem(history, i18n("View history"),this,SLOT(viewHistory()),HotKey::shortCutFromFile("kadu_viewhistory"));
	pm->insertItem(loadIcon("identity.png"), i18n("View/edit user info"), this, SLOT(showUserInfo()),HotKey::shortCutFromFile("kadu_persinfo"));
	searchuser= pm->insertItem(loadIcon("viewmag.png"), i18n("Lookup in directory"), this, SLOT(lookupInDirectory()),HotKey::shortCutFromFile("kadu_searchuser"));
	if (!user.uin) {
		pm->setItemEnabled(deletehistoryitem, false);
		pm->setItemEnabled(historyitem, false);
		pm->setItemEnabled(searchuser, false);
//		pm->setItemEnabled(KADU_CMD_SEND_MESSAGE, false);
		pm->setItemEnabled(openchatitem, false);
		}
	pm->insertSeparator();
	pm->insertItem(i18n("About..."), this, SLOT(about()));

	pm->exec(QCursor::pos());    	
}

/* if something's pending, open it, if not, open new message */
void Kadu::sendMessage(QListBoxItem *item) {
	QString tmp;
	int i, j, k = -1, l;
	bool stop = false;
/*	rMessage *rmsg;
	Message *msg; */
	UinsList uins;
	PendingMsgs::Element elem;
	uin_t uin;
	QString toadd;
	bool msgsFromHist = false;

	uin = userlist.byAltNick(item->text()).uin;

	if (!uin) {
		sendSmsToUser();
		return;
		}

	for (i = 0; i < pending.count(); i++) {
		elem = pending[i];
		if ((!uins.count() && elem.uins.contains(uin)) || (uins.count() && elem.uins.equals(uins)))
			if ((elem.msgclass & GG_CLASS_CHAT) == GG_CLASS_CHAT
				|| (elem.msgclass & GG_CLASS_MSG) == GG_CLASS_MSG
				|| !elem.msgclass) {
				if (!uins.count())
					uins = elem.uins;
				for (j = 0; j < elem.uins.count(); j++)
					if (!userlist.containsUin(elem.uins[j])) {
						tmp = QString::number(pending[i].uins[j]);
						if (trayicon)
							userlist.addUser("", "", tmp, tmp, "", tmp, GG_STATUS_NOT_AVAIL,
								0, false, false, true, "", "", "", true);
						else
							addUser("", "", tmp, tmp, "", tmp, GG_STATUS_NOT_AVAIL,
								0, "", "", "", true);
						}
				
				l = chats.count();
				k = openChat(elem.uins);
				QValueList<UinsList>::iterator it = wasFirstMsgs.begin();
				while (it != wasFirstMsgs.end() && !elem.uins.equals(*it))
					it++;
				if (it != wasFirstMsgs.end())
					wasFirstMsgs.remove(*it);
				if (!msgsFromHist) {
					if (l < chats.count())
						chats[k].ptr->writeMessagesFromHistory(elem.uins, elem.time);
					msgsFromHist = true;
					}
				chats[k].ptr->formatMessage(false,
					userlist.byUin(elem.uins[0]).altnick, elem.msg,
					timestamp(elem.time), toadd);	    
				pending.deleteMsg(i);
				kdebug("Kadu::sendMessage(): k=%d\n", k);
				i--;
				stop = true;
				}
	    	/*	else {
				if (!stop) {
  		    			rmsg = new rMessage(item->text(),
						elem.msgclass, elem.uins, elem.msg);
					deletePendingMessage(i);
					UserBox::all_refresh();
					rmsg->init();
					rmsg->show();
					}
				else
					chats[k].ptr->scrollMessages(toadd);
				return;
				}*/
		}

	if (stop) {
		chats[k].ptr->scrollMessages(toadd);
		UserBox::all_refresh();
		return;
		}

	uins.clear();
	for (i = 0; i < userbox->count(); i++)
		if (userbox->isSelected(i))
			uins.append(userlist.byAltNick(userbox->text(i)).uin);
	if (!uins.count())
		uins.append(userlist.byAltNick(item->text()).uin);

/*	if (uins.count() > 1 || (userlist.byUin(uins[0]).status != GG_STATUS_NOT_AVAIL
		&& userlist.byUin(uins[0]).status != GG_STATUS_NOT_AVAIL_DESCR))
		openChat(uins);
	else {
		msg = new Message(item->text());
		msg->init();
		msg->show();
		}*/
//	zawsze otwieraja sie czaty
	l = chats.count();
	k = openChat(uins);
	if (!msgsFromHist && l < chats.count())
		chats[k].ptr->writeMessagesFromHistory(uins, 0);
}

/* when we want to change the status */
void Kadu::slotHandleState(int command) {
	ChooseDescription *cd;

	switch (command) {
		case 0:
			autohammer = true;
			setStatus(GG_STATUS_AVAIL);
			break;
		case 1:
			cd = new ChooseDescription(1);
			if (cd->exec() == QDialog::Accepted) {
				autohammer = true;
				setStatus(GG_STATUS_AVAIL_DESCR);
				}
			break;
		case 2:
			autohammer = true;
			setStatus(GG_STATUS_BUSY);
			break;
		case 3:
			cd = new ChooseDescription(3);
			if (cd->exec() == QDialog::Accepted) {
				autohammer = true;
				setStatus(GG_STATUS_BUSY_DESCR);
				}
			break;
		case 4:
			autohammer = true;
			setStatus(GG_STATUS_INVISIBLE);
			break;
		case 5:
			cd = new ChooseDescription(5);
			if (cd->exec() == QDialog::Accepted) {
				autohammer = true;
				setStatus(GG_STATUS_INVISIBLE_DESCR);
				}
			break;
		case 6:
			disconnectNetwork();
			AutoConnectionTimer::off();
			autohammer = false;
			setCurrentStatus(GG_STATUS_NOT_AVAIL);
			break;
		case 7:
			cd = new ChooseDescription(7);
			if (cd->exec() == QDialog::Accepted) {
				setStatus(GG_STATUS_NOT_AVAIL_DESCR);
				statusppm->setItemEnabled(7, false);
				disconnectNetwork();
				AutoConnectionTimer::off();
				autohammer = false;
				}
			break;	    
		case 8:
			statusppm->setItemChecked(8, !statusppm->isItemChecked(8));
			dockppm->setItemChecked(8, !dockppm->isItemChecked(8));	    
			config.privatestatus = statusppm->isItemChecked(8);
			if (!statusppm->isItemChecked(6) && !statusppm->isItemChecked(7))
				setStatus(sess->status & (~GG_STATUS_FRIENDS_MASK));
			break;
		}
}

void Kadu::slotShowStatusMenu() {
	QPoint point = statuslabeltxt->mapToGlobal(QPoint(0, 0));
	point.setY(point.y() - statusppm->sizeHint().height());
	statusppm->popup(point);
}

void Kadu::setCurrentStatus(int status) {
	int statusnr;
	int i = 0;

	statusnr = statusGGToStatusNr(status);
	while (i<8) {
		statusppm->setItemChecked(i, false);
		dockppm->setItemChecked(i, false);
		i++;
		}
	statusppm->setItemChecked(statusnr, true);
	dockppm->setItemChecked(statusnr, true);	

	statuslabeltxt->setText(i18n(statustext[statusnr]));
	statusppm->setItemEnabled(7, statusnr != 6);
	dockppm->setItemEnabled(7, statusnr != 6);
	QPixmap *pix = icons->loadIcon(gg_icons[statusnr]);
	statuslabel->setPixmap(*pix);
	setIcon(*pix);
	if (!pending.pendingMsgs() && trayicon)
		trayicon->setType(*pix);
}

void Kadu::setStatus(int status) {
	QHostAddress ip;

	kdebug("Kadu::setStatus(): setting status: %d\n",
		status | (GG_STATUS_FRIENDS_MASK * config.privatestatus));

	bool with_description;
	unsigned char *descr;
    
	with_description = ifStatusWithDescription(status);
	status &= ~GG_STATUS_FRIENDS_MASK;
    	
	i_wanna_be_invisible = false;
	if (status == GG_STATUS_INVISIBLE || status == GG_STATUS_INVISIBLE_DESCR)
		i_wanna_be_invisible = true;
	disconnect_planned = false;		

	if (!userlist_sent) {
		doBlink = true;
		if (!blinktimer) {
			blinktimer = new QTimer;
			QObject::connect(blinktimer, SIGNAL(timeout()), kadu, SLOT(blink()));
			}
		blinktimer->start(1000, TRUE);
		}
    
	if (socket_active) {
		doBlink = false;
		if (with_description) {
			descr = (unsigned char *)strdup(unicode2cp(own_description).data());
			if (status == GG_STATUS_NOT_AVAIL_DESCR)
				gg_change_status_descr(sess, status, (const char *)descr);
			else
				gg_change_status_descr(sess,
					status | (GG_STATUS_FRIENDS_MASK * config.privatestatus), (const char *)descr);
			free(descr);
			}
		else
			gg_change_status(sess, status | (GG_STATUS_FRIENDS_MASK * config.privatestatus));
		if (sess->check & GG_CHECK_WRITE)
			kadusnw->setEnabled(true);
	
		setCurrentStatus(status);

		kdebug("Kadu::setStatus(): actual status: %d\n", sess->status);
		/** AutoConnectionTimer u�ywa loginparams.status jako statusu do nowego po��czenia si�
				po stracie po��czenia, czyli kadu b�dzie pr�bowal sie po��czy� z statusem takim
				samym, co by� ostatnio ustawiony(opr�cz niedost�pny i niedost�pny z opisem).
		**/
		if (status != GG_STATUS_NOT_AVAIL && status != GG_STATUS_NOT_AVAIL_DESCR)
			loginparams.status = status | (GG_STATUS_FRIENDS_MASK * config.privatestatus);
		return;
		}

	if (config.allowdcc)
		prepareDcc();

	if (config.useproxy) {
		char *gg_proxy_username;
		char *gg_proxy_password;
		if (gg_proxy_host)
			delete gg_proxy_host;
		gg_proxy_host = strdup(config.proxyaddr.toString().latin1());
		gg_proxy_port = config.proxyport;
		if (config.proxyuser.length()) {
			gg_proxy_username = (char *)config.proxyuser.latin1();
			gg_proxy_password = (char *)config.proxypassword.latin1();
			}
		else
			gg_proxy_username = gg_proxy_password = NULL;
		gg_proxy_enabled = 1;
		}
	else
		gg_proxy_enabled = 0;

	loginparams.status = status | (GG_STATUS_FRIENDS_MASK * config.privatestatus);
	loginparams.password = (char *)config.password.latin1();
	loginparams.uin = config.uin;
	loginparams.client_version = GG_DEFAULT_CLIENT_VERSION;
	loginparams.has_audio = 1;
//	if (with_description)
//		descr = (unsigned char *)strdup(unicode2cp(own_description).data());
//	else
//		descr = NULL;
	
	if (config.allowdcc && config.extip.ip4Addr() && config.extport > 1023) {
		loginparams.external_addr = htonl(config.extip.ip4Addr());
		loginparams.external_port = config.extport;
		}
	else {
		loginparams.external_addr = 0;
		loginparams.external_port = 0;
		}	
	if (config.servers.count() && !config.default_servers && config.servers[server_nr].ip4Addr()) {
		loginparams.server_addr = htonl(config.servers[server_nr].ip4Addr());
		loginparams.server_port = config.default_port;
		server_nr++;
		if (server_nr >= config.servers.count())
			server_nr = 0;
		}
	else {
		if (server_nr) {
			loginparams.server_addr = htonl(gg_servers[server_nr - 1].ip4Addr());
			loginparams.server_port = config.default_port;
			}
		else {
			loginparams.server_addr = 0;
			loginparams.server_port = 0;
			}
		server_nr++;
		if (server_nr > 7)
			server_nr = 0;
		}
	loginparams.tls = config.tls;
	loginparams.protocol_version = 0x21;
	loginparams.client_version = strdup("6, 0, 0, 135");
	if (config.tls) {
		loginparams.server_port = 0;
		if (config.default_servers)
			loginparams.server_addr = 0;
		loginparams.server_port = 443;
		}
	else
		loginparams.server_port = config.default_port;
	sess = gg_login(&loginparams);
	free(loginparams.client_version);
//	if (descr)
//		free(descr);

	AutoConnectionTimer::off();

	if (sess) {
		socket_active = true;
		last_ping = time(NULL);

		kadusnw = new QSocketNotifier(sess->fd, QSocketNotifier::Write, this);
		QObject::connect(kadusnw, SIGNAL(activated(int)), kadu, SLOT(dataSent()));

		kadusnr = new QSocketNotifier(sess->fd, QSocketNotifier::Read, this);
		QObject::connect(kadusnr, SIGNAL(activated(int)), kadu, SLOT(dataReceived()));
		}
	else {
		disconnectNetwork();
		QMessageBox::warning(kadu, i18n("Connection problem"),
			i18n("Couldn't connect.\nCheck your internet connection."));
		}
}

/* patrz plik events.cpp
void Kadu::checkConnection(void) {
	// Since it doesnt work anymore...
	readevent->start(10000, TRUE);
	return;	
}
*/

void Kadu::dataReceived(void) {
	kdebug("Kadu::dataReceived()\n");
	if (sess->check && GG_CHECK_READ)
		event_manager.eventHandler(sess);
}

void Kadu::dataSent(void) {
	kdebug("Kadu::dataSent()\n");
	kadusnw->setEnabled(false);
	if (sess->check & GG_CHECK_WRITE)
		event_manager.eventHandler(sess);
}

void Kadu::pingNetwork(void) {
	kdebug("Kadu::pingNetwork()\n");
	gg_ping(sess);
	pingtimer->start(60000, TRUE);
}

void Kadu::disconnectNetwork() {
	int i;
	doBlink = false;

	kdebug("Kadu::disconnectNetwork(): calling offline routines\n");

	if (config.autoaway)
		AutoAwayTimer::off();
	if (pingtimer) {
		pingtimer->stop();
		delete pingtimer;
		pingtimer = NULL;
		}
	if (blinktimer) {
		blinktimer->stop();
		delete blinktimer;
		blinktimer = NULL;
		}
/* patrz events.cpp
	if (readevent) {
		readevent->stop();
		delete readevent;
		readevent = NULL;
		}
*/	
	if (kadusnw) {
		kadusnw->setEnabled(false);
		delete kadusnw;
		kadusnw = NULL;
		}
	if (kadusnr) {
		kadusnr->setEnabled(false);
		delete kadusnr;
		kadusnr = NULL;
		}
	if (dccsnr) {
		delete dccsnr;
		dccsnr = NULL;
		}
	if (dccsnw) {
		delete dccsnw;
		dccsnw = NULL;
		}
	if (dccsock) {
		gg_dcc_free(dccsock);
		dccsock = NULL;
		gg_dcc_ip = 0;
		gg_dcc_port = 0;
		}

	disconnect_planned = true;
	if (sess) {
		gg_logoff(sess);
		gg_free_session(sess);
		sess = NULL;
		}
	userlist_sent = false;

	i = 0;
	while (i < userlist.count()) {
		userlist[i].status = GG_STATUS_NOT_AVAIL;
		userlist[i].description = "";
		i++;
		}

	i = 0;
	while (i < chats.count()) {
		chats[i].ptr->setTitle();
		i++;
		}

	UserBox::all_refresh();
	
	socket_active = false;
	
	setCurrentStatus(GG_STATUS_NOT_AVAIL);
}


void Kadu::dccFinished(dccSocketClass *dcc) {
	kdebug("dccFinished\n");
	delete dcc;
}

bool Kadu::event(QEvent *e) {
	QCustomEvent *ce;
	dccSocketClass *dcc;
	dccSocketClass **data;

	if (e->type() == QEvent::User) {
		kdebug("Kadu::event()\n");
		ce = (QCustomEvent *)e;
		data = (dccSocketClass **)ce->data();
		dcc = *data;
		switch (dcc->state) {
			case DCC_SOCKET_TRANSFER_FINISHED:
				QMessageBox::information(0, i18n("Information"),
					i18n("File has been transferred sucessfully."),
					i18n("&OK"));
				break;
			case DCC_SOCKET_TRANSFER_DISCARDED:
//				QMessageBox::information(0, i18n("Information"), 
//					i18n("File transfer has been discarded."), i18n("&OK"));
				break;
			case DCC_SOCKET_TRANSFER_ERROR:
			case DCC_SOCKET_CONNECTION_BROKEN:
				QMessageBox::information(0, i18n("Error"),
					i18n("File transfer error!"),
					i18n("&OK"));
				break;
			case DCC_SOCKET_COULDNT_OPEN_FILE:
				QMessageBox::information(0, i18n("Error"),
					i18n("Couldn't open file!"),
					i18n("&OK"));
				break;
			}
		delete data;
		delete dcc;
		ce->setData(NULL);
		}
	return QWidget::event(e);
}

void Kadu::dccReceived(void) {
	kdebug("Kadu::dccReceived()\n");
	watchDcc();
}

void Kadu::dccSent(void) {
	kdebug("Kadu::dccSent()\n");
	dccsnw->setEnabled(false);
	if (dccsock->check & GG_CHECK_WRITE)
		watchDcc();
}

void Kadu::watchDcc(void) {
	kdebug("Kadu::watchDcc(): data on socket\n");			
	if (!(dcc_e = gg_dcc_watch_fd(dccsock))) {
		kdebug("Kadu::watchDcc(): Connection broken unexpectedly!\n");
		config.allowdcc = false;
		delete dccsnr;
		dccsnr = NULL;
		delete dccsnw;
		dccsnw = NULL;
		return;
		}

	switch (dcc_e->type) {
		case GG_EVENT_NONE:
			break;
		case GG_EVENT_DCC_ERROR:
			kdebug("Kadu::watchDcc(): GG_EVENT_DCC_ERROR\n");
			break;
		case GG_EVENT_DCC_NEW:
			if (dccSocketClass::count < 8) {
				dccSocketClass *dcc;    
				dcc = new dccSocketClass(dcc_e->event.dcc_new);
				connect(dcc, SIGNAL(dccFinished(dccSocketClass *)), this, SLOT(dccFinished(dccSocketClass *)));
				dcc->initializeNotifiers();
				kdebug("Kadu::watchDcc(): GG_EVENT_DCC_NEW: spawning object\n");
				}
			else {
				if (dcc_e->event.dcc_new->file_fd > 0)
					close(dcc_e->event.dcc_new->file_fd);
				gg_dcc_free(dcc_e->event.dcc_new);
				}
			break;
		default:
			break;
		}

	if (dccsock->check == GG_CHECK_WRITE)
		dccsnw->setEnabled(true);

	gg_free_event(dcc_e);
}

bool Kadu::close(bool quit) {
	if (!quit && trayicon) {
		kdebug("Kadu::close(): Kadu hide\n");
		hide();
		return false;
		}
	else {
		config.splitsize.setWidth(userbox->size().height());
		config.splitsize.setHeight(descrtb->size().height());
		config.geometry.setX(pos().x());
		config.geometry.setY(pos().y());
		config.geometry.setWidth(size().width());
		config.geometry.setHeight(size().height());
		saveKaduConfig();
		pending.writeToFile();
		writeIgnored();
		if (config.disconnectwithdesc && getActualStatus() != GG_STATUS_NOT_AVAIL) {
			kdebug("Kadu::close(): Set status NOT_AVAIL_DESCR with disconnect description(%s)\n",(const char *)config.disconnectdesc.local8Bit());
			own_description = config.disconnectdesc;
			setStatus(GG_STATUS_NOT_AVAIL_DESCR);
		}
		disconnectNetwork();
		kdebug("Kadu::close(): Saved config, disconnect and ignored\n");
		QWidget::close(true);
//		a->quit();
		kdebug("Kadu::close(): Graceful shutdown...\n");
		return true;
	}
}

void Kadu::quitApplication() {
	kdebug("Kadu::quitApplication()\n");
	close(true);
}

Kadu::~Kadu(void) {
	kdebug("Kadu::~Kadu()\n");
}

void Kadu::createMenu() {

	mmb = new QMenuBar(this, "mmb");

	QPopupMenu *ppm = new QPopupMenu(this, "ppm");
	ppm->insertItem(i18n("Manage &ignored"), this, SLOT(manageIgnored()));
	ppm->insertItem(loadIcon("configure.png"), i18n("&Configuration"), this, SLOT(configure()),HotKey::shortCutFromFile("kadu_configure"));
	ppm->insertItem(loadIcon("reload.png"), i18n("Resend &userlist"), this, SLOT(sendUserlist1()));
	if (mute) {
		muteitem= ppm->insertItem(loadIcon("mute.png"), i18n("Unmute sounds"), this, SLOT(muteUnmuteSounds()));
		}
	else {
		muteitem= ppm->insertItem(loadIcon("unmute.png"), i18n("Mute sounds"), this, SLOT(muteUnmuteSounds()));
		}
	ppm->insertSeparator();

	grpmenu = new QPopupMenu(this);
	grpmenu->insertItem(i18n("All"), 600);
	grpmenu->insertSeparator();

	ppm->insertItem(i18n("Remind &password"), this, SLOT(remindPassword1()));
	ppm->insertItem(i18n("&Change password"), this, SLOT(changePassword1()));
	ppm->insertItem(loadIcon("newuser.png"),i18n("Register &new user"), this, SLOT(registerUser()));
	ppm->insertItem(i18n("Unregister user"), this, SLOT(unregisterUser()));
	ppm->insertItem(i18n("Personal information"), this,SLOT(personalInfo()));
	ppm->insertSeparator();
	QPixmap find;
	find = loadIcon("viewmag.png");
	ppm->insertItem(find, i18n("&Search for users"), this, SLOT(searchInDirectory()));
	ppm->insertItem(i18n("I&mport userlist"), this, SLOT(importUserlist()));
	ppm->insertItem(i18n("E&xport userlist"), this, SLOT(exportUserlist()));
	ppm->insertItem(*icons->loadIcon("online"), i18n("&Add user"), this, SLOT(addUserAction()),HotKey::shortCutFromFile("kadu_adduser"));
	ppm->insertItem(i18n("Send SMS"), this,SLOT(sendSms()));
	ppm->insertSeparator();	
	ppm->insertItem(i18n("H&elp"), this, SLOT(help()));	
	ppm->insertItem(i18n("A&bout..."), this, SLOT(about()));
	ppm->insertSeparator();
	ppm->insertItem(i18n("&Hide Kadu"), this, SLOT(hideKadu()));
	ppm->insertItem(loadIcon("exit.png"), i18n("&Exit Kadu"), this, SLOT(quit()));

	mmb->insertItem(i18n("&Kadu"), ppm);
//	mmb->polish();
}

void Kadu::statusMenuAboutToHide() {
	closestatusppmtime.restart();
}

void Kadu::createStatusPopupMenu() {

	QPixmap *pix;
	QIconSet icon;

	statusppm = new QPopupMenu(this, "statusppm");
	dockppm = new QPopupMenu(this, "dockppm");

	for (int i=0; i<8; i++) {
		pix = icons->loadIcon(gg_icons[i]);
		icon = QIconSet(*pix);
		statusppm->insertItem(icon, i18n((const char *)statustext[i]), i);
		dockppm->insertItem(icon, i18n((const char *)statustext[i]), i);
		}

	statusppm->insertSeparator();
	dockppm->insertSeparator();
	statusppm->insertItem(i18n("Private"), 8);
	statusppm->setItemChecked(8, config.privatestatus);
	dockppm->insertItem(i18n("Private"), 8);
	dockppm->setItemChecked(8, config.privatestatus);

	statusppm->setCheckable(true);
	dockppm->setCheckable(true);
	statusppm->setItemChecked(6, true);
	dockppm->setItemChecked(6, true);
	
	statusppm->setItemEnabled(7, false);
	dockppm->setItemEnabled(7, false);

	connect(statusppm, SIGNAL(activated(int)), this, SLOT(slotHandleState(int)));
}

void MyLabel::mousePressEvent (QMouseEvent * e) {
	if (e->button() == Qt::LeftButton && closestatusppmtime.elapsed() >= 100)
		kadu->slotShowStatusMenu();
}

void Kadu::showdesc(bool show) {
	if (show)
		descrtb->show();
	else
		descrtb->hide();
}

void Kadu::infopanelUpdate(uin_t uin) {
	if (!config.showdesc)
		return;
	kdebug("Kadu::infopanelUpdate(%d)\n", uin);
	if (userbox->currentItem() != -1 && uin == userlist.byAltNick(userbox->currentText()).uin)
		descrtb->setText(parse(config.panelsyntax,userlist.byUin(uin)));
}
