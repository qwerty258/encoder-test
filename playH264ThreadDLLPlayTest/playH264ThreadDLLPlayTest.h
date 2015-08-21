
// playH264ThreadDLLPlayTest.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CplayH264ThreadDLLPlayTestApp:
// See playH264ThreadDLLPlayTest.cpp for the implementation of this class
//

class CplayH264ThreadDLLPlayTestApp : public CWinApp
{
public:
	CplayH264ThreadDLLPlayTestApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CplayH264ThreadDLLPlayTestApp theApp;