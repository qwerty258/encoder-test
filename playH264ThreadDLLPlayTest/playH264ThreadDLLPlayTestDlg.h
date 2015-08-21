
// playH264ThreadDLLPlayTestDlg.h : header file
//

#pragma once

typedef struct _paramUser
{
    int width;
    int height;
    int playWidth;
    int playHeight;
    int fps;
    HWND playHandle;
    int stopPlay;
    int playChannel;
}paramUser;

typedef int(*functioninitVideoDLL)(void);
typedef int(*functionfreeVideos)(int INSTANCE);
typedef int(*functionGetIdlevideoINSTANCE)(void);
typedef int(*functionInitVideoParam)(int INSTANCE, paramUser* pParamUser, int type);
typedef int(*functionplayVideos)(int INSTANCE);
typedef int(*functioninputBuf)(int INSTANCE, char* buf, int bufSize);

// CplayH264ThreadDLLPlayTestDlg dialog
class CplayH264ThreadDLLPlayTestDlg : public CDialogEx
{
    // Construction
public:
    CplayH264ThreadDLLPlayTestDlg(CWnd* pParent = NULL);	// standard constructor

    // Dialog Data
    enum
    {
        IDD = IDD_PLAYH264THREADDLLPLAYTEST_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


    // Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    HMODULE m_hPlayH264DLL;
    functionfreeVideos m_pfunctionfreeVideos;
    functionGetIdlevideoINSTANCE m_pfunctionGetIdlevideoINSTANCE;
    functioninitVideoDLL m_pfunctioninitVideoDLL;
    functionInitVideoParam m_pfunctionInitVideoParam;
    functioninputBuf m_pfunctioninputBuf;
    functionplayVideos m_pfunctionplayVideos;
    HANDLE m_hThread;
    virtual BOOL DestroyWindow();
    unsigned int* m_pUINTsize;
    char* m_pBuffer;
    paramUser m_paramUser;
    int m_instance;
    bool m_bLoop;
    int m_test_video_data_file_size;
    int m_test_video_size_file_size;
};
