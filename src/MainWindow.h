#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHostAddress>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}
class NetworkManager;
class DatabaseLogger;
class AppConfig;

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(const AppConfig& config, NetworkManager* networkManager, DatabaseLogger* dbLogger,
                        QWidget* parent = nullptr);
    ~MainWindow();

   private slots:
    void onSendMessageClicked();
    void displayLocalNetworkInfo();
    void handleReceivedMessage(const QString& message, const QHostAddress& senderIp, quint16 senderPort);
    void handleNetworkError(const QString& errorString);
    void onTargetConfigChanged();

   private:
    void setupUiElements();
    void logAndDisplayMessage(const QString& message, const QHostAddress& sourceIp, quint16 sourcePort,
                              const QHostAddress& destIp, quint16 destPort, bool isOutgoing);

    Ui::MainWindow*  ui;
    NetworkManager*  m_networkManager;
    DatabaseLogger*  m_dbLogger;
    const AppConfig& m_appConfig;

    class QTextEdit*   m_chatDisplay;
    class QLineEdit*   m_messageInput;
    class QPushButton* m_sendButton;
    class QLabel*      m_localInfoLabel;
    class QLineEdit*   m_targetIpInput;
    class QLineEdit*   m_targetPortInput;
    class QLabel*      m_statusLabel;
};

#endif  // MAINWINDOW_H