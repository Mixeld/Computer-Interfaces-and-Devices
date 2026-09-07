#include "Reports.h"
#include "Notifications.h"
#include <powrprof.h> //Для получения схемы питания
#include <ctime>      //Для работы со временем
#include <thread>     //Для отдельного потока
#include <chrono>     //Для задержек
#include <cstdio>

#pragma comment(lib, "powrprof.lib") //Линкуем

using namespace std;

//========== ФУНКЦИИ РАБОТЫ С ОТЧЕТАМИ ==========

//Функция получения текущего времени в строку
string GetCurrentTimeString() {
    time_t now = time(nullptr);
    struct tm tstruct;
    struct tm* ptm = localtime(&now);
    if (ptm) {
        tstruct = *ptm;
    } else {
        tstruct = {};
    }
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
    return string(buf);
}

//Функция получения активной схемы питания
string GetActivePowerScheme() {
    GUID* activeGuid = nullptr; //Указатель на GUID, который заполнит функция

    if (PowerGetActiveScheme(NULL, &activeGuid) == ERROR_SUCCESS) {
        char str[64];
        snprintf(str, sizeof(str),
                 "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                 activeGuid->Data1, activeGuid->Data2, activeGuid->Data3,
                 activeGuid->Data4[0], activeGuid->Data4[1],
                 activeGuid->Data4[2], activeGuid->Data4[3],
                 activeGuid->Data4[4], activeGuid->Data4[5],
                 activeGuid->Data4[6], activeGuid->Data4[7]);

        //Освобождаем память, выделенную функцией
        LocalFree(activeGuid);
        return string(str);
    }
    return "Неизвестно";
}

//Функция получения температуры батареи (упрощенная)
bool GetBatteryTemperature(int& temperature) {
    //В реальном приложении здесь должен быть код через WMI
    //Для демонстрации возвращаем примерное значение
    temperature = 25;
    return true;
}

//Функция сохранения события в лог
void SaveEventToLog(const wstring& event) {
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
    wstring logPath = path;
    // БАГФИКС: экранирование слэшей
    logPath += L"\\PowerReports\\events.log";

    //Создаем папку если её нет
    wstring folderPath = logPath.substr(0, logPath.find_last_of(L'\\'));
    CreateDirectoryW(folderPath.c_str(), NULL);

    HANDLE hFile = CreateFileW(logPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                               NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        SetFilePointer(hFile, 0, NULL, FILE_END);

        string timeStr = GetCurrentTimeString();
        string content = timeStr + " - ";

        //Конвертируем wstring в string через WinAPI
        int len = WideCharToMultiByte(CP_ACP, 0, event.c_str(), -1, NULL, 0, NULL, NULL);
        if (len > 0) {
            string buffer(len - 1, 0);
            WideCharToMultiByte(CP_ACP, 0, event.c_str(), -1, &buffer[0], len, NULL, NULL);
            content += buffer;
        }
        content += "\r\n";

        DWORD bytesWritten = 0;
        WriteFile(hFile, content.c_str(), (DWORD)content.length(), &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

//Функция сохранения отчета
void SaveReport() {
    //Получаем путь к папке отчетов
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
    wstring reportsPath = path;
    // БАГФИКС: экранирование слэшей
    reportsPath += L"\\PowerReports\\";

    //Создаем папку если её нет
    CreateDirectoryW(reportsPath.c_str(), NULL);

    //Формируем имя файла с датой
    time_t now = time(nullptr);
    struct tm tstruct;
    struct tm* ptm = localtime(&now);
    if (ptm) {
        tstruct = *ptm;
    } else {
        tstruct = {};
    }
    char timeBuf[80];
    strftime(timeBuf, sizeof(timeBuf), "%Y%m%d_%H%M%S", &tstruct);

    wstring filename = reportsPath;
    filename += L"power_report_";

    //Конвертируем char* в wchar_t* через WinAPI
    wchar_t wtimeBuf[80] = {0};
    MultiByteToWideChar(CP_ACP, 0, timeBuf, -1, wtimeBuf, 80);
    filename += wtimeBuf;
    filename += L".txt";

    //Получаем статус питания
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status)) {
        return;
    }

    //Открываем файл для записи
    HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_WRITE, 0,
                               NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    //Формируем содержимое отчета
    string content = "=== ОТЧЕТ О СОСТОЯНИИ ПИТАНИЯ ===\r\n";
    content += "Время: " + GetCurrentTimeString() + "\r\n";
    content += "----------------------------------------\r\n";

    //Уровень заряда
    if (status.BatteryLifePercent != 255) {
        content += "Уровень заряда: " + to_string((int)status.BatteryLifePercent) + "%\r\n";
    } else {
        content += "Уровень заряда: Неизвестно\r\n";
    }

    //Источник питания
    switch (status.ACLineStatus) {
        case 0:  content += "Источник питания: Батарея\r\n"; break;
        case 1:  content += "Источник питания: Сеть\r\n";    break;
        default: content += "Источник питания: Неизвестно\r\n";
    }

    //Состояние зарядки
    if (status.BatteryFlag & 8) {
        content += "Состояние: Заряжается\r\n";
    } else {
        content += "Состояние: Не заряжается\r\n";
    }

    //Оставшееся время
    // БАГФИКС: BatteryLifeTime имеет тип DWORD; неизвестное значение = 0xFFFFFFFF
    if (status.BatteryLifeTime != (DWORD)-1 && status.BatteryLifeTime != 0) {
        int hours = status.BatteryLifeTime / 3600;
        int minutes = (status.BatteryLifeTime % 3600) / 60;
        content += "Оставшееся время: " + to_string(hours) + "ч " + to_string(minutes) + "мин\r\n";
    }

    //Активная схема питания
    content += "Активная схема питания: " + GetActivePowerScheme() + "\r\n";

    //Температура батареи
    int temperature = 0;
    if (GetBatteryTemperature(temperature)) {
        content += "Температура батареи: " + to_string(temperature) + "°C\r\n";
    }

    content += "----------------------------------------\r\n";
    content += "Порог критического заряда: " + to_string(g_criticalThreshold) + "%\r\n";
    content += "Интервал отчетов: "         + to_string(g_reportInterval)    + " мин\r\n";

    //Записываем в файл
    DWORD bytesWritten = 0;
    WriteFile(hFile, content.c_str(), (DWORD)content.length(), &bytesWritten, NULL);
    CloseHandle(hFile);

    //Показываем уведомление
    wchar_t msg[256];
    wsprintfW(msg, L"Отчет сохранен в папке PowerReports");
    ShowNotification(L"Отчет сохранен", msg, NIIF_INFO);
}

//Функция потока для автоматического сохранения отчетов
void ReportThread() {
    int counter = 0;

    while (g_isMonitoring) {
        //Спим 1 минуту
        this_thread::sleep_for(chrono::minutes(1));
        if (!g_isMonitoring) break;
        counter++;

        //Если прошло g_reportInterval минут - сохраняем отчет
        if (counter >= g_reportInterval) {
            SaveReport();
            counter = 0;
        }
    }
}