#pragma once
#include <mfxvideo++.h>
#include <concurrent_queue.h>

typedef void(*H264DataCallback)(char* H264Data, int dataSize, unsigned int width, unsigned int height, unsigned int frameRate);

typedef struct containerYUVData
{
    char* data;
    int size;
}YUVDataPacket;

typedef struct containerInstanceData
{
    mfxIMPL implementation;
    mfxVersion intelSDKVersion;
    mfxU16 originWidth;
    mfxU16 originHeight;
    mfxU32 frameRate;
    mfxU32 frameCount;
    mfxU32 frameCountForKeyFrame;
    H264DataCallback pFunctionH264DataCallback;
    bool encodeLoop;
    Concurrency::concurrent_queue<YUVDataPacket*> YUVDataQueue;
    int pixelFormat;
}instanceData;
