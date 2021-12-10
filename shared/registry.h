#include <windows.h>

DWORD LoadRegistryDword(const char* name);
bool SaveRegistryDword(const char* name, DWORD value);
void DeleteRegistryFolders();
