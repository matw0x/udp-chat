#ifndef DATABASELOGGER_H
#define DATABASELOGGER_H

#include <QDateTime>
#include <QHostAddress>
#include <QObject>
#include <QSqlDatabase>
#include <QString>

class AppConfig;

class DatabaseLogger : public QObject {
    Q_OBJECT

   public:
    explicit DatabaseLogger(const AppConfig& config, QObject* parent = nullptr);
    ~DatabaseLogger();

    bool initialize();
    bool logMessage(const QString& message, const QDateTime& timestamp, bool isOutgoing, const QHostAddress& sourceIp,
                    quint16 sourcePort, const QHostAddress& destIp, quint16 destPort);

   private:
    bool openDatabase();
    bool createTableIfNotExists();

    QSqlDatabase     m_db;
    const AppConfig& m_config;
    QString          m_connectionName;
};

#endif  // DATABASELOGGER_H