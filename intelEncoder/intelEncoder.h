// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the INTELENCODER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// INTELENCODER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef INTELENCODER_EXPORTS
#define INTELENCODER_API __declspec(dllexport)
#else
#define INTELENCODER_API __declspec(dllimport)
#endif

#define MFX_MAKEFOURCC(A,B,C,D)    ((((int)A))+(((int)B)<<8)+(((int)C)<<16)+(((int)D)<<24))

typedef void(*H264DataCallback)(char* H264Data, int dataSize, unsigned int width, unsigned int height, unsigned int frameRate);

#ifdef __cplusplus
extern "C" {
#endif

    //************************************
    // Function:  initial Intel Encoder, call once per process
    // Returns:   int: 0 success, -1 failure
    // Parameter: int maxInstance
    //************************************
    INTELENCODER_API int initialIntelEncoder(int maxInstance);

    //************************************
    // Function:  free Intel Encoder, release resource, call once per process
    // Returns:   int: 0 success, -1 failure
    // Parameter: void
    //************************************
    INTELENCODER_API void freeIntelEncoder(void);

    //************************************
    // Function:  return idle instance
    // Returns:   int: idle instance number on success, -1 on failure
    // Parameter: void
    //************************************
    INTELENCODER_API int getIdleIntelEncoderInstance(void);

    // 
    //************************************
    // Function:  free encode instance specified by user,
    //            you MUST stop putting YUV420 data into encoder
    //            before calling freeIntelEncoderInstance()
    // Returns:   int: 0 success, -1 failure
    // Parameter: int instance
    //************************************
    INTELENCODER_API int freeIntelEncoderInstance(int instance);

    //************************************
    // Function:  set encode parameters, please DO NOT put time consuming task in the callback function
    // Returns:   int: 0 success, -1 failure
    // Parameter: int instance: the instance you want to set parameters
    // Parameter: unsigned int width: raw video's width
    // Parameter: unsigned int height: raw video's height
    // Parameter: unsigned int frameRate: raw video's framerate
    // Parameter: H264DataCallback pFunction: the callback function to give out H264 data
    // Parameter: unsigned int rawDataType: add new data type in future, default is YUV420
    //************************************
    INTELENCODER_API int setIntelEncoderParameter(int instance, unsigned int width, unsigned int height, unsigned int frameRate, H264DataCallback pFunction, unsigned int rawDataType = MFX_MAKEFOURCC('N', 'V', '1', '2'));

    //************************************
    // Function:  put raw video data into intel encoder
    // Returns:   int: 0 success, -1 failure
    // Parameter: int instance: the idle instance return by getIdleIntelEncoderInstance()
    // Parameter: char* rawData: a raw frame's buffer
    // Parameter: int dataSize: frame buffer size in bytes
    //************************************
    INTELENCODER_API int putRawDataIntoIntelEncoder(int instance, char* rawData, int dataSize);

#ifdef __cplusplus
}
#endif
