#include "DatabaseLogger.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

#include "AppConfig.h"

DatabaseLogger::DatabaseLogger(const AppConfig& config, QObject* parent) : QObject(parent), m_config(config) {
    m_connectionName = QString("DBLoggerConnection_%1").arg(QUuid::createUuid().toString());
}

DatabaseLogger::~DatabaseLogger() {
    if (m_db.isOpen()) {
        m_db.close();
    }

    QSqlDatabase::removeDatabase(m_connectionName);
}

bool DatabaseLogger::initialize() {
    if (!openDatabase()) {
        return false;
    }

    if (!createTableIfNotExists()) {
        return false;
    }

    return true;
}

bool DatabaseLogger::openDatabase() {
    m_db = QSqlDatabase::addDatabase(m_config.dbDriver, m_connectionName);
    m_db.setDatabaseName(m_config.dbName);

    if (m_config.dbDriver != "QSQLITE") {
        if (!m_config.dbHostName.isEmpty()) m_db.setHostName(m_config.dbHostName);
        if (m_config.dbPort > 0) m_db.setPort(m_config.dbPort);
        if (!m_config.dbUserName.isEmpty()) m_db.setUserName(m_config.dbUserName);
        if (!m_config.dbPassword.isEmpty()) m_db.setPassword(m_config.dbPassword);
    }

    if (!m_db.open()) {
        return qCritical() << "Failed to connect to database:" << m_db.lastError().text(), false;
    }

    return qInfo() << "Database connection established:" << m_config.dbName, true;
}

bool DatabaseLogger::createTableIfNotExists() {
    QSqlQuery query(m_db);
    QString   createTableSql = R"(
        CREATE TABLE IF NOT EXISTS chat_log (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            direction TEXT NOT NULL,
            source_ip TEXT,
            source_port INTEGER,
            dest_ip TEXT,
            dest_port INTEGER,
            message TEXT NOT NULL
        );
    )";

    if (m_config.dbDriver == "QPSQL") {
        createTableSql = R"(
            CREATE TABLE IF NOT EXISTS chat_log (
                id SERIAL PRIMARY KEY,
                timestamp TIMESTAMPTZ NOT NULL,
                direction VARCHAR(10) NOT NULL,
                source_ip VARCHAR(45),
                source_port INTEGER,
                dest_ip VARCHAR(45),
                dest_port INTEGER,
                message TEXT NOT NULL
            );
        )";
    }

    if (!query.exec(createTableSql)) {
        return qCritical() << "Failed to create table 'chat_log':" << query.lastError().text(), false;
    }

    return qInfo() << "Table 'chat_log' is ready.", true;
}

bool DatabaseLogger::logMessage(const QString& message, const QDateTime& timestamp, bool isOutgoing,
                                const QHostAddress& sourceIp, quint16 sourcePort, const QHostAddress& destIp,
                                quint16 destPort) {
    if (!m_db.isOpen()) {
        return qWarning() << "Database is not open. Cannot log message.", false;
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO chat_log (timestamp, direction, source_ip, source_port, dest_ip, dest_port, message)
        VALUES (:timestamp, :direction, :source_ip, :source_port, :dest_ip, :dest_port, :message)
    )");

    query.bindValue(":timestamp", timestamp.toString(Qt::ISODate));
    query.bindValue(":direction", isOutgoing ? "OUTGOING" : "INCOMING");
    query.bindValue(":source_ip", sourceIp.isNull() ? QVariant(QVariant::String) : sourceIp.toString());
    query.bindValue(":source_port", sourceIp.isNull() ? QVariant(QVariant::Int) : sourcePort);
    query.bindValue(":dest_ip", destIp.isNull() ? QVariant(QVariant::String) : destIp.toString());
    query.bindValue(":dest_port", destIp.isNull() ? QVariant(QVariant::Int) : destPort);
    query.bindValue(":message", message);

    if (!query.exec()) {
        return qWarning() << "Failed to log message:" << query.lastError().text(), false;
    }

    return true;
}