#include "AppConfig.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <iostream>

AppConfig::AppConfig()
    : dbDriver("QSQLITE"),
      dbHostName(""),
      dbName("chat_log.sqlite"),
      dbUserName(""),
      dbPassword(""),
      dbPort(0),
      listenPort(0),
      targetIp(QHostAddress::LocalHost),
      targetPort(0) {}

bool AppConfig::parseCommandLineArguments(const QStringList& args) {
    QCommandLineParser parser;
    parser.setApplicationDescription("UDP Chat Application with DB Logging");
    parser.addHelpOption();

    QCommandLineOption dbDriverOption("dbdriver", "Database driver (e.g., QSQLITE, QPSQL, QMYSQL). Default: QSQLITE.",
                                      "driver", "QSQLITE");
    QCommandLineOption dbHostOption("dbhost", "Database host name.", "hostname");
    QCommandLineOption dbNameOption("dbname", "Database name (or file path for SQLite). Default: chat_log.sqlite.",
                                    "databasename", "chat_log.sqlite");
    QCommandLineOption dbUserOption("dbuser", "Database user name.", "username");
    QCommandLineOption dbPassOption("dbpass", "Database password.", "password");
    QCommandLineOption dbPortOption("dbport", "Database port.", "port");

    QCommandLineOption listenPortOption("listenport", "Port to listen on for incoming messages.", "port");
    QCommandLineOption targetIpOption("targetip", "Target IP address to send messages to.", "ip", "127.0.0.1");
    QCommandLineOption targetPortOption("targetport", "Target port to send messages to.", "port");

    parser.addOption(dbDriverOption);
    parser.addOption(dbHostOption);
    parser.addOption(dbNameOption);
    parser.addOption(dbUserOption);
    parser.addOption(dbPassOption);
    parser.addOption(dbPortOption);

    parser.addOption(listenPortOption);
    parser.addOption(targetIpOption);
    parser.addOption(targetPortOption);

    parser.process(QCoreApplication::arguments());

    dbDriver   = parser.value(dbDriverOption);
    dbHostName = parser.value(dbHostOption);
    dbName     = parser.value(dbNameOption);
    dbUserName = parser.value(dbUserOption);
    dbPassword = parser.value(dbPassOption);
    if (parser.isSet(dbPortOption)) {
        bool ok;
        dbPort = parser.value(dbPortOption).toInt(&ok);
        if (!ok) {
            std::cerr << "Error: Invalid database port." << std::endl;
            parser.showHelp(1);
            return false;
        }
    }

    if (parser.isSet(listenPortOption)) {
        bool ok;
        listenPort = parser.value(listenPortOption).toUShort(&ok);
        if (!ok || listenPort == 0) {
            std::cerr << "Error: Invalid or missing listen port." << std::endl;
            parser.showHelp(1);
            return false;
        }
    } else {
        std::cerr << "Error: --listenport is required." << std::endl;
        parser.showHelp(1);
        return false;
    }

    if (parser.isSet(targetIpOption)) {
        targetIp = QHostAddress(parser.value(targetIpOption));
        if (targetIp.isNull()) {
            std::cerr << "Error: Invalid target IP address." << std::endl;
            parser.showHelp(1);
            return false;
        }
    }

    if (parser.isSet(targetPortOption)) {
        bool ok;
        targetPort = parser.value(targetPortOption).toUShort(&ok);
        if (!ok || targetPort == 0) {
            std::cerr << "Error: Invalid or missing target port." << std::endl;
            parser.showHelp(1);
            return false;
        }
    } else {
        std::cerr << "Error: --targetport is required." << std::endl;
        parser.showHelp(1);
        return false;
    }
    return true;
}

void AppConfig::printUsage() const {
    qInfo() << "Usage: udp-chat --listenport <port> --targetip <ip> --targetport <port> [DB options]";
    qInfo() << "DB options:";
    qInfo() << "  --dbdriver <driver> (Default: QSQLITE)";
    qInfo() << "  --dbname <databasename> (Default: chat_log.sqlite)";
    qInfo() << "  --dbhost <hostname>";
    qInfo() << "  --dbport <port>";
    qInfo() << "  --dbuser <username>";
    qInfo() << "  --dbpass <password>";
}