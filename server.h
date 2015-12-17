#ifndef SERVER
#define SERVER

#include <QtNetwork>
#include <QMessageBox>

class server : public QTcpServer
{
    Q_OBJECT

public:
    explicit server(QObject *parent = 0);
    ~server();
    void connectTo();
    int port;
    QTcpSocket *server_socket;
    QTcpServer *serva;
    QTimer *okayToGoTimer;
    bool okayToGo = 0;
    void setPort(int port_no);

public slots:
      void getData();
      void tcpError(QAbstractSocket::SocketError error);
      void startListen();
      void setOkayToGetData();
signals:
      void sendCSGOOutput(QString);
      void listeningStarted();
      void listeningError(QString);

protected:
      void incomingConnection(int descriptor);
};

#endif // SERVER

