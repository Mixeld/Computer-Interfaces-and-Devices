#ifndef REPORTS_H
#define REPORTS_H

#include "Globals.h"
#include <string>

std::string GetCurrentTimeString();
std::string GetActivePowerScheme();
bool GetBatteryTemperature(int& temperature);
void SaveEventToLog(const std::wstring& event);
void SaveReport();
void ReportThread();

#endif // REPORTS_H