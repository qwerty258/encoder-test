// useScreenCapture.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <libScreenCapture.h>
#include <Windows.h>

void H264DataCallbackFunction(char* H264Data, int dataSize, unsigned int width, unsigned int height, unsigned int frameRate)
{
    char path[256];

    //sprintf(path, "D:\\test_video_data_%d.h264", count);
    sprintf_s(path, "D:\\test_video_data.h264");

    FILE* pFile;

    fopen_s(&pFile, path, "ab");

    if(NULL == pFile)
    {
        return;
    }

    fwrite(H264Data, dataSize, 1, pFile);

    fclose(pFile);

    sprintf_s(path, "D:\\test_video_size");

    fopen_s(&pFile, path, "ab");
    if(NULL == pFile)
    {
        return;
    }

    fwrite(&dataSize, 4, 1, pFile);

    fclose(pFile);
}

int _tmain(int argc, _TCHAR* argv[])
{
    _tsystem(_T("pause"));
    startScreenCapture(20, H264DataCallbackFunction);
    _tsystem(_T("pause"));
    stopScreenCapture();
    _tsystem(_T("pause"));
    return 0;
}

