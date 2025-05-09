#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

class NetworkManager : public QObject {
    Q_OBJECT

   public:
    explicit NetworkManager(QObject* parent = nullptr);

    bool initialize(quint16 listenPort);
    void setTarget(const QHostAddress& address, quint16 port);
    void sendMessage(const QString& message);

    QHostAddress localAddress() const;
    quint16      localPort() const;

   signals:
    void messageReceived(const QString& message, const QHostAddress& senderIp, quint16 senderPort);
    void networkError(const QString& errorString);

   private slots:
    void onReadyRead();

   private:
    QUdpSocket*  m_udpSocket;
    QHostAddress m_targetIp;
    quint16      m_targetPort;
    quint16      m_listenPort;
};

#endif  // NETWORKMANAGER_H