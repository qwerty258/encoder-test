// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the H264DLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// H264DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef H264DLL_EXPORTS
#define H264DLL_API __declspec(dllexport)
#else
#define H264DLL_API __declspec(dllimport)
#endif

typedef struct
{
    int width;
    int height;
    int fps;
    int bitRate;
}paramInput;

#ifdef __cplusplus
extern "C" {
#endif

    H264DLL_API int InitDLL();

    H264DLL_API int GetIdleDecordINSTANCE();

    H264DLL_API int InitParam(int INSTANCE, paramInput* paramUser);

    H264DLL_API int EncodeBuf(int INSTANCE, BYTE *inBuf, int inBufsize, int picType, BYTE *outBuf);

    H264DLL_API void ClearDLL(int INSTANCE);

#ifdef __cplusplus
}
#endif