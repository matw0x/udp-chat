#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QHostAddress>
#include <QString>

class AppConfig {
   public:
    AppConfig();

    QString dbDriver;
    QString dbHostName;
    QString dbName;
    QString dbUserName;
    QString dbPassword;
    int     dbPort;

    quint16      listenPort;
    QHostAddress targetIp;
    quint16      targetPort;

    bool parseCommandLineArguments(const QStringList& args);
    void printUsage() const;
};

#endif  // APPCONFIG_H