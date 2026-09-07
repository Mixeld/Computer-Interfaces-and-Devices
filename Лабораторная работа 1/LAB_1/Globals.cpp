#include "Globals.h"

// Определения глобальных переменных
NOTIFYICONDATAW nid = {};
HWND hwndMain = nullptr;
HWND hwndSettings = nullptr;

int  g_criticalThreshold = 15;
int  g_reportInterval = 5;
bool g_isMonitoring = true;

PowerState currentState;