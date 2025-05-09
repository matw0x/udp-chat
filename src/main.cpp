#include <QApplication>
#include <QDebug>
#include <cstdlib>

#include "AppConfig.h"
#include "DatabaseLogger.h"
#include "MainWindow.h"
#include "NetworkManager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("UDPChat");

    AppConfig config;
    if (!config.parseCommandLineArguments(QCoreApplication::arguments())) {
        return EXIT_FAILURE;
    }

    DatabaseLogger dbLogger(config);
    if (!dbLogger.initialize()) {
        return qCritical() << "Failed to initialize database logger. Exiting.", EXIT_FAILURE;
    }

    NetworkManager networkManager;
    if (!networkManager.initialize(config.listenPort)) {
        return qCritical() << "Failed to initialize network manager:" << networkManager.localPort(), EXIT_FAILURE;
    }

    MainWindow w(config, &networkManager, &dbLogger);
    w.show();

    return app.exec();
}