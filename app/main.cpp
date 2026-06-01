//
// Created by BenzoicAcid on 2026/5/12.
//

#include "app/Application.h"
#include "presentation/common/Theme.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    QFont appFont = app.font();
    appFont.setPointSize(Theme::AppFontPointSize);
    app.setFont(appFont);

    Application::instance().start();

    return app.exec();
}
