#include <windows.h>
#include <gl/gl.h>

//These are initialized before any custom or shared screensaver code is run
extern int screenWidth;
extern int screenHeight;

//These must be defined in the screensaver source file
void Init();
void Draw(HDC hdc, float timeElapsed);
BOOL DialogInit(HWND hDlg);
BOOL DialogCommand(HWND hDlg, WORD command);
