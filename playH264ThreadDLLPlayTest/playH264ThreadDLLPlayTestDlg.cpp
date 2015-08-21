
// playH264ThreadDLLPlayTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "playH264ThreadDLLPlayTest.h"
#include "playH264ThreadDLLPlayTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment( linker, "/subsystem:console /entry:WinMainCRTStartup" )

CplayH264ThreadDLLPlayTestDlg::CplayH264ThreadDLLPlayTestDlg(CWnd* pParent /*=NULL*/): CDialogEx(CplayH264ThreadDLLPlayTestDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pUINTsize = NULL;
    m_pBuffer = NULL;
}

void CplayH264ThreadDLLPlayTestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CplayH264ThreadDLLPlayTestDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

DWORD WINAPI DecodeThread(LPVOID lpParam)
{
    unsigned int CurrentPos = 0;
    unsigned int DataJumpBytes = 0;
    CplayH264ThreadDLLPlayTestDlg* pCplayH264ThreadDLLPlayTestDlgTemp = static_cast<CplayH264ThreadDLLPlayTestDlg*>(lpParam);
    while(pCplayH264ThreadDLLPlayTestDlgTemp->m_bLoop)
    {
        pCplayH264ThreadDLLPlayTestDlgTemp->m_pfunctioninputBuf(pCplayH264ThreadDLLPlayTestDlgTemp->m_instance,
                                                                pCplayH264ThreadDLLPlayTestDlgTemp->m_pBuffer + DataJumpBytes,
                                                                pCplayH264ThreadDLLPlayTestDlgTemp->m_pUINTsize[CurrentPos]);

        Sleep(30);

        DataJumpBytes += pCplayH264ThreadDLLPlayTestDlgTemp->m_pUINTsize[CurrentPos];
        CurrentPos++;
        if(CurrentPos >= pCplayH264ThreadDLLPlayTestDlgTemp->m_test_video_size_file_size / 4)
        {
            CurrentPos = 0;
            DataJumpBytes = 0;
        }
    }

    return 0;
}


// CplayH264ThreadDLLPlayTestDlg message handlers

BOOL CplayH264ThreadDLLPlayTestDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    AllocConsole();

    DWORD lastError;

    wprintf(_T("file: %s, line: %d\n"), __FILEW__, __LINE__);

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here
    m_hPlayH264DLL = LoadLibrary(_T("playH264ThreadDLL.dll"));
    if(NULL == m_hPlayH264DLL)
    {
        AfxMessageBox(_T("LoadLibrary playH264ThreadDLL.dll failed"));
        lastError = GetLastError();
        exit(-1);
    }

    m_pfunctionfreeVideos = (functionfreeVideos)GetProcAddress(m_hPlayH264DLL, "freeVideos");
    if(NULL == m_pfunctionfreeVideos)
    {
        AfxMessageBox(_T("GetProcAddress freeVideos error"));
    }

    m_pfunctionGetIdlevideoINSTANCE = (functionGetIdlevideoINSTANCE)GetProcAddress(m_hPlayH264DLL, "GetIdlevideoINSTANCE");
    if(NULL == m_pfunctionGetIdlevideoINSTANCE)
    {
        AfxMessageBox(_T("GetProcAddress GetIdlevideoINSTANCE error"));
    }

    m_pfunctioninitVideoDLL = (functioninitVideoDLL)GetProcAddress(m_hPlayH264DLL, "initVideoDLL");
    if(NULL == m_pfunctioninitVideoDLL)
    {
        AfxMessageBox(_T("GetProcAddress initVideoDLL error"));
    }

    m_pfunctionInitVideoParam = (functionInitVideoParam)GetProcAddress(m_hPlayH264DLL, "InitVideoParam");
    if(NULL == m_pfunctionInitVideoParam)
    {
        AfxMessageBox(_T("GetProcAddress InitVideoParam error"));
    }

    m_pfunctioninputBuf = (functioninputBuf)GetProcAddress(m_hPlayH264DLL, "inputBuf");
    if(NULL == m_pfunctioninputBuf)
    {
        AfxMessageBox(_T("GetProcAddress inputBuf error"));
    }

    m_pfunctionplayVideos = (functionplayVideos)GetProcAddress(m_hPlayH264DLL, "playVideos");
    if(NULL == m_pfunctionplayVideos)
    {
        AfxMessageBox(_T("GetProcAddress playVideos error"));
    }

    FILE* pDataFile;
    errno_t error = fopen_s(&pDataFile, "D:\\test_video_data.h264", "rb");
    FILE* pSizeFile;
    error = fopen_s(&pSizeFile, "D:\\test_video_size", "rb");

    fseek(pDataFile, 0, SEEK_END);
    fseek(pSizeFile, 0, SEEK_END);

    m_test_video_data_file_size = ftell(pDataFile);
    m_test_video_size_file_size = ftell(pSizeFile);

    rewind(pDataFile);
    rewind(pSizeFile);

    m_pUINTsize = new unsigned int[m_test_video_size_file_size / 4];
    m_pBuffer = new char[m_test_video_data_file_size];

    fread(m_pUINTsize, m_test_video_size_file_size, 1, pSizeFile);
    fread(m_pBuffer, m_test_video_data_file_size, 1, pDataFile);

    fclose(pSizeFile);
    fclose(pDataFile);

    lastError = m_pfunctioninitVideoDLL();

    m_instance = m_pfunctionGetIdlevideoINSTANCE();

    RECT rectTemp;

    GetWindowRect(&rectTemp);

    m_paramUser.fps = 25;
    m_paramUser.height = 1080;
    m_paramUser.playChannel = 1;
    m_paramUser.playHandle = this->m_hWnd;
    m_paramUser.playHeight = rectTemp.bottom - rectTemp.top;
    m_paramUser.playWidth = rectTemp.right - rectTemp.left;
    m_paramUser.stopPlay = 0;
    m_paramUser.width = 1920;

    lastError = m_pfunctionInitVideoParam(m_instance, &m_paramUser, 1);

    m_bLoop = true;

    m_hThread = CreateThread(NULL, 0, DecodeThread, this, 0, &lastError);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CplayH264ThreadDLLPlayTestDlg::OnPaint()
{
    if(IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CplayH264ThreadDLLPlayTestDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}



BOOL CplayH264ThreadDLLPlayTestDlg::DestroyWindow()
{
    // TODO: Add your specialized code here and/or call the base class

    m_bLoop = false;

    WaitForSingleObject(m_hThread, INFINITE);

    m_pfunctionfreeVideos(m_instance);

    if(NULL != m_pUINTsize)
    {
        delete[] m_pUINTsize;
        m_pUINTsize = NULL;
    }

    if(NULL != m_pBuffer)
    {
        delete[] m_pBuffer;
        m_pBuffer = NULL;
    }

    FreeLibrary(m_hPlayH264DLL);

    FreeConsole();

    return CDialogEx::DestroyWindow();
}
