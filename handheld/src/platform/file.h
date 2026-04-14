#ifndef FILE_H__
#define FILE_H__

bool DeleteDirectory(const std::string&, bool noRecycleBin = true);

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#include <windows.h>
#include <string>
#include <vector>

bool DeleteDirectory(const std::string& dir, bool noRecycleBin /*true*/)
{
    try {
        std::wstring wideDir(dir.begin(), dir.end());
        DWORD attributes = GetFileAttributesW(wideDir.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES) {
            SetFileAttributesW(wideDir.c_str(), attributes & ~FILE_ATTRIBUTE_READONLY);
        }
        
        std::wstring searchPattern = wideDir + L"\\*";
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(searchPattern.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::wstring fileName = findData.cFileName;
                if (fileName != L"." && fileName != L"..") {
                    std::wstring fullPath = wideDir + L"\\" + fileName;
                    
                    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        std::string subDir(fullPath.begin(), fullPath.end());
                        DeleteDirectory(subDir, noRecycleBin);
                    } else {
                        DeleteFileW(fullPath.c_str());
                    }
                }
            } while (FindNextFileW(hFind, &findData));
            FindClose(hFind);
        }
        
        return RemoveDirectoryW(wideDir.c_str()) != FALSE;
    } catch (...) {
        return false;
    }
}

#elif defined(WIN32)
    #include <windows.h>
    #include <tchar.h>
    #include <shellapi.h>
    #include <string>

bool DeleteDirectory(const std::string& dir, bool noRecycleBin /*true*/)
{
    int len = strlen(dir.c_str());
    //TCHAR *pszFrom = new TCHAR[len+2];
	char* pszFrom = new char[len+2];
    strncpy(pszFrom, dir.c_str(), len);
    pszFrom[len] = 0;
    pszFrom[len+1] = 0;

    SHFILEOPSTRUCT fileop;
    fileop.hwnd   = NULL;    // no status display
    fileop.wFunc  = FO_DELETE;  // delete operation
    fileop.pFrom  = pszFrom;  // source file name as double null terminated string
    fileop.pTo    = NULL;    // no destination needed
    fileop.fFlags = FOF_NOCONFIRMATION|FOF_SILENT;  // do not prompt the user

    if(!noRecycleBin)
		fileop.fFlags |= FOF_ALLOWUNDO;

    fileop.fAnyOperationsAborted = FALSE;
    fileop.lpszProgressTitle     = NULL;
    fileop.hNameMappings         = NULL;

    int ret = SHFileOperation(&fileop);
    delete [] pszFrom;
    return (ret == 0);
}
#else
	#include <cstdio>
	#include <dirent.h>

bool DeleteDirectory(const std::string& d, bool noRecycleBin /*true*/)
{
	const char* folder = d.c_str();

	const size_t CMAX = 1024;
	char fullPath[CMAX];
	DIR* dir = opendir(folder);
	if (!dir)
		return false;

	struct dirent *entry;
	while ((entry = readdir(dir))) {
		if (strcmp(".", entry->d_name) && strcmp("..", entry->d_name)) {
			snprintf(fullPath, CMAX, "%s/%s", folder, entry->d_name);
			remove(fullPath);
		}
	}

	closedir(dir);

	return remove(folder) == 0;
}

#endif /*WIN32/UWP/Non-Windows*/

#endif /*FILE_H__*/
