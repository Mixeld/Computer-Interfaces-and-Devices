#ifndef REPORTS_H
#define REPORTS_H

#include "Globals.h"
#include <string>

//Функция получения текущего времени в строку
std::string GetCurrentTimeString();

//Функция получения активной схемы питания
std::string GetActivePowerScheme();

//Функция получения температуры батареи (упрощенная)
bool GetBatteryTemperature(int& temperature);

//Функция сохранения события в лог
void SaveEventToLog(const std::wstring& event);

//Функция сохранения отчета
void SaveReport();

//Функция потока для автоматического сохранения отчетов
void ReportThread();

#endif // REPORTS_H