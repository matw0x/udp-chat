#include "NetworkManager.h"

#include <QDebug>
#include <QNetworkDatagram>

NetworkManager::NetworkManager(QObject* parent)
    : QObject(parent), m_udpSocket(nullptr), m_targetPort(0), m_listenPort(0) {}

bool NetworkManager::initialize(quint16 listenPort) {
    m_listenPort = listenPort;
    m_udpSocket  = new QUdpSocket(this);
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_udpSocket, &QUdpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError);
        emit networkError(m_udpSocket->errorString());
    });

    if (!m_udpSocket->bind(QHostAddress::AnyIPv4, m_listenPort)) {
        emit networkError(QString("Failed to bind to port %1: %2").arg(m_listenPort).arg(m_udpSocket->errorString()));
        return false;
    }

    return qInfo() << "Listening on UDP port:" << m_udpSocket->localPort(), true;
}

void NetworkManager::setTarget(const QHostAddress& address, quint16 port) {
    m_targetIp   = address;
    m_targetPort = port;
    qInfo() << "Target set to" << address.toString() << ":" << port;
}

void NetworkManager::sendMessage(const QString& message) {
    if (!m_udpSocket) {
        emit networkError("Socket not initialized.");
        return;
    }

    if (m_targetIp.isNull() || m_targetPort == 0) {
        emit networkError("Target IP or port not set.");
        return;
    }

    QByteArray datagram  = message.toUtf8();
    qint64     bytesSent = m_udpSocket->writeDatagram(datagram, m_targetIp, m_targetPort);

    if (bytesSent == -1) {
        emit networkError(QString("Failed to send message: %1").arg(m_udpSocket->errorString()));
    } else {
        qInfo() << "Message sent to" << m_targetIp.toString() << ":" << m_targetPort << "-" << bytesSent << "bytes";
    }
}

QHostAddress NetworkManager::localAddress() const { return m_udpSocket ? m_udpSocket->localAddress() : QHostAddress(); }

quint16 NetworkManager::localPort() const { return m_udpSocket ? m_udpSocket->localPort() : 0; }

void NetworkManager::onReadyRead() {
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        QString          message  = QString::fromUtf8(datagram.data());
        emit             messageReceived(message, datagram.senderAddress(), datagram.senderPort());
    }
}