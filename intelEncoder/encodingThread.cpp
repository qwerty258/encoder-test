#include "stdafx.h"
#include "encodingThread.h"
#include "common_directx.h"
#include "common_utils.h"
#include <concurrent_queue.h>

#define EXITCODE -99

#include "dataStructs.h"

// For use with asynchronous task management
typedef struct
{
    mfxBitstream mfxBS;
    mfxSyncPoint syncp;
} Task;

int GetFreeSurfaceIndex(mfxFrameSurface1** pSurfacesPool, mfxU16 nPoolSize)
{
    if(pSurfacesPool)
    {
        for(mfxU16 i = 0; i < nPoolSize; i++)
        {
            if(0 == pSurfacesPool[i]->Data.Locked)
            {
                return i;
            }
        }
    }
    return MFX_ERR_NOT_FOUND;
}

// Get free task 
int GetFreeTaskIndex(Task* pTaskPool, mfxU16 nPoolSize)
{
    if(pTaskPool)
        for(int i = 0; i < nPoolSize; i++)
            if(!pTaskPool[i].syncp)
                return i;
    return MFX_ERR_NOT_FOUND;
}

DWORD WINAPI encodingThread(LPVOID lpParam)
{
    instanceData* pInstanceData = static_cast<instanceData*>(lpParam);

    mfxStatus sts = MFX_ERR_NONE;

    MFXVideoSession mfxSession;
    sts = mfxSession.Init(pInstanceData->implementation, &pInstanceData->intelSDKVersion);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Initialize encoder parameters
    mfxVideoParam mfxEncParams;
    memset(&mfxEncParams, 0, sizeof(mfxEncParams));
    mfxEncParams.mfx.CodecId = MFX_CODEC_AVC;
    mfxEncParams.mfx.TargetUsage = MFX_TARGETUSAGE_BEST_SPEED;
    mfxEncParams.mfx.TargetKbps = 10240;
    mfxEncParams.mfx.RateControlMethod = MFX_RATECONTROL_VCM;
    mfxEncParams.mfx.FrameInfo.FrameRateExtN = pInstanceData->frameRate;
    mfxEncParams.mfx.FrameInfo.FrameRateExtD = 1;
    mfxEncParams.mfx.FrameInfo.FourCC = pInstanceData->pixelFormat;  //MFX_FOURCC_NV12;
    if(MFX_FOURCC_NV12 == pInstanceData->pixelFormat)
    {
        mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    }
    mfxEncParams.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    mfxEncParams.mfx.FrameInfo.CropX = 0;
    mfxEncParams.mfx.FrameInfo.CropY = 0;
    mfxEncParams.mfx.FrameInfo.CropW = pInstanceData->originWidth;
    mfxEncParams.mfx.FrameInfo.CropH = pInstanceData->originHeight;
    // Width must be a multiple of 16 
    // Height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
    mfxEncParams.mfx.FrameInfo.Width = MSDK_ALIGN16(pInstanceData->originWidth);
    mfxEncParams.mfx.FrameInfo.Height = (MFX_PICSTRUCT_PROGRESSIVE == mfxEncParams.mfx.FrameInfo.PicStruct) ? MSDK_ALIGN16(pInstanceData->originHeight) : MSDK_ALIGN32(pInstanceData->originHeight);
    mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
    mfxEncParams.mfx.GopPicSize = pInstanceData->frameRate;
    //mfxEncParams.mfx.GopRefDist = 5;
    mfxEncParams.mfx.GopOptFlag = MFX_GOP_CLOSED;
    //mfxEncParams.mfx.EncodedOrder = 1;
    mfxEncParams.mfx.CodecProfile = MFX_PROFILE_AVC_BASELINE;
    //mfxEncParams.mfx.CodecLevel = MFX_LEVEL_AVC_4;

    // Create Media SDK encoder
    MFXVideoENCODE mfxENC(mfxSession);

    // Validate video encode parameters (optional)
    // - In this example the validation result is written to same structure
    // - MFX_WRN_INCOMPATIBLE_VIDEO_PARAM is returned if some of the video parameters are not supported,
    //   instead the encoder will select suitable parameters closest matching the requested configuration
    sts = mfxENC.Query(&mfxEncParams, &mfxEncParams);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Query number of required surfaces for encoder
    mfxFrameAllocRequest EncRequest;
    memset(&EncRequest, 0, sizeof(EncRequest));
    sts = mfxENC.QueryIOSurf(&mfxEncParams, &EncRequest);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    mfxU16 nEncSurfNum = EncRequest.NumFrameSuggested;

    // Allocate surfaces for encoder
    // - Width and height of buffer must be aligned, a multiple of 32 
    // - Frame surface array keeps pointers all surface planes and general frame info
    mfxU16 width = (mfxU16)MSDK_ALIGN32(EncRequest.Info.Width);
    mfxU16 height = (mfxU16)MSDK_ALIGN32(EncRequest.Info.Height);
    mfxU8  bitsPerPixel = 12;  // NV12 format is a 12 bits per pixel format
    mfxU32 surfaceSize = width * height * bitsPerPixel / 8;
    mfxU8* surfaceBuffers = (mfxU8 *)new mfxU8[surfaceSize * nEncSurfNum];

    mfxFrameSurface1** pEncSurfaces = new mfxFrameSurface1*[nEncSurfNum];
    MSDK_CHECK_POINTER(pEncSurfaces, MFX_ERR_MEMORY_ALLOC);
    for(int i = 0; i < nEncSurfNum; i++)
    {
        pEncSurfaces[i] = new mfxFrameSurface1;
        memset(pEncSurfaces[i], 0, sizeof(mfxFrameSurface1));
        memcpy(&(pEncSurfaces[i]->Info), &(mfxEncParams.mfx.FrameInfo), sizeof(mfxFrameInfo));
        pEncSurfaces[i]->Data.Y = &surfaceBuffers[surfaceSize * i];
        pEncSurfaces[i]->Data.U = pEncSurfaces[i]->Data.Y + width * height;
        pEncSurfaces[i]->Data.V = pEncSurfaces[i]->Data.U + 1;
        pEncSurfaces[i]->Data.Pitch = width;
        pEncSurfaces[i]->Data.B = new mfxU8[1920 * 1080 * 4 + 1024];
    }

    // Initialize the Media SDK encoder
    sts = mfxENC.Init(&mfxEncParams);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Retrieve video parameters selected by encoder.
    // - BufferSizeInKB parameter is required to set bit stream buffer size
    mfxVideoParam par;
    memset(&par, 0, sizeof(par));
    sts = mfxENC.GetVideoParam(&par);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Prepare Media SDK bit stream buffer
    mfxBitstream mfxBS;
    memset(&mfxBS, 0, sizeof(mfxBS));
    mfxBS.MaxLength = par.mfx.BufferSizeInKB * 1000;
    mfxBS.Data = new mfxU8[mfxBS.MaxLength];
    MSDK_CHECK_POINTER(mfxBS.Data, MFX_ERR_MEMORY_ALLOC);

    int nEncSurfIdx = 0;
    mfxSyncPoint syncp;

    // enter encoding loop
    while(pInstanceData->encodeLoop)
    {
        //
        // Stage 1: Main encoding loop
        //
        while(MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts)
        {
            nEncSurfIdx = GetFreeSurfaceIndex(pEncSurfaces, nEncSurfNum); // Find free frame surface  
            MSDK_CHECK_ERROR(MFX_ERR_NOT_FOUND, nEncSurfIdx, MFX_ERR_MEMORY_ALLOC);

            if(MFX_FOURCC_NV12 == pInstanceData->pixelFormat)
            {
                sts = LoadRawFrame(pEncSurfaces[nEncSurfIdx], pInstanceData);
            }
            if(MFX_FOURCC_RGB4 == pInstanceData->pixelFormat)
            {
                sts = LoadRawRGBFrame(pEncSurfaces[nEncSurfIdx], pInstanceData);
            }
            MSDK_BREAK_ON_ERROR(sts);

            for(;;)
            {
                // Encode a frame asychronously (returns immediately)
                sts = mfxENC.EncodeFrameAsync(NULL, pEncSurfaces[nEncSurfIdx], &mfxBS, &syncp);

                if(MFX_ERR_NONE < sts && !syncp) // Repeat the call if warning and no output
                {
                    if(MFX_WRN_DEVICE_BUSY == sts)
                        Sleep(1); // Wait if device is busy, then repeat the same call            
                }
                else if(MFX_ERR_NONE < sts && syncp)
                {
                    sts = MFX_ERR_NONE; // Ignore warnings if output is available                                    
                    break;
                }
                else if(MFX_ERR_NOT_ENOUGH_BUFFER == sts)
                {
                    // Allocate more bitstream buffer memory here if needed...
                    break;
                }
                else
                    break;
            }

            if(MFX_ERR_NONE == sts)
            {
                sts = mfxSession.SyncOperation(syncp, 60000); // Synchronize. Wait until encoded frame is ready
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

                sts = WriteBitStreamFrame(&mfxBS, pInstanceData);
                MSDK_BREAK_ON_ERROR(sts);
            }
        }

        // MFX_ERR_MORE_DATA means that the input file has ended, need to go to buffering loop, exit in case of other errors
        MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_DATA);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        //
        // Stage 2: Retrieve the buffered encoded frames
        //
        while(MFX_ERR_NONE <= sts)
        {
            for(;;)
            {
                // Encode a frame asychronously (returns immediately)
                sts = mfxENC.EncodeFrameAsync(NULL, NULL, &mfxBS, &syncp);

                if(MFX_ERR_NONE < sts && !syncp) // Repeat the call if warning and no output
                {
                    if(MFX_WRN_DEVICE_BUSY == sts)
                        Sleep(1); // Wait if device is busy, then repeat the same call                 
                }
                else if(MFX_ERR_NONE < sts && syncp)
                {
                    sts = MFX_ERR_NONE; // Ignore warnings if output is available                                    
                    break;
                }
                else
                    break;
            }

            if(MFX_ERR_NONE == sts)
            {
                sts = mfxSession.SyncOperation(syncp, 60000); // Synchronize. Wait until encoded frame is ready
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

                sts = WriteBitStreamFrame(&mfxBS, pInstanceData);
                MSDK_BREAK_ON_ERROR(sts);
            }
        }

        // MFX_ERR_MORE_DATA indicates that there are no more buffered frames, exit in case of other errors
        MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_DATA);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    }

    // ===================================================================
    // Clean up resources
    //  - It is recommended to close Media SDK components first, before releasing allocated surfaces, since
    //    some surfaces may still be locked by internal Media SDK resources.

    mfxENC.Close();
    // mfxSession closed automatically on destruction

    for(int i = 0; i < nEncSurfNum; i++)
    {
        delete[] pEncSurfaces[i]->Data.B;
        delete pEncSurfaces[i];
    }
    MSDK_SAFE_DELETE_ARRAY(pEncSurfaces);
    MSDK_SAFE_DELETE_ARRAY(surfaceBuffers);
    MSDK_SAFE_DELETE_ARRAY(mfxBS.Data);

    return 0;
}