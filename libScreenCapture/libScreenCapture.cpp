// libScreenCapture.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "libScreenCapture.h"
#include <intelEncoder.h>
#include <stdlib.h>

#define MFX_MAKEFOURCC(A,B,C,D)    ((((int)A))+(((int)B)<<8)+(((int)C)<<16)+(((int)D)<<24))

typedef struct containerGDIObjects
{
    int nScreenWidth;
    int nScreenHeight;
    int frameRate;
    HWND hDesktopWnd;
    HDC hDesktopDC;
    HDC hCaptureDC;
    HBITMAP hCaptureBitmap;
    HBITMAP hOldBitmap;
    char* pBmpBuffer;
    bool bLoop;
    HGDIOBJ oldGDIobj;
    int instance;
}GDIObjects;

GDIObjects globalGIDObjects;
HANDLE hGlobalThread;

DWORD WINAPI captureThread(LPVOID lpParam)
{
    GDIObjects* pGDIObjects = static_cast<GDIObjects*>(lpParam);

    int sleepTime = 1000 / pGDIObjects->frameRate;

    BITMAPINFO* bmpInfo = (BITMAPINFO*)malloc(sizeof(BITMAPINFO) + 64);
    if(NULL == bmpInfo)
    {
        exit(-1);
    }

    char* bmpBuffer = (char*)malloc(1920 * 1080 * 4 + 1024);
    if(NULL == bmpBuffer)
    {
        exit(-1);
    }

    memset(bmpBuffer, 0xFF, 1920 * 1080 * 4 + 1024);

    char* pCurrentBGRA;
    char* pCurrentARGB;

    memset(bmpInfo, 0x0, sizeof(BITMAPINFO));
    bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    GetDIBits(pGDIObjects->hDesktopDC, pGDIObjects->hCaptureBitmap, 0, 0, NULL, bmpInfo, DIB_RGB_COLORS);

    SYSTEMTIME start;
    SYSTEMTIME end;

    while(pGDIObjects->bLoop)
    {
        GetSystemTime(&start);

        BitBlt(pGDIObjects->hCaptureDC, 0, 0, pGDIObjects->nScreenWidth, pGDIObjects->nScreenHeight, pGDIObjects->hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);
        GetDIBits(pGDIObjects->hDesktopDC, pGDIObjects->hCaptureBitmap, 0, bmpInfo->bmiHeader.biHeight, pGDIObjects->pBmpBuffer, bmpInfo, DIB_RGB_COLORS);

        pCurrentBGRA = pGDIObjects->pBmpBuffer;
        pCurrentARGB = bmpBuffer;

        for(size_t i = 0; i < pGDIObjects->nScreenWidth * pGDIObjects->nScreenHeight; i++)
        {
            pCurrentARGB[3] = pCurrentBGRA[0];
            pCurrentARGB[2] = pCurrentBGRA[1];
            pCurrentARGB[1] = pCurrentBGRA[2];
            pCurrentARGB[0] = 0xFF;

            pCurrentBGRA += 4;
            pCurrentARGB += 4;
        }

        putRawDataIntoIntelEncoder(pGDIObjects->instance, pGDIObjects->pBmpBuffer, bmpInfo->bmiHeader.biSizeImage);

        GetSystemTime(&end);

        if(start.wYear != end.wYear || start.wMonth != end.wMonth || start.wDay != end.wDay || start.wHour != end.wHour || start.wMinute != end.wMinute)
        {
            continue;
        }

        if(sleepTime > (end.wSecond - start.wSecond) * 1000 + end.wMilliseconds - start.wMilliseconds)
        {
            Sleep(sleepTime - (end.wSecond - start.wSecond) * 1000 - end.wMilliseconds + start.wMilliseconds);
        }
        else
        {
            continue;
        }
    }

    if(NULL != bmpBuffer)
    {
        free(bmpBuffer);
        bmpBuffer = NULL;
    }

    if(NULL != bmpInfo)
    {
        free(bmpInfo);
        bmpInfo = NULL;
    }

    return 0;
}

