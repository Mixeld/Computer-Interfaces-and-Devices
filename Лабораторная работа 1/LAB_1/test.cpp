#include <windows.h>
#include <iostream>
#include <string>

define _WIN32_WINNT 0x0600 //указываем что мы используем Vista+ систему

void PrintPowerStatus(const SYSTEM_POWER_STATUS& status) {
    cout << "--- Статус питания ---" << endl;

    cout << "Питание от сети: ";
    switch(status.ACLineStatus){
        case 0: cout << "Offline (" << (int)status.BatteryLifeTime << ")" << endl; break;
        case 1: cout << "Online (" << /*(int)status.BatteryFullLifeTime <<*/ ")" << endl; break;
        default: cout << "Неизвестно" << endl;            //Глобально можно 255 (как в документации), но на все случа лучше реагировать на всё
    }  

    cout << "Состояние батареи: ";
    if (status.BatteryFlag & 8) cout << "Заряжается";
    else cout << "Не заряжается ";
    if (status.BatteryFlag & 1) cout << "(высокий заряд)";
    else if (status.BatteryFlag & 2) cout << "(низкий заряд)";
    else if (status.BatteryFlag & 4) cout << "(КРИТИЧЕСКИЙ заряд)";
    else if (status.BatteryFlag & 128) cout << "(батарея отсутствует)"; 
    cout << endl;


    if (status.BatteryLifePercent != 255){
        cout << "Уровень заряда: " << (int)status.BatteryLifePercent << "%" << endl;
    } else {
        cout << "уровень заряда батареи неизвестен" << endl;
    }   
}

int main() {
    SYSTEM_POWER_STATUS status;

    if (GetSystemPowerStatus(&status)) {
        PrintPowerStatus(status);
    } else {
        cerr << "Ошибка получения статуса питания!" << endl;
        return 1;
    }

    system ("pause");
    return 0;
}