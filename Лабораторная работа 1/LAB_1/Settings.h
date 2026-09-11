#ifndef SETTINGS_H
#define SETTINGS_H

#include "Globals.h"
#include <string>

// Функция получения пути к файлу настроек
std::wstring GetConfigPath();

// Функция проверки существования файла конфигурации
bool ConfigFileExists();

// Функция загрузки настроек из файла
void LoadSettings();

// Функция сохранения настроек в файл
void SaveSettings();

#endif // SETTINGS_H