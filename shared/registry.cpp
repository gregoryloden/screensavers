#include "registry.h"
#include <scrnsave.h>

const char* getRegistryPath();

DWORD LoadRegistryDword(const char* name) {
	DWORD value;
	DWORD valueSize = sizeof(value);
	LSTATUS status = RegGetValue(HKEY_CURRENT_USER, getRegistryPath(), name, RRF_RT_REG_DWORD, nullptr, &value, &valueSize);
	return status == ERROR_SUCCESS ? value : 0;
}

bool SaveRegistryDword(const char* name, DWORD value) {
	if (value != 0) {
		RegSetKeyValue(HKEY_CURRENT_USER, getRegistryPath(), name, REG_DWORD, &value, sizeof(value));
		return true;
	} else {
		RegDeleteKeyValue(HKEY_CURRENT_USER, getRegistryPath(), name);
		return false;
	}
}

void DeleteRegistryFolders() {
	const char* originalPath = getRegistryPath();
	int pathLen = strlen(originalPath);
	char* path = new char[pathLen + 1];
	memcpy(path, originalPath, pathLen + 1);
	for (char* pathIter = path + pathLen; pathIter != path; pathIter--) {
		// Don't try to delete "Software", only go up to "Gregory Loden"
		if (pathIter[-1] != '\\')
			continue;
		LSTATUS status = RegDeleteKey(HKEY_CURRENT_USER, path);
		if (status != ERROR_SUCCESS)
			break;
		pathIter[-1] = 0;
		HKEY pathKey;
		status = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_READ, &pathKey);
		if (status != ERROR_SUCCESS)
			break;
		DWORD subKeysCount = 1;
		DWORD valuesCount = 1;
		status = RegQueryInfoKey(
			pathKey, nullptr, nullptr, nullptr,
			&subKeysCount, nullptr, nullptr,
			&valuesCount, nullptr, nullptr, nullptr, nullptr);
		if (status != ERROR_SUCCESS || subKeysCount > 0 || valuesCount > 0)
			break;
	}
	delete[] path;
}

const char* getRegistryPath() {
	static char path[256] = "Software\\Gregory Loden\\Screensavers\\";
	static bool pathInitialized = false;
	if (!pathInitialized) {
		int baseLen = strlen(path);
		LoadString(nullptr, IDS_DESCRIPTION, path + baseLen, sizeof(path) - baseLen);
		pathInitialized = true;
	}
	return path;
}
