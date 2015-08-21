// H264DLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "H264DeCode.h"
#include <intelEncoder.h>
#include <stdio.h>
#include <concurrent_queue.h>

typedef struct containerH264data
{
    char* data;
    int size;
}H264DataPackage;

BYTE* pGlobalBufferPointer;
unsigned int count;
Concurrency::concurrent_queue<H264DataPackage*> H264DataQueue;
UINT globalCount;
SYSTEMTIME current;

void H264DataCallbackFunction(char* H264Data, int dataSize, unsigned int width, unsigned int height, unsigned int frameRate)
{
    H264DataPackage* pH264DataPackageTemp = new H264DataPackage;
    pH264DataPackageTemp->data = new char[dataSize];
    pH264DataPackageTemp->size = dataSize;
    memcpy(pH264DataPackageTemp->data, H264Data, dataSize);

    H264DataQueue.push(pH264DataPackageTemp);

    //char path[256];

    //++count;

    ////sprintf(path, "D:\\test_video_data_%d.h264", count);
    //sprintf(path, "D:\\test_video_data.h264");

    //FILE* pFile = fopen(path, "ab");
    //if(NULL == pFile)
    //{
    //    return;
    //}

    //fwrite(H264Data, dataSize, 1, pFile);

    //fclose(pFile);

    //sprintf(path, "D:\\test_video_size");

    //pFile = fopen(path, "ab");
    //if(NULL == pFile)
    //{
    //    return;
    //}

    //fwrite(&dataSize, 4, 1, pFile);

    //fclose(pFile);
}

H264DLL_API int InitDLL()
{
    if(0 > initialIntelEncoder(5))
    {
        freeIntelEncoder();
        return -1;
    }

    count = 0;

    globalCount = 0;

    return 0;
}

H264DLL_API int GetIdleDecordINSTANCE()
{
    return getIdleIntelEncoderInstance();
}

H264DLL_API int InitParam(int INSTANCE, paramInput* paramUser)
{
    if(0 > setIntelEncoderParameter(INSTANCE, paramUser->width, paramUser->height, paramUser->fps, H264DataCallbackFunction, MFX_MAKEFOURCC('N', 'V', '1', '2')))
    {
        freeIntelEncoderInstance(INSTANCE);
        return -1;
    }

    return 0;
}

H264DLL_API int EncodeBuf(int INSTANCE, BYTE *inBuf, int inBufsize, int picType, BYTE *outBuf)
{
    int temp;
    pGlobalBufferPointer = outBuf;

    // count capture rate begin
    //globalCount++;
    //char* pChar = new char[1024];

    //GetSystemTime(&current);

    //sprintf_s(pChar, 1024, "YUV count: %u, min: %u,sec: %u,millsec: %u\n", count, current.wMinute, current.wSecond, current.wMilliseconds);

    //FILE* pFile;

    //fopen_s(&pFile, "D:\\input_raw_buffer_rate.log", "ab");

    //fwrite(pChar, strlen(pChar), 1, pFile);

    //fclose(pFile);
    // count capture rate end

    temp = putRawDataIntoIntelEncoder(INSTANCE, (char*)inBuf, inBufsize);
    if(0 > temp)
    {
        return 0;
    }

    H264DataPackage* pH264DataPackageTemp;

    if(H264DataQueue.try_pop(pH264DataPackageTemp))
    {
        memcpy(outBuf, pH264DataPackageTemp->data, pH264DataPackageTemp->size);
        temp = pH264DataPackageTemp->size;
        if(NULL != pH264DataPackageTemp->data)
        {
            delete[] pH264DataPackageTemp->data;
        }
        delete pH264DataPackageTemp;
        return temp;
    }
    else
    {
        return 0;
    }
}

H264DLL_API void ClearDLL(int INSTANCE)
{
    freeIntelEncoderInstance(INSTANCE);
}