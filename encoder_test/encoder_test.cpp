// encoder_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat\avformat.h>
#include <libavutil\avutil.h>
#include <libavutil\opt.h>

#ifdef __cplusplus
}
#endif

#include <iostream>
#include <vector>
using namespace std;

typedef struct containerH264Data
{
    char* buffer;
    int size;
}H264Data;

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

    av_register_all();

    AVCodec* pAVCodec = avcodec_find_encoder(CODEC_ID_H264);
    AVCodecContext* pAVCodecContext = avcodec_alloc_context3(pAVCodec);

    pAVCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    pAVCodecContext->width = 1920;
    pAVCodecContext->height = 1080;
    pAVCodecContext->time_base.den = 25;
    pAVCodecContext->time_base.num = 1;
    pAVCodecContext->gop_size = 24;
    pAVCodecContext->max_b_frames = 23;

    //av_opt_set(pAVCodecContext->priv_data, "preset", "slow", 0);
    av_opt_set(pAVCodecContext->priv_data, "preset", "ultrafast", 0);
    av_opt_set(pAVCodecContext->priv_data, "tune", "zerolatency", 0);
    av_opt_set(pAVCodecContext->priv_data, "crf", "30", 0);

    if(0 > avcodec_open2(pAVCodecContext, pAVCodec, NULL))
    {
        cout << "avcodec_open2 error" << endl;
        system("puase");
        exit(-1);
    }

    AVFrame* pAVFrame = av_frame_alloc();

    avpicture_alloc((AVPicture*)pAVFrame, AV_PIX_FMT_YUV420P, 1920, 1080);

    pAVFrame->width = 1920;
    pAVFrame->height = 1080;
    pAVFrame->format = AV_PIX_FMT_YUV420P;

    AVPacket* pAVPacket = (AVPacket*)malloc(sizeof(AVPacket));

    char* pCurrentYUV420Frame = YUVDataBuffer;

    int gotPicture;

    vector<H264Data*> H264DataVector;

    H264Data* pBufferTemp;

    SYSTEMTIME startSystemTime;
    GetLocalTime(&startSystemTime);

    for(size_t i = 0; i < 10; i++)
    {
        /* encode 1 second of video */
        for(size_t j = 0; j < 25; j++)
        {

            av_init_packet(pAVPacket);
            pAVPacket->data = NULL;
            pAVPacket->size = 0;

            fflush(stdout);

            memcpy(pAVFrame->data[0], pCurrentYUV420Frame, 1920 * 1080);
            memcpy(pAVFrame->data[1], pCurrentYUV420Frame + 1920 * 1080, 1920 * 1080 / 4);
            memcpy(pAVFrame->data[2], pCurrentYUV420Frame + 1920 * 1080 * 5 / 4, 1920 * 1080 / 4);
            pAVFrame->pts = j;
            pCurrentYUV420Frame += 1920 * 1080 * 3 / 2;

            if(0 > avcodec_encode_video2(pAVCodecContext, pAVPacket, pAVFrame, &gotPicture))
            {
                continue;
            }
            else
            {
                if(gotPicture)
                {
                    pBufferTemp = (H264Data*)malloc(sizeof(H264Data));
                    pBufferTemp->buffer = (char*)malloc(pAVPacket->size);
                    memcpy(pBufferTemp->buffer, pAVPacket->data, pAVPacket->size);
                    pBufferTemp->size = pAVPacket->size;
                    H264DataVector.push_back(pBufferTemp);
                    av_free_packet(pAVPacket);
                }
            }
            Sleep(30);
        }
    }

    SYSTEMTIME endSystemTime;
    GetLocalTime(&endSystemTime);

    av_free_packet(pAVPacket);

    free(pAVPacket);
    pAVPacket = NULL;

    av_frame_free(&pAVFrame);

    avcodec_close(pAVCodecContext);
    avcodec_free_context(&pAVCodecContext);

    free(YUVDataBuffer);
    YUVDataBuffer = NULL;

    cout << "H264 total frame number: " << H264DataVector.size() << endl;

    pDataFile = fopen("D:\\x264_encoder.h264", "wb");
    if(NULL == pDataFile)
    {
        cout << "fopen error" << endl;
        system("puase");
        exit(-1);
    }

    for(size_t i = 0; i < H264DataVector.size(); i++)
    {
        fwrite(H264DataVector[i]->buffer, H264DataVector[i]->size, 1, pDataFile);
        free(H264DataVector[i]->buffer);
        free(H264DataVector[i]);
    }

    if(0 != fclose(pDataFile))
    {
        cout << "fclose error" << endl;
        system("puase");
        exit(-1);
    }

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

    cout << "frame per second: " << 10 * 25 * 100 / ((endSystemTime.wMinute - startSystemTime.wMinute) * 60 * 1000 + (endSystemTime.wSecond - startSystemTime.wSecond) * 1000 + endSystemTime.wMilliseconds - startSystemTime.wMilliseconds - 10 * 25 * 30) << endl;

    system("pause");

    return 0;
}

