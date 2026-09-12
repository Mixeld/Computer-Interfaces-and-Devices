#ifndef SETTINGS_H
#define SETTINGS_H

#include "Globals.h"
#include <string>

std::wstring GetConfigPath();
bool ConfigFileExists();
void LoadSettings();
void SaveSettings();

#endif // SETTINGS_H