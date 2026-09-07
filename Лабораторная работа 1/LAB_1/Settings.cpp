#include "Settings.h"
#include <string>

using namespace std;

//========== ФУНКЦИИ РАБОТЫ С НАСТРОЙКАМИ ==========

//Функция получения пути к файлу настроек
wstring GetConfigPath() {
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
    wstring configPath = path;
    // БАГФИКС: экранирование обратных слэшей
    configPath += L"\\PowerReports\\config.ini";
    return configPath;
}

//Функция проверки существования файла конфигурации
bool ConfigFileExists() {
    wstring configPath = GetConfigPath();
    DWORD attrs = GetFileAttributesW(configPath.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

//Функция загрузки настроек из файла
void LoadSettings() {
    wstring configPath = GetConfigPath();

    //Создаем папку если её нет
    wstring folderPath = configPath.substr(0, configPath.find_last_of(L'\\'));
    CreateDirectoryW(folderPath.c_str(), NULL);

    // Добавить проверку на существование файла конфигурации.
    if (ConfigFileExists()) {
        // Очищаем кэш Windows для этого файла, чтобы принудительно прочесть изменения с диска
        WritePrivateProfileStringW(NULL, NULL, NULL, configPath.c_str());

        // Если таковой имеется то значения по умолчанию выставлять именно из этого файла
        g_criticalThreshold = GetPrivateProfileIntW(L"PowerMonitor", L"CriticalThreshold", 15, configPath.c_str());
        g_reportInterval    = GetPrivateProfileIntW(L"PowerMonitor", L"ReportInterval", 5, configPath.c_str());
    } else {
        //Файла нет - используем значения по умолчанию
        g_criticalThreshold = 15;
        g_reportInterval = 5;
        
        // Сразу сохраняем дефолтные значения, чтобы файл создался на диске
        SaveSettings();
    }
}

//Функция сохранения настроек в файл
void SaveSettings() {
    wstring configPath = GetConfigPath();

    //Создаем папку если её нет
    wstring folderPath = configPath.substr(0, configPath.find_last_of(L'\\'));
    CreateDirectoryW(folderPath.c_str(), NULL);

    wchar_t buf[32];

    // Записываем значения через стандартный WinAPI
    wsprintfW(buf, L"%d", g_criticalThreshold);
    WritePrivateProfileStringW(L"PowerMonitor", L"CriticalThreshold", buf, configPath.c_str());

    wsprintfW(buf, L"%d", g_reportInterval);
    WritePrivateProfileStringW(L"PowerMonitor", L"ReportInterval", buf, configPath.c_str());

    // Принудительно сбрасываем кэш на диск, чтобы изменения применились мгновенно
    WritePrivateProfileStringW(NULL, NULL, NULL, configPath.c_str());
}