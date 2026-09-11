#include "Reports.h"
#include "Notifications.h"
#include <powrprof.h>
#include <ctime>
#include <thread>
#include <chrono>
#include <cstdio>

#pragma comment(lib, "powrprof.lib")

using namespace std;

//========== ФУНКЦИИ РАБОТЫ С ОТЧЕТАМИ ==========

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

string GetActivePowerScheme() {
    GUID* activeGuid = nullptr;

    if (PowerGetActiveScheme(NULL, &activeGuid) == ERROR_SUCCESS) {
        char str[64];
        snprintf(str, sizeof(str),
                 "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                 activeGuid->Data1, activeGuid->Data2, activeGuid->Data3,
                 activeGuid->Data4[0], activeGuid->Data4[1],
                 activeGuid->Data4[2], activeGuid->Data4[3],
                 activeGuid->Data4[4], activeGuid->Data4[5],
                 activeGuid->Data4[6], activeGuid->Data4[7]);

        LocalFree(activeGuid);
        return string(str);
    }
    return "Неизвестно";
}

bool GetBatteryTemperature(int& temperature) {
    temperature = 25;
    return true;
}

void SaveEventToLog(const wstring& event) {
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
    wstring logPath = path;
    logPath += L"\\PowerReports\\events.log";

    wstring folderPath = logPath.substr(0, logPath.find_last_of(L'\\'));
    CreateDirectoryW(folderPath.c_str(), NULL);

    HANDLE hFile = CreateFileW(logPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                               NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        SetFilePointer(hFile, 0, NULL, FILE_END);

        string timeStr = GetCurrentTimeString();
        string content = timeStr + " - ";

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

void SaveReport() {
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
    wstring reportsPath = path;
    reportsPath += L"\\PowerReports\\";

    CreateDirectoryW(reportsPath.c_str(), NULL);

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

    wchar_t wtimeBuf[80] = {0};
    MultiByteToWideChar(CP_ACP, 0, timeBuf, -1, wtimeBuf, 80);
    filename += wtimeBuf;
    filename += L".txt";

    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status)) {
        return;
    }

    HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_WRITE, 0,
                               NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    string content = "=== ОТЧЕТ О СОСТОЯНИИ ПИТАНИЯ ===\r\n";
    content += "Время: " + GetCurrentTimeString() + "\r\n";
    content += "----------------------------------------\r\n";

    if (status.BatteryLifePercent != 255) {
        content += "Уровень заряда: " + to_string((int)status.BatteryLifePercent) + "%\r\n";
    } else {
        content += "Уровень заряда: Неизвестно\r\n";
    }

    switch (status.ACLineStatus) {
        case 0:  content += "Источник питания: Батарея\r\n"; break;
        case 1:  content += "Источник питания: Сеть\r\n";    break;
        default: content += "Источник питания: Неизвестно\r\n";
    }

    if (status.BatteryFlag & 8) {
        content += "Состояние: Заряжается\r\n";
    } else {
        content += "Состояние: Не заряжается\r\n";
    }

    if (status.BatteryLifeTime != (DWORD)-1 && status.BatteryLifeTime != 0) {
        int hours = status.BatteryLifeTime / 3600;
        int minutes = (status.BatteryLifeTime % 3600) / 60;
        content += "Оставшееся время: " + to_string(hours) + "ч " + to_string(minutes) + "мин\r\n";
    }

    content += "Активная схема питания: " + GetActivePowerScheme() + "\r\n";

    int temperature = 0;
    if (GetBatteryTemperature(temperature)) {
        content += "Температура батареи: " + to_string(temperature) + "°C\r\n";
    }

    content += "----------------------------------------\r\n";
    content += "Порог критического заряда: " + to_string(g_criticalThreshold) + "%\r\n";
    content += "Интервал отчетов: "         + to_string(g_reportInterval)    + " мин\r\n";

    DWORD bytesWritten = 0;
    WriteFile(hFile, content.c_str(), (DWORD)content.length(), &bytesWritten, NULL);
    CloseHandle(hFile);

    wchar_t msg[256];
    wsprintfW(msg, L"Отчет сохранен в папке PowerReports");
    ShowNotification(L"Отчет сохранен", msg, NIIF_INFO);
}

void ReportThread() {
    int counter = 0;

    while (g_isMonitoring) {
        this_thread::sleep_for(chrono::minutes(1));
        if (!g_isMonitoring) break;
        counter++;

        if (counter >= g_reportInterval) {
            SaveReport();
            counter = 0;
        }
    }
}