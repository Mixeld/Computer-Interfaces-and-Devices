#include "Globals.h"
#include "Settings.h"
#include "TrayApp.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Программа живёт в трее — не выходим при закрытии окон
    app.setQuitOnLastWindowClosed(false);

    // Загружаем настройки из .ini
    LoadSettings();

    // Создаём трей и запускаем всё
    TrayApp trayApp;

    return app.exec();
}