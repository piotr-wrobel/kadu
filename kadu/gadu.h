#ifndef GADU_H
#define GADU_H

#include <qhostaddress.h>
#include <qobject.h>
#include <qstring.h>
#include <qtimer.h>
#include <qvaluelist.h>

#include "libgadu.h"

class UinsList;

class QSocketNotifier;

extern struct gg_session* sess;
extern struct gg_login_params loginparams;

extern QSocketNotifier* kadusnr;
extern QSocketNotifier* kadusnw;

extern bool userlist_sent;
extern bool socket_active;
extern int last_read_event;
extern unsigned int server_nr;
extern bool timeout_connected;
extern QTimer* pingtimer;
extern QValueList<QHostAddress> config_servers;
extern bool i_wanna_be_invisible;

struct SearchResult
{
	QString uin;
	QString first;
	QString nick;
	QString born;
	QString city;
	int status;

	SearchResult();
	SearchResult(const SearchResult&);
	void setData(const char *uin, const char *first, const char *nick, const char *born, const char *city, const char *status);
};

typedef QValueList<SearchResult> SearchResults;

struct SearchRecord
{
	int seq;
	int fromUin;
	QString uin;
	QString firstName;
	QString lastName;
	QString nickName;
	QString city;
	QString birthYearFrom;
	QString birthYearTo;
	int gender;
	bool active;

	SearchRecord();
	virtual ~SearchRecord();

	void reqUin(const QString& uin);
	void reqFirstName(const QString& firstName);
	void reqLastName(const QString& lastName);
	void reqNickName(const QString& nickName);
	void reqCity(const QString& city);
	void reqBirthYear(const QString& birthYearFrom, const QString& birthYearTo);
	void reqGender(bool female);
	void reqActive();

	void clearData();

};

class SocketNotifiers : public QObject
{
	Q_OBJECT

	private:
		struct gg_http *h;
		QSocketNotifier *snr;
		QSocketNotifier *snw;

	private slots:
		void dataReceived();
		void dataSent();

	public:
		SocketNotifiers();
		~SocketNotifiers();

		void setGGHttp(struct gg_http *);
		struct gg_http *getGGHttp();

		void createSocketNotifiers(struct gg_http *);
		void deleteSocketNotifiers();
		void recreateSocketNotifiers();

		void checkWrite();
		
	signals:
		void socketEvent();

};

typedef uin_t UinType;

class GaduProtocol : public QObject
{
	Q_OBJECT

	private:
		SocketNotifiers *registerSN;
		SocketNotifiers *unregisterSN;
		SocketNotifiers *remindSN;
		SocketNotifiers *changePasswordSN;

	private slots:
		void registerSocketEvent();
		void unregisterSocketEvent();
		void remindSocketEvent();
		void changePasswordSocketEvent();

	public:	
		static void initModule();
		GaduProtocol();
		virtual ~GaduProtocol();
		/**
			Wysyla wiadomosc. bez formatowania tekstu.
			Jesli adresatow jest wiecej niz
			jeden wysylana jest wiadomosc konferencyjna.
			Zwracany jest numer sekwencyjny wiadomosci, jesli
			przypadkiem mysli chcieli sledzic jej potwierdzenie.
		**/
		int sendMessage(const UinsList& uins,const char* msg);
		/**
			Wysyla wiadomosc z formatowaniem tekstu.
			Jesli adresatow jest wiecej niz
			jeden wysylana jest wiadomosc konferencyjna.
			Zwracany jest numer sekwencyjny wiadomosci, jesli
			przypadkiem mysli chcieli sledzic jej potwierdzenie.
		**/
		int sendMessageRichText(const UinsList& uins,const char* msg,unsigned char* myLastFormats,int myLastFormatsLength);
		/**
			Wysy�a pro�b� o przys�anie obrazka.
		**/
		bool sendImageRequest(UinType uin,int size,uint32_t crc32);
		bool sendImage(UinType uin,const QString& file_name,uint32_t size,char* data);

		/**
		  	Szuka ludzi w katalogu publicznym
		**/
		void searchInPubdir(SearchRecord& searchRecord);
		void searchNextInPubdir(SearchRecord& searchRecord);

		/**
		 	Rejestruje nowego u�ytkownika
		**/
		bool doRegister(QString& mail, QString& password, QString& token_id, QString& token_val);

		/**
		 	Wyrejestrowuje u�ytkownika
		**/
		bool doUnregister(UinType uin, QString& password, QString& token_id, QString& token_val);

		/**
		  	Przypomina has�o
		**/
		bool doRemind(UinType uin, QString& token_id, QString& token_val);

		/**
		  	Zmienia has�o
		**/
		bool doChangePassword(UinType uin, QString& mail, QString& password, QString& new_password, QString& token_id, QString& token_val);
	
	private slots:
		void newResults(gg_pubdir50_t res);

	public slots:
		void sendUserList();

	signals:
		void newSearchResults(SearchResults& searchResults, int seq, int lastUin);
		void registered(bool ok, UinType uin);
		void unregistered(bool ok);
		void reminded(bool ok);
		void passwordChanged(bool ok);
};

extern GaduProtocol* gadu;

#endif

