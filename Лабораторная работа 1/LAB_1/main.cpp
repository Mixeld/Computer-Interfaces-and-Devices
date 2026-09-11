#include "Globals.h"
#include "Settings.h"
#include "MainWindow.h"
#include "Reports.h"
#include <thread>

using namespace std;

//========== ТОЧКА ВХОДА ==========

extern "C" int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    // Регистрируем класс главного окна
    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"PowerMonitorClass";
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClassExW(&wc);

    // Загружаем настройки из файла
    LoadSettings();

    // Создаем главное окно (оно скрыто, работаем из трея)
    hwndMain = CreateWindowExW(
        0,
        L"PowerMonitorClass",
        L"Power Monitor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 300,
        NULL, NULL,
        hInstance, NULL
    );

    if (!hwndMain) {
        MessageBoxW(NULL, L"Ошибка создания окна!", L"Ошибка", MB_OK);
        return 1;
    }

    ShowWindow(hwndMain, SW_HIDE);

    // Запускаем поток для автоматического сохранения отчетов
    thread reportThread(ReportThread);
    reportThread.detach();

    // Цикл обработки сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}