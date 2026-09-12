#include "Settings.h"
#include <string>

using namespace std;

wstring GetConfigPath() {
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
    wstring configPath = path;
    configPath += L"\\PowerReports\\config.ini";
    return configPath;
}

bool ConfigFileExists() {
    wstring configPath = GetConfigPath();
    DWORD attrs = GetFileAttributesW(configPath.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

void LoadSettings() {
    wstring configPath = GetConfigPath();

    wstring folderPath = configPath.substr(0, configPath.find_last_of(L'\\'));
    CreateDirectoryW(folderPath.c_str(), NULL);

    if (ConfigFileExists()) {
        WritePrivateProfileStringW(NULL, NULL, NULL, configPath.c_str());

        g_criticalThreshold = GetPrivateProfileIntW(L"PowerMonitor", L"CriticalThreshold", 15, configPath.c_str());
        g_reportInterval    = GetPrivateProfileIntW(L"PowerMonitor", L"ReportInterval", 5, configPath.c_str());
    } else {
        g_criticalThreshold = 15;
        g_reportInterval    = 5;
        SaveSettings();
    }
}

void SaveSettings() {
    wstring configPath = GetConfigPath();

    wstring folderPath = configPath.substr(0, configPath.find_last_of(L'\\'));
    CreateDirectoryW(folderPath.c_str(), NULL);

    wchar_t buf[32];

    wsprintfW(buf, L"%d", g_criticalThreshold);
    WritePrivateProfileStringW(L"PowerMonitor", L"CriticalThreshold", buf, configPath.c_str());

    wsprintfW(buf, L"%d", g_reportInterval);
    WritePrivateProfileStringW(L"PowerMonitor", L"ReportInterval", buf, configPath.c_str());

    WritePrivateProfileStringW(NULL, NULL, NULL, configPath.c_str());
}