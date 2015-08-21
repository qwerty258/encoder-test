// intelEncoder.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "intelEncoder.h"
#include "errorHandling.h"
#include "encodingThread.h"
#include <mfxvideo.h>
#include <concurrent_queue.h>
#include "dataStructs.h"

#define EXITCODE -99

int globalMaxInstance;
mfxVersion globalIntelSDKVersion;
mfxIMPL globalImplementation;
instanceData** pGlobalInstanceDataList;
HANDLE* pGlobalHANDLEList;

int checkInstance(int instance)
{
    if(0 > instance || globalMaxInstance < instance || NULL == pGlobalInstanceDataList)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

INTELENCODER_API int initialIntelEncoder(int maxInstance)
{
    if(0 > maxInstance)
    {
        return -1;
    }

    globalMaxInstance = maxInstance;

    pGlobalInstanceDataList = new instanceData*[maxInstance];
    if(NULL == pGlobalInstanceDataList)
    {
        showError(_T("new return NULL"), __LINE__, __FILEW__, -1);
        exit(-1);
    }

    memset(pGlobalInstanceDataList, 0x0, maxInstance * sizeof(instanceData*));

    pGlobalHANDLEList = new HANDLE[maxInstance];
    if(NULL == pGlobalHANDLEList)
    {
        showError(_T("new return NULL"), __LINE__, __FILEW__, -1);
        exit(-1);
    }

    memset(pGlobalHANDLEList, 0x0, maxInstance * sizeof(HANDLE));

    // initial intel SDK
    mfxVersion ver;
    ver.Major = 1;
    ver.Minor = 0;
    mfxSession session;
    mfxStatus status = MFXInit(MFX_IMPL_AUTO_ANY, &ver, &session);
    if(MFX_ERR_NONE > status)
    {
        showError(_T("MFXInit"), __LINE__, __FILEW__, status);
        return -1;
    }

    status = MFXQueryIMPL(session, &globalImplementation);
    if(MFX_ERR_NONE > status)
    {
        showError(_T("MFXQueryIMPL"), __LINE__, __FILEW__, status);
        return -1;
    }

    status = MFXQueryVersion(session, &globalIntelSDKVersion);
    if(MFX_ERR_NONE > status)
    {
        showError(_T("MFXQueryVersion"), __LINE__, __FILEW__, status);
        return -1;
    }

    status = MFXClose(session);
    if(MFX_ERR_NONE > status)
    {
        showError(_T("MFXClose"), __LINE__, __FILEW__, status);
        return -1;
    }

    return 0;
}

INTELENCODER_API void freeIntelEncoder(void)
{
    for(int i = 0; i < globalMaxInstance; i++)
    {
        freeIntelEncoderInstance(i);
    }

    if(NULL != pGlobalInstanceDataList)
    {
        delete[] pGlobalInstanceDataList;
        pGlobalInstanceDataList = NULL;
    }

    if(NULL != pGlobalHANDLEList)
    {
        delete[] pGlobalHANDLEList;
        pGlobalHANDLEList = NULL;
    }
}

INTELENCODER_API int getIdleIntelEncoderInstance(void)
{
    for(int i = 0; i < globalMaxInstance; i++)
    {
        if(NULL == pGlobalInstanceDataList[i])
        {
            pGlobalInstanceDataList[i] = new instanceData;
            if(NULL == pGlobalInstanceDataList[i])
            {
                showError(_T("new return NULL"), __LINE__, __FILEW__, -1);
                return -1;
            }

            pGlobalInstanceDataList[i]->encodeLoop = false;
            pGlobalInstanceDataList[i]->frameRate = 0;
            pGlobalInstanceDataList[i]->frameCount = 0;
            pGlobalInstanceDataList[i]->frameCountForKeyFrame = 0;
            pGlobalInstanceDataList[i]->implementation = 0;
            pGlobalInstanceDataList[i]->intelSDKVersion.Version = 0;
            pGlobalInstanceDataList[i]->originHeight = 0;
            pGlobalInstanceDataList[i]->originWidth = 0;
            pGlobalInstanceDataList[i]->pFunctionH264DataCallback = NULL;
            pGlobalInstanceDataList[i]->pixelFormat = 0;

            return i;
        }
    }
    return -1;
}

INTELENCODER_API int freeIntelEncoderInstance(int instance)
{
    if(0 > checkInstance(instance))
    {
        return -1;
    }

    if(NULL != pGlobalInstanceDataList[instance])
    {
        pGlobalInstanceDataList[instance]->encodeLoop = false;
        YUVDataPacket* pYUVDataPacket = new YUVDataPacket;
        pYUVDataPacket->data = NULL;
        pYUVDataPacket->size = EXITCODE;
        pGlobalInstanceDataList[instance]->YUVDataQueue.push(pYUVDataPacket);
        WaitForSingleObject(pGlobalHANDLEList[instance], INFINITE);

        CloseHandle(pGlobalHANDLEList[instance]);
        pGlobalHANDLEList[instance] = NULL;

        while(pGlobalInstanceDataList[instance]->YUVDataQueue.try_pop(pYUVDataPacket))
        {
            if(NULL != pYUVDataPacket)
            {
                if(NULL != pYUVDataPacket->data)
                {
                    delete[] pYUVDataPacket->data;
                }
                delete pYUVDataPacket;
            }
        }

        delete pGlobalInstanceDataList[instance];
        pGlobalInstanceDataList[instance] = NULL;
    }

    return 0;
}

INTELENCODER_API int setIntelEncoderParameter(int instance, unsigned int width, unsigned int height, unsigned int frameRate, H264DataCallback pFunction, unsigned int rawDataType)
{
    if(0 > checkInstance(instance) || NULL == pGlobalInstanceDataList[instance])
    {
        return -1;
    }

    pGlobalInstanceDataList[instance]->encodeLoop = true;
    pGlobalInstanceDataList[instance]->frameRate = frameRate;
    pGlobalInstanceDataList[instance]->implementation = globalImplementation;
    pGlobalInstanceDataList[instance]->intelSDKVersion = globalIntelSDKVersion;
    pGlobalInstanceDataList[instance]->originHeight = height;
    pGlobalInstanceDataList[instance]->originWidth = width;
    pGlobalInstanceDataList[instance]->pFunctionH264DataCallback = pFunction;
    pGlobalInstanceDataList[instance]->pixelFormat = rawDataType;
    DWORD threadID;
    pGlobalHANDLEList[instance] = CreateThread(NULL, 0, encodingThread, pGlobalInstanceDataList[instance], 0, &threadID);

    return 0;
}

INTELENCODER_API int putRawDataIntoIntelEncoder(int instance, char* YUVData, int dataSize)
{
    if(0 > checkInstance(instance))
    {
        return -1;
    }

    YUVDataPacket* pYUVDataPacket;

    if(NULL != pGlobalHANDLEList[instance])
    {
        pYUVDataPacket = new YUVDataPacket;
        pYUVDataPacket->data = new char[dataSize];
        pYUVDataPacket->size = dataSize;

        memcpy(pYUVDataPacket->data, YUVData, dataSize);

        pGlobalInstanceDataList[instance]->YUVDataQueue.push(pYUVDataPacket);
    }

    return 0;
}