#include "Notifications.h"
#include <strsafe.h> //Функции работы со строками

using namespace std;

//========== ФУНКЦИИ РАБОТЫ С УВЕДОМЛЕНИЯМИ ==========

//Функция показа уведомлений
void ShowNotification(const wstring& title, const wstring& message, int iconType) {
    NOTIFYICONDATAW nidNotify = nid;
    nidNotify.uFlags = NIF_INFO;      // Показываем уведомление
    nidNotify.dwInfoFlags = iconType; // Тип уведомления (INFO/WARNING/ERROR)
    nidNotify.uTimeout = 5000;        // 5 сек

    //Копируем строки
    StringCchCopyW(nidNotify.szInfoTitle, 64,  title.c_str());
    StringCchCopyW(nidNotify.szInfo,      256, message.c_str());

    //Обновляем иконку
    Shell_NotifyIconW(NIM_MODIFY, &nidNotify);
}