#include "Notifications.h"
#include <strsafe.h>

using namespace std;

//========== ФУНКЦИИ РАБОТЫ С УВЕДОМЛЕНИЯМИ ==========

void ShowNotification(const wstring& title, const wstring& message, int iconType) {
    NOTIFYICONDATAW nidNotify = nid;
    nidNotify.uFlags = NIF_INFO;
    nidNotify.dwInfoFlags = iconType;
    nidNotify.uTimeout = 5000;

    StringCchCopyW(nidNotify.szInfoTitle, 64,  title.c_str());
    StringCchCopyW(nidNotify.szInfo,      256, message.c_str());

    Shell_NotifyIconW(NIM_MODIFY, &nidNotify);
}