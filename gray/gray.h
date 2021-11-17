#include <windows.h>
#include <scrnsave.h>
#include <commctrl.h>
#include <thread>
#include <gl/gl.h>

HDC hdc;
HGLRC hglrc;
int screenWidth;
int screenHeight;
LARGE_INTEGER ticksPerSecond;
LARGE_INTEGER startTime;

void SetupOpengl(HWND hWnd);
void TeardownOpengl(HWND hWnd);
void Draw();
