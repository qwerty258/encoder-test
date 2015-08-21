#include "stdafx.h"
#include "errorHandling.h"

void showError(TCHAR* message, size_t line, TCHAR* file, int errorCode)
{
    TCHAR buffer[1024];
#ifdef UNICODE
    swprintf(buffer, _T("file: %s, line: %d\nmessage: %s, error code: %d"), file, line, message, errorCode);
#else
    snprintf(buffer, _T("file: %s, line: %d\nmessage: %s, error code: %d"), file, line, message, errorCode);
#endif
    MessageBox(NULL, buffer, L"ERROR", MB_OK);
}