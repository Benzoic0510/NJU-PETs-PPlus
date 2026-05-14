//
// Created by BenzoicAcid on 2026/5/12.
//

#include "app/Application.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    Application::instance().start();

    return app.exec();
}
