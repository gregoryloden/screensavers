#include <windows.h>

const DWORD backgroundTypeWallpaper = 0;
const DWORD backgroundTypeColors = 1;
const DWORD backgroundTypeScreenshot = 2;

void LoadAppropriateBackground(DWORD background);
DWORD LoadRegistryBackground();
bool SaveRegistryBackground(DWORD background);