LIBSCREENCAPTURE_API int startScreenCapture(int frameRate, H264DataCallback functionH264Data)
{
    globalGIDObjects.frameRate = frameRate;

    globalGIDObjects.pBmpBuffer = new char[1920 * 1080 * 4 + 1024];
    if(NULL == globalGIDObjects.pBmpBuffer)
    {
        return -1;
    }

    globalGIDObjects.nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    globalGIDObjects.nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    if(0 == globalGIDObjects.nScreenHeight || 0 == globalGIDObjects.nScreenWidth)
    {
        return -1;
    }

    globalGIDObjects.hDesktopWnd = GetDesktopWindow();
    if(NULL == globalGIDObjects.hDesktopWnd)
    {
        return -1;
    }

    globalGIDObjects.hDesktopDC = GetDC(globalGIDObjects.hDesktopWnd);
    if(NULL == globalGIDObjects.hDesktopDC)
    {
        return -1;
    }

    globalGIDObjects.hCaptureDC = CreateCompatibleDC(globalGIDObjects.hDesktopDC);
    if(NULL == globalGIDObjects.hCaptureDC)
    {
        return -1;
    }

    globalGIDObjects.hCaptureBitmap = CreateCompatibleBitmap(globalGIDObjects.hDesktopDC, globalGIDObjects.nScreenWidth, globalGIDObjects.nScreenHeight);
    if(NULL == globalGIDObjects.hCaptureBitmap)
    {
        return -1;
    }

    globalGIDObjects.hOldBitmap = (HBITMAP)SelectObject(globalGIDObjects.hCaptureDC, globalGIDObjects.hCaptureBitmap);
    if(NULL == globalGIDObjects.hOldBitmap)
    {
        return -1;
    }

    if(0 > initialIntelEncoder(2))
    {
        return -1;
    }

    globalGIDObjects.instance = getIdleIntelEncoderInstance();
    if(0 > globalGIDObjects.instance)
    {
        return -1;
    }

    setIntelEncoderParameter(globalGIDObjects.instance, globalGIDObjects.nScreenWidth, globalGIDObjects.nScreenHeight, globalGIDObjects.frameRate, MFX_MAKEFOURCC('R', 'G', 'B', '4'), functionH264Data);

    globalGIDObjects.bLoop = true;

    hGlobalThread = CreateThread(NULL, 0, captureThread, &globalGIDObjects, 0, NULL);

    return 0;
}

LIBSCREENCAPTURE_API int stopScreenCapture(void)
{
    globalGIDObjects.bLoop = false;

    WaitForSingleObject(hGlobalThread, INFINITE);

    CloseHandle(hGlobalThread);

    freeIntelEncoderInstance(globalGIDObjects.instance);

    freeIntelEncoder();

    if(NULL != globalGIDObjects.hCaptureDC && NULL != globalGIDObjects.hOldBitmap)
    {
        SelectObject(globalGIDObjects.hCaptureDC, globalGIDObjects.hOldBitmap);
        globalGIDObjects.hOldBitmap = NULL;
    }

    if(NULL != globalGIDObjects.hCaptureBitmap)
    {
        DeleteObject(globalGIDObjects.hCaptureBitmap);
        globalGIDObjects.hCaptureBitmap = NULL;
    }

    if(NULL != globalGIDObjects.hCaptureDC)
    {
        DeleteDC(globalGIDObjects.hCaptureDC);
        globalGIDObjects.hCaptureDC = NULL;
    }

    if(NULL != globalGIDObjects.hDesktopWnd && NULL != globalGIDObjects.hDesktopDC)
    {
        ReleaseDC(globalGIDObjects.hDesktopWnd, globalGIDObjects.hDesktopDC);
        globalGIDObjects.hDesktopDC = NULL;
    }

    if(NULL != globalGIDObjects.pBmpBuffer)
    {
        delete[] globalGIDObjects.pBmpBuffer;
        globalGIDObjects.pBmpBuffer = NULL;
    }

    return 0;
}
