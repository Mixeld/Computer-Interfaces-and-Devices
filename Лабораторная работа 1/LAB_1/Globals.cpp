#include "Globals.h"

int  g_criticalThreshold = 15;
int  g_reportInterval    = 5;
volatile bool g_isMonitoring = true;

PowerState currentState;