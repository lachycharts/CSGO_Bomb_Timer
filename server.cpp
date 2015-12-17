#include "server.h"

server::server(QObject *parent):QTcpServer(parent)
{
    //Connect slots to know when port is ready to be read.
    server_socket = new QTcpSocket(this);
    connect(server_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpError(QAbstractSocket::SocketError)));
    connect(server_socket, SIGNAL(readyRead()),this, SLOT(getData()));
}

server::~server()
{
    disconnect(server_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpError(QAbstractSocket::SocketError)));
    disconnect(server_socket, SIGNAL(readyRead()),this, SLOT(getData()));
    disconnect(okayToGoTimer, SIGNAL(timeout()), this, SLOT(setOkayToGetData()));
    this->close();
}

void server::startListen()
{
    //Check if listening to the port.
    if(!this->listen(QHostAddress::LocalHost, port))
        emit listeningError(server_socket->errorString());
    else
    {
        //Let mainwindow know we are started.
        emit listeningStarted();

        //Allow data to be collected.
        okayToGo = 1;

        //Start timer.
        okayToGoTimer = new QTimer(this);
        okayToGoTimer->setInterval(200);
        connect(okayToGoTimer, SIGNAL(timeout()), this, SLOT(setOkayToGetData()));
        okayToGoTimer->start();
    }
}

void server::incomingConnection(int descriptor)
{
    //Need this to grab the data.
    server_socket->setSocketDescriptor(descriptor);
}

void server::getData()
{
    //This is absolute bullshit, soz for this.
    QByteArray rawData = server_socket->read(server_socket->bytesAvailable());
    QString rawDataString(rawData);
    QStringList rawDataStringList = rawDataString.split("\n");
    QStringList jsonOnlyList = rawDataStringList.mid(10, rawDataStringList.size() -1);
    QString jsonOnly = jsonOnlyList.join("\n");

    //Sometimes the json doesn't have the data we want in it.
    if(jsonOnly.contains("round") && okayToGo == 1)
    {
        emit sendCSGOOutput(jsonOnly);
        okayToGo = 0;
    }

    //Send a response so that csgo knows we are listening -> no timeouts/errors.
    server_socket->write(":)");
    server_socket->flush();
    server_socket->close();
}

void server::tcpError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    this->close();
    emit listeningError(server_socket->errorString());
}

void server::setPort(int port_no)
{
    port = port_no;
}

void server::setOkayToGetData()
{
    okayToGo = 1;
}
