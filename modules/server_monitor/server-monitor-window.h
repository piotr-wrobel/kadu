 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SERVER_MONITOR_WINDOW_H
#define SERVER_MONITOR_WINDOW_H

#include <QtCore/QTimer>
#include <QtGui/QLabel>
#include <QtGui/QScrollArea>
#include <QtGui/QPushButton>


class QHttp;
class QGridLayout;
class QBuffer;


#include "configuration/configuration-aware-object.h"

#include "server-status-widget.h"

class ServerMonitorWindow : public QScrollArea, ConfigurationAwareObject
{
    Q_OBJECT

public:
    ServerMonitorWindow(QWidget *parent = 0);
    ~ServerMonitorWindow();
private:
    QList<ServerStatusWidget*> servers;
    QString serverFileListName;

    QPushButton buttonRefresh;
    QTimer refreshTimer;

    QLabel stats;
    quint32 avalibleServers;
    quint32 unavalibleServers;
    quint32 unknownStatusServers;

    QHttp   *http;
    QBuffer *serverListBuffer;

    QGridLayout *layout;
    QWidget *scrollBarLayout;

    virtual void configurationUpdated();
    void setConfiguration();

    void removeAllServer();

    void cleanLayout();

private slots:
    void downloadedServersList(bool);
    void readServerList();
    void refreshList();
    void updateStats( ServerStatusWidget::ServerState, ServerStatusWidget::ServerState );
};

#endif // SERVER_MONITOR_WINDOW_H
