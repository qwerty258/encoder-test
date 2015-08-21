// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBSCREENCAPTURE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LIBSCREENCAPTURE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LIBSCREENCAPTURE_EXPORTS
#define LIBSCREENCAPTURE_API __declspec(dllexport)
#else
#define LIBSCREENCAPTURE_API __declspec(dllimport)
#endif

typedef void(*H264DataCallback)(char* H264Data, int dataSize, unsigned int width, unsigned int height, unsigned int frameRate);

#ifdef __cplusplus
extern "C" {
#endif

    LIBSCREENCAPTURE_API int startScreenCapture(int frameRate, H264DataCallback functionH264Data);

    LIBSCREENCAPTURE_API int stopScreenCapture(void);

#ifdef __cplusplus
}
#endif