//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2013 Intel Corporation. All Rights Reserved.
//
#include "stdafx.h"
#include "common_utils.h"

// =================================================================
// Utility functions, not directly tied to Intel Media SDK functionality
//

mfxStatus ReadPlaneData(mfxU16 w, mfxU16 h, mfxU8 *buf, mfxU8 *ptr, mfxU16 pitch, mfxU16 offset, FILE* fSource)
{
    mfxU32 nBytesRead;
    for(mfxU16 i = 0; i < h; i++)
    {
        nBytesRead = (mfxU32)fread(buf, 1, w, fSource);
        if(w != nBytesRead)
            return MFX_ERR_MORE_DATA;
        for(mfxU16 j = 0; j < w; j++)
            ptr[i * pitch + j * 2 + offset] = buf[j];
    }
    return MFX_ERR_NONE;
}

mfxStatus LoadRawFrame(mfxFrameSurface1* pSurface, instanceData* pIstanceData, bool bSim)
{
    if(bSim)
    {
        // Simulate instantaneous access to 1000 "empty" frames. 
        static int frameCount = 0;
        if(1000 == frameCount++) return MFX_ERR_MORE_DATA;
        else return MFX_ERR_NONE;
    }

    YUVDataPacket* pYUVDataPacket;

    if(pIstanceData->frameCount >= pIstanceData->frameRate)
    {
        pIstanceData->frameCount = 0;
        return MFX_ERR_MORE_DATA;
    }

    while(!pIstanceData->YUVDataQueue.try_pop(pYUVDataPacket))
    {
        Sleep(30);
    }

    if(-99 == pYUVDataPacket->size)
    {
        if(NULL != pYUVDataPacket)
        {
            if(NULL != pYUVDataPacket->data)
            {
                delete[] pYUVDataPacket->data;
            }
            delete pYUVDataPacket;
        }
        return MFX_ERR_MORE_DATA;
    }

    mfxStatus sts = MFX_ERR_NONE;
    mfxU16 w, h, pitch;
    mfxU8 *ptr;
    mfxFrameInfo* pInfo = &pSurface->Info;
    mfxFrameData* pData = &pSurface->Data;

    if(pInfo->CropH > 0 && pInfo->CropW > 0)
    {
        w = pInfo->CropW;
        h = pInfo->CropH;
    }
    else
    {
        w = pInfo->Width;
        h = pInfo->Height;
    }

    pitch = pData->Pitch;
    ptr = pData->Y + pInfo->CropX + pInfo->CropY * pData->Pitch;

    char* currentPosition = pYUVDataPacket->data;
    // read luminance plane
    //for(i = 0; i < h; i++)
    //{
    //nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, fSource);
    //if(w != nBytesRead)
    //    return MFX_ERR_MORE_DATA;
    //    currentPosition += w;
    //    memcpy(ptr + i * pitch, currentPosition, w);
    //}
    memcpy(ptr, currentPosition, w * h);

    //mfxU8 buf[2048]; // maximum supported chroma width for nv12
    w /= 2;
    h /= 2;
    ptr = pSurface->Data.UV + pSurface->Info.CropX + (pSurface->Info.CropY / 2) * pitch;
    if(w > 2048)
        return MFX_ERR_UNSUPPORTED;

    currentPosition = pYUVDataPacket->data + w * h * 4;

    // load U
    //sts = ReadPlaneData(w, h, buf, ptr, pitch, 0, fSource);
    //if(MFX_ERR_NONE != sts) return sts;
    for(mfxU16 i = 0; i < h; i++)
    {
        //nBytesRead = (mfxU32)fread(buf, 1, w, fSource);
        //if(w != nBytesRead)
        //    return MFX_ERR_MORE_DATA;
        currentPosition += w;
        for(mfxU16 j = 0; j < w; j++)
            ptr[i * pitch + j * 2] = currentPosition[j];
    }

    // load V
    //ReadPlaneData(w, h, buf, ptr, pitch, 1, fSource);
    //if(MFX_ERR_NONE != sts) return sts;

    currentPosition = pYUVDataPacket->data + w * h * 4 + w * h;

    for(mfxU16 i = 0; i < h; i++)
    {
        //nBytesRead = (mfxU32)fread(buf, 1, w, fSource);
        //if(w != nBytesRead)
        //    return MFX_ERR_MORE_DATA;
        currentPosition += w;
        for(mfxU16 j = 0; j < w; j++)
            ptr[i * pitch + j * 2 + 1] = currentPosition[j];
    }

    pIstanceData->frameCount++;

    if(NULL != pYUVDataPacket)
    {
        if(NULL != pYUVDataPacket->data)
        {
            delete[] pYUVDataPacket->data;
        }
        delete pYUVDataPacket;
    }

    return MFX_ERR_NONE;
}

