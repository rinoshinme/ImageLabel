
// ImageLabel.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CImageLabelApp:
// �йش����ʵ�֣������ ImageLabel.cpp
//

class CImageLabelApp : public CWinApp
{
public:
	CImageLabelApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CImageLabelApp theApp;