// functionTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <intelEncoder.h>
#include <Windows.h>
#include <iostream>
#include <vector>
using namespace std;

typedef struct containerH264Data
{
    char* data;
    int size;
}H264DataPacket;

vector<H264DataPacket*>* pH264DataList;

void H264DataCallbackFunction(char* H264Data, int dataSize, unsigned int width, unsigned int height, unsigned int frameRate)
{
    H264DataPacket* pH264DataPacketTemp = new H264DataPacket;
    pH264DataPacketTemp->data = new char[dataSize];
    memcpy(pH264DataPacketTemp->data, H264Data, dataSize);
    pH264DataPacketTemp->size = dataSize;
    pH264DataList->push_back(pH264DataPacketTemp);
}

int _tmain(int argc, _TCHAR* argv[])
{
    FILE* pDataFile = fopen("D:\\test_video_data.yuv", "rb");
    if(NULL == pDataFile)
    {
        cout << "fopen error" << endl;
        system("puase");
        exit(-1);
    }

    // YUV420p 10 seconds 25fps
    char* YUVDataBuffer = (char*)malloc(10 * 25 * 1920 * 1080 * 3 / 2);
    if(NULL == YUVDataBuffer)
    {
        cout << "memory error" << endl;
        system("puase");
        exit(-1);
    }

    if(10 * 25 != fread(YUVDataBuffer, 1920 * 1080 * 3 / 2, 10 * 25, pDataFile))
    {
        cout << "file read error" << endl;
        system("puase");
        exit(-1);
    }

    if(0 != fclose(pDataFile))
    {
        cout << "fclose error" << endl;
        system("puase");
        exit(-1);
    }

    pH264DataList = new vector<H264DataPacket*>;

    initialIntelEncoder(10);

    int instanceTemp = getIdleIntelEncoderInstance();

    setIntelEncoderParameter(instanceTemp, 1920, 1080, 25, H264DataCallbackFunction);

    char* pCurrentYUV420Frame = YUVDataBuffer;

    cout << "start encoding" << endl;
    system("pause");

    SYSTEMTIME startSystemTime;
    GetLocalTime(&startSystemTime);

    for(size_t i = 0; i < 10 * 25; i++)
    {
        putRawDataIntoIntelEncoder(instanceTemp, pCurrentYUV420Frame, 1920 * 1080 * 3 / 2);
        Sleep(30);
        pCurrentYUV420Frame += 1920 * 1080 * 3 / 2;
    }

    //pCurrentYUV420Frame = YUVDataBuffer;

    //for(size_t i = 0; i < 10 * 25; i++)
    //{
    //    putRawDataIntoIntelEncoder(instanceTemp, pCurrentYUV420Frame, 1920 * 1080 * 3 / 2);
    //    //Sleep(30);
    //    pCurrentYUV420Frame += 1920 * 1080 * 3 / 2;
    //}

    //pCurrentYUV420Frame = YUVDataBuffer;

    //for(size_t i = 0; i < 10 * 25; i++)
    //{
    //    putRawDataIntoIntelEncoder(instanceTemp, pCurrentYUV420Frame, 1920 * 1080 * 3 / 2);
    //    //Sleep(30);
    //    pCurrentYUV420Frame += 1920 * 1080 * 3 / 2;
    //}

    SYSTEMTIME endSystemTime;
    GetLocalTime(&endSystemTime);

    cout << "end encoding" << endl;
    system("pause");

    freeIntelEncoderInstance(instanceTemp);

    freeIntelEncoder();

    free(YUVDataBuffer);
    YUVDataBuffer = NULL;

    pDataFile = fopen("D:\\intel_encoder.h264", "wb");
    if(NULL == pDataFile)
    {
        cout << "fopen error" << endl;
        system("puase");
        exit(-1);
    }

    cout << "H264 total frame number: " << pH264DataList->size() << endl;

    for(size_t i = 0; i < pH264DataList->size(); i++)
    {
        fwrite((*pH264DataList)[i]->data, (*pH264DataList)[i]->size, 1, pDataFile);
        delete[](*pH264DataList)[i]->data;
        delete (*pH264DataList)[i];
    }

    if(0 != fclose(pDataFile))
    {
        cout << "fclose error" << endl;
        system("puase");
        exit(-1);
    }

    delete pH264DataList;

    cout << "start time:" << endl;
    cout << "Year: " << startSystemTime.wYear << endl;
    cout << "Month: " << startSystemTime.wMonth << endl;
    cout << "Day: " << startSystemTime.wDay << endl;
    cout << "Hour: " << startSystemTime.wHour << endl;
    cout << "Minute: " << startSystemTime.wMinute << endl;
    cout << "Second: " << startSystemTime.wSecond << endl;
    cout << "Milliseconds: " << startSystemTime.wMilliseconds << endl;

    cout << endl;

    cout << "start time:" << endl;
    cout << "Year: " << endSystemTime.wYear << endl;
    cout << "Month: " << endSystemTime.wMonth << endl;
    cout << "Day: " << endSystemTime.wDay << endl;
    cout << "Hour: " << endSystemTime.wHour << endl;
    cout << "Minute: " << endSystemTime.wMinute << endl;
    cout << "Second: " << endSystemTime.wSecond << endl;
    cout << "Milliseconds: " << endSystemTime.wMilliseconds << endl;

    cout << endl;

    cout << "total time: " << (endSystemTime.wMinute - startSystemTime.wMinute) * 60 * 1000 + (endSystemTime.wSecond - startSystemTime.wSecond) * 1000 + endSystemTime.wMilliseconds - startSystemTime.wMilliseconds - 10 * 25 * 30 << " ms" << endl;

    cout << "frame per second: " << 10 * 25/* * 3*/ * 1000 / ((endSystemTime.wMinute - startSystemTime.wMinute) * 60 * 1000 + (endSystemTime.wSecond - startSystemTime.wSecond) * 1000 + endSystemTime.wMilliseconds - startSystemTime.wMilliseconds - 10 * 25 * 30/* * 3*/) << " fps" << endl;

    system("pause");

    return 0;
}

