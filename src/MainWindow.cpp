#include "MainWindow.h"

#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "AppConfig.h"
#include "DatabaseLogger.h"
#include "NetworkManager.h"

MainWindow::MainWindow(const AppConfig& config, NetworkManager* networkManager, DatabaseLogger* dbLogger,
                       QWidget* parent)
    : QMainWindow(parent), ui(nullptr), m_networkManager(networkManager), m_dbLogger(dbLogger), m_appConfig(config) {
    setupUiElements();

    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessageClicked);
    connect(m_networkManager, &NetworkManager::messageReceived, this, &MainWindow::handleReceivedMessage);
    connect(m_networkManager, &NetworkManager::networkError, this, &MainWindow::handleNetworkError);
    connect(m_targetIpInput, &QLineEdit::editingFinished, this, &MainWindow::onTargetConfigChanged);
    connect(m_targetPortInput, &QLineEdit::editingFinished, this, &MainWindow::onTargetConfigChanged);

    m_networkManager->setTarget(m_appConfig.targetIp, m_appConfig.targetPort);
    m_targetIpInput->setText(m_appConfig.targetIp.toString());
    m_targetPortInput->setText(QString::number(m_appConfig.targetPort));

    displayLocalNetworkInfo();

    setWindowTitle(QString("UDP Chat - Listening on: %1").arg(m_networkManager->localPort()));
}

MainWindow::~MainWindow() {}

void MainWindow::setupUiElements() {
    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    m_localInfoLabel = new QLabel("Initializing...", this);
    mainLayout->addWidget(m_localInfoLabel);

    QGroupBox*   targetGroup  = new QGroupBox("Target Configuration", this);
    QFormLayout* targetLayout = new QFormLayout();
    m_targetIpInput           = new QLineEdit(this);
    m_targetPortInput         = new QLineEdit(this);
    m_targetPortInput->setValidator(new QIntValidator(1, 65535, this));
    targetLayout->addRow("Target IP:", m_targetIpInput);
    targetLayout->addRow("Target Port:", m_targetPortInput);
    targetGroup->setLayout(targetLayout);
    mainLayout->addWidget(targetGroup);

    m_chatDisplay = new QTextEdit(this);
    m_chatDisplay->setReadOnly(true);
    mainLayout->addWidget(m_chatDisplay, 1);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_messageInput           = new QLineEdit(this);
    m_sendButton             = new QPushButton("Send", this);
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    mainLayout->addLayout(inputLayout);

    m_statusLabel = new QLabel("Status: Ready", this);
    mainLayout->addWidget(m_statusLabel);

    m_messageInput->setFocus();
}

void MainWindow::displayLocalNetworkInfo() {
    quint16 port = m_networkManager->localPort();
    if (port > 0) {
        m_localInfoLabel->setText(
            QString("Listening on: %1:%2").arg(m_networkManager->localAddress().toString()).arg(port));
    } else {
        m_localInfoLabel->setText("Error: Not listening on any port.");
    }
}

void MainWindow::onSendMessageClicked() {
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    QHostAddress currentTargetIp(m_targetIpInput->text());
    quint16      currentTargetPort = m_targetPortInput->text().toUShort();

    if (currentTargetIp.isNull() || currentTargetPort == 0) {
        QMessageBox::warning(this, "Error", "Target IP or Port is invalid. Please check configuration.");
        return;
    }

    m_networkManager->setTarget(currentTargetIp, currentTargetPort);

    logAndDisplayMessage(message, m_networkManager->localAddress(), m_networkManager->localPort(), currentTargetIp,
                         currentTargetPort, true);

    m_networkManager->sendMessage(message);
    m_messageInput->clear();
    m_messageInput->setFocus();
}

void MainWindow::handleReceivedMessage(const QString& message, const QHostAddress& senderIp, quint16 senderPort) {
    logAndDisplayMessage(message, senderIp, senderPort, m_networkManager->localAddress(), m_networkManager->localPort(),
                         false);
}

void MainWindow::logAndDisplayMessage(const QString& message, const QHostAddress& sourceIp, quint16 sourcePort,
                                      const QHostAddress& destIp, quint16 destPort, bool isOutgoing) {
    QDateTime timestamp       = QDateTime::currentDateTime();
    QString   directionPrefix = isOutgoing ? "Me" : QString("[%1:%2]").arg(sourceIp.toString()).arg(sourcePort);
    QString   fullMessage =
        QString("[%1] %2: %3").arg(timestamp.toString("yyyy-MM-dd hh:mm:ss")).arg(directionPrefix).arg(message);

    m_chatDisplay->append(fullMessage);

    if (m_dbLogger) {
        m_dbLogger->logMessage(message, timestamp, isOutgoing, sourceIp, sourcePort, destIp, destPort);
    }
}

void MainWindow::handleNetworkError(const QString& errorString) {
    QMessageBox::critical(this, "Network Error", errorString);
    m_statusLabel->setText("Error: " + errorString);
}

void MainWindow::onTargetConfigChanged() {
    QHostAddress newTargetIp(m_targetIpInput->text());
    bool         portOk;
    quint16      newTargetPort = m_targetPortInput->text().toUShort(&portOk);

    if (!newTargetIp.isNull() && portOk && newTargetPort > 0) {
        m_networkManager->setTarget(newTargetIp, newTargetPort);
        m_statusLabel->setText(QString("Target updated to %1:%2").arg(newTargetIp.toString()).arg(newTargetPort));
    } else {
        m_statusLabel->setText(QString("Invalid target IP/Port in input fields."));
    }
}