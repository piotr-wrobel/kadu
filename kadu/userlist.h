#ifndef USERLIST_H
#define USERLIST_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <qhostaddress.h>
#include <qdns.h>
#include <qptrlist.h>

#include "libgadu.h"

class DnsHandler : public QObject
{
	Q_OBJECT

	public:
		DnsHandler(uin_t uin);
		~DnsHandler();
		bool isCompleted();

		static int counter;

	private:
		QDns dnsresolver;
		uin_t uin;
		bool completed;

	public slots:
		void resultsReady();
};

typedef QPtrList<DnsHandler> DnsLookups;

class UserList;

struct UserListElement
{
	private:
		QString Group;
		UserList* Parent;
		friend class UserList;
	
	public:
		UserListElement(UserList* parent);
		UserListElement();
		QString group();
		void setGroup(const QString& group);
	
		// te trzeba kiedys trzeba tak uporzadkowac
		// jak Group
		QString first_name;
		QString last_name;
		QString nickname;
		QString altnick;
		QString mobile;
		QString description;
		QString email;
		uin_t uin;
		unsigned int status;
		int image_size;
		bool anonymous;
		QHostAddress ip;
		QString dnsname;
		short port;
		int version;
		bool blocking;
		bool offline_to_user;
		bool notify;
};

/**
	Klasa reprezentująca listę kontaktów
**/
class UserList : public QObject, public QValueList<UserListElement>
{
	Q_OBJECT

	public:
		UserList();
		~UserList();
		UserList::UserList(UserList &source);
		UserListElement& byUin(uin_t uin);
		UserListElement& byNick(const QString& nickname);
		UserListElement& byAltNick(const QString& altnick);
		UserListElement byUinValue(uin_t uin);
		bool containsUin(uin_t uin);
		bool containsAltNick(const QString& altnick);
		void addUser(UserListElement &ule);
		void changeUserInfo(const QString &oldaltnick, UserListElement &ule);
		void changeUserStatus(const uin_t uin, const unsigned int status);
		void removeUser(const QString &altnick);
		bool writeToFile(QString filename = "");
		bool readFromFile();
		UserList &operator=(const UserList& userlist);
		void setDnsName(uin_t uin, const QString &name);
		void addDnsLookup(uin_t uin, const QHostAddress &ip);
		void merge(UserList &userlist);

	protected:
		DnsLookups dnslookups;
		friend class UserListElement;

	signals:
		void modified();
		void statusModified(UserListElement *);
		void dnsNameReady(uin_t);
};

extern UserList userlist;

#endif