mfxStatus LoadRawRGBFrame(mfxFrameSurface1* pSurface, instanceData* pIstanceData, bool bSim)
{
    if(bSim)
    {
        // Simulate instantaneous access to 1000 "empty" frames. 
        static int frameCount = 0;
        if(1000 == frameCount++) return MFX_ERR_MORE_DATA;
        else return MFX_ERR_NONE;
    }

    YUVDataPacket* pYUVDataPacket;

    if(pIstanceData->frameCount >= pIstanceData->frameRate)
    {
        pIstanceData->frameCount = 0;
        return MFX_ERR_MORE_DATA;
    }

    while(!pIstanceData->YUVDataQueue.try_pop(pYUVDataPacket))
    {
        Sleep(30);
    }

    if(-99 == pYUVDataPacket->size)
    {
        if(NULL != pYUVDataPacket)
        {
            if(NULL != pYUVDataPacket->data)
            {
                delete[] pYUVDataPacket->data;
            }
            delete pYUVDataPacket;
        }
        return MFX_ERR_MORE_DATA;
    }

    size_t nBytesRead;
    mfxU16 w, h;
    mfxFrameInfo* pInfo = &pSurface->Info;

    if(pInfo->CropH > 0 && pInfo->CropW > 0)
    {
        w = pInfo->CropW;
        h = pInfo->CropH;
    }
    else
    {
        w = pInfo->Width;
        h = pInfo->Height;
    }

    memcpy(pSurface->Data.B, pYUVDataPacket->data, pYUVDataPacket->size);

    //for(mfxU16 i = 0; i < h; i++)
    //{
    //    nBytesRead = (mfxU32)fread(pSurface->Data.B + i * pSurface->Data.Pitch, 1, w * 4, fSource);
    //    if(w * 4 != nBytesRead)
    //        return MFX_ERR_MORE_DATA;
    //}

    if(NULL != pYUVDataPacket)
    {
        if(NULL != pYUVDataPacket->data)
        {
            delete[] pYUVDataPacket->data;
        }
        delete pYUVDataPacket;
    }

    return MFX_ERR_NONE;
}

mfxStatus WriteBitStreamFrame(mfxBitstream *pMfxBitstream, instanceData* pInstanceData)
{
    //mfxU32 nBytesWritten = (mfxU32)fwrite(pMfxBitstream->Data + pMfxBitstream->DataOffset, 1, pMfxBitstream->DataLength, fSink);
    mfxU8* pCurrentPos = pMfxBitstream->Data + pMfxBitstream->DataOffset;
    mfxU8* pNextPos = NULL;
    mfxU8 binaryToFind[4] = {0, 0, 0, 1};

    pCurrentPos = myFind(pCurrentPos, pMfxBitstream->DataLength, binaryToFind, 4);
    if(NULL == pCurrentPos)
    {
        return MFX_ERR_NONE;
    }

    do
    {
        //if(0x27 == *(pCurrentPos + 4))
        //{
        //    *(pCurrentPos + 4) = 0x67;
        //}

        //if(0x28 == *(pCurrentPos + 4))
        //{
        //    *(pCurrentPos + 4) = 0x68;
        //}

        pNextPos = myFind(pCurrentPos + 4, pMfxBitstream->DataLength - (pCurrentPos + 4 - pMfxBitstream->Data - pMfxBitstream->DataOffset), binaryToFind, 4);

        if(NULL != pNextPos)
        {
            pInstanceData->pFunctionH264DataCallback((char*)pCurrentPos, pNextPos - pCurrentPos, pInstanceData->originWidth, pInstanceData->originHeight, pInstanceData->frameRate);
        }
        else
        {
            pInstanceData->pFunctionH264DataCallback((char*)pCurrentPos, pMfxBitstream->DataLength - (pCurrentPos - (pMfxBitstream->Data + pMfxBitstream->DataOffset)), pInstanceData->originWidth, pInstanceData->originHeight, pInstanceData->frameRate);
        }

        pCurrentPos = pNextPos;
    } while(NULL != pCurrentPos);


    //if(nBytesWritten != pMfxBitstream->DataLength)
    //    return MFX_ERR_UNDEFINED_BEHAVIOR;

    pMfxBitstream->DataLength = 0;

    return MFX_ERR_NONE;
}

mfxStatus ReadBitStreamData(mfxBitstream *pBS, FILE* fSource)
{
    memmove(pBS->Data, pBS->Data + pBS->DataOffset, pBS->DataLength);
    pBS->DataOffset = 0;

    mfxU32 nBytesRead = (mfxU32)fread(pBS->Data + pBS->DataLength, 1, pBS->MaxLength - pBS->DataLength, fSource);
    if(0 == nBytesRead)
        return MFX_ERR_MORE_DATA;

    pBS->DataLength += nBytesRead;

    return MFX_ERR_NONE;
}

mfxStatus WriteSection(mfxU8* plane, mfxU16 factor, mfxU16 chunksize, mfxFrameInfo *pInfo, mfxFrameData *pData, mfxU32 i, mfxU32 j, FILE* fSink)
{
    if(chunksize != fwrite(plane + (pInfo->CropY * pData->Pitch / factor + pInfo->CropX) + i * pData->Pitch + j,
                            1,
                            chunksize,
                            fSink))
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    return MFX_ERR_NONE;
}

mfxStatus WriteRawFrame(mfxFrameSurface1 *pSurface, FILE* fSink)
{
    mfxFrameInfo *pInfo = &pSurface->Info;
    mfxFrameData *pData = &pSurface->Data;
    mfxU32 i, j, h, w;
    mfxStatus sts = MFX_ERR_NONE;

    for(i = 0; i < pInfo->CropH; i++)
        sts = WriteSection(pData->Y, 1, pInfo->CropW, pInfo, pData, i, 0, fSink);

    h = pInfo->CropH / 2;
    w = pInfo->CropW;
    for(i = 0; i < h; i++)
        for(j = 0; j < w; j += 2)
            sts = WriteSection(pData->UV, 2, 1, pInfo, pData, i, j, fSink);
    for(i = 0; i < h; i++)
        for(j = 1; j < w; j += 2)
            sts = WriteSection(pData->UV, 2, 1, pInfo, pData, i, j, fSink);

    return sts;
}

unsigned char* myFind(unsigned char* source, size_t sourceSize, unsigned char* subBintoFind, size_t subBinSize)
{
    if(NULL == source || NULL == subBintoFind)
    {
        return NULL;
    }
    if(sourceSize < subBinSize)
    {
        return NULL;
    }
    size_t i;
    for(i = 0; i <= sourceSize - subBinSize; i++)
    {
        if(0 == memcmp(source + i, subBintoFind, subBinSize))
        {
            return source + i;
        }
    }
    return NULL;
}