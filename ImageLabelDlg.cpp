
// ImageLabelDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ImageLabel.h"
#include "ImageLabelDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CImageLabelDlg �Ի���

CImageLabelDlg::CImageLabelDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CImageLabelDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pImageView = NULL;
	m_CurFileIndex = 0;
	m_WindowState = _T("");
}

void CImageLabelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FILES, m_ListFiles);
	DDX_Text(pDX, IDC_STATIC_STATE, m_WindowState);
}

BEGIN_MESSAGE_MAP(CImageLabelDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CLOSE, &CImageLabelDlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_OPEN_IMAGE, &CImageLabelDlg::OnBnClickedOpenImage)
	ON_BN_CLICKED(IDC_SAVE_IMAGE_ANNOT, &CImageLabelDlg::OnBnClickedSaveImageAnnot)
	ON_BN_CLICKED(IDC_OPEN_FOLDER, &CImageLabelDlg::OnBnClickedOpenFolder)
	ON_BN_CLICKED(IDC_NEXT_IMAGE, &CImageLabelDlg::OnBnClickedNextImage)
	ON_BN_CLICKED(IDC_PREV_IMAGE, &CImageLabelDlg::OnBnClickedPrevImage)

	ON_COMMAND(ID_MENU_ADD_ANNOT, &CImageLabelDlg::OnMenuAddAnnot)
	ON_COMMAND(ID_MENU_ADD_SUB_ANNOT, &CImageLabelDlg::OnMenuAddSubAnnot)
	ON_COMMAND(ID_MENU_DEL_ANNOT, &CImageLabelDlg::OnMenuDelAnnot)
	ON_COMMAND(ID_MENU_DEL_SUB_ANNOT, &CImageLabelDlg::OnMenuDelSubAnnot)
	ON_COMMAND(ID_MENU_EDIT_ANNOT, &CImageLabelDlg::OnMenuEditAnnot)
	ON_COMMAND(ID_MENU_EDIT_SUB_ANNOT, &CImageLabelDlg::OnMenuEditSubAnnot)
	ON_COMMAND(ID_MENU_CANCEL, &CImageLabelDlg::OnMenuCancel)

	ON_MESSAGE(MSG_IMAGEVIEW_REFRESH_STATUS, &CImageLabelDlg::OnMsgRefreshStatus)

	ON_LBN_DBLCLK(IDC_LIST_FILES, &CImageLabelDlg::OnFileListDbClick)
END_MESSAGE_MAP()


// CImageLabelDlg ��Ϣ�������

BOOL CImageLabelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	m_RBtnMenu.LoadMenu(IDR_MENU1);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	if (m_pImageView == NULL)
	{
		CRect rc;
		rc.left = 2;
		rc.right = DEFAULT_WIDTH + 2;
		rc.top = 2;
		rc.bottom = DEFAULT_HEIGHT + 2;
		m_pImageView = new CImageView();
		m_pImageView->Create(_T("IMAGE_VIEW"), _T(""), WS_CHILD, rc, this, IDC_IMAGE_VIEW, NULL);

		m_pImageView->SetRBtnMenu(m_RBtnMenu.GetSubMenu(0));
		m_pImageView->SetHwndRcvMenuMsg(GetSafeHwnd());
		m_pImageView->ShowWindow(SW_SHOW);
	}

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CImageLabelDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CImageLabelDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CImageLabelDlg::OnBnClickedClose()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CImageLabelDlg::OnBnClickedOpenImage()
{
	// TODO: Add your control notification handler code here
	CString openFilter = _T("ͼ���ļ�(*.bmp *.jpg *jpeg)|*.bmp;*.jpg;*jpeg|All Files(*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, openFilter);
	BOOL bRet = dlg.DoModal();
	if (bRet != IDOK)
		return;

	CString pathName = dlg.GetPathName();
	std::string path = CStringToStdString(pathName);

	char driver[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath(path.c_str(), driver, dir, fname, ext);
	m_CurFolderName = std::string(driver) + std::string(dir);
	std::string filename = std::string(fname) + std::string(ext);
	m_CurFileName = filename;

	GetDlgItem(IDC_STATIC_CUR_FILE_NAME)->SetWindowText(CString(m_CurFileName.c_str()));

	cv::Mat image = cv::imread(path);

	if (image.empty())
		return;

	m_pImageView->SetSrcImage(image, path);

	while (m_ListFiles.GetCount())
		m_ListFiles.DeleteString(0);
	m_ListFiles.AddString(CString(filename.c_str()));
	m_CurFileIndex = 0;
}

void CImageLabelDlg::OnBnClickedOpenFolder()
{
	CFileFind finder;
	CString path;
	LPITEMIDLIST rootLocation;

	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &rootLocation);
	if (rootLocation == NULL)
		return; // error

	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.pidlRoot = rootLocation;
	bi.lpszTitle = _T("ѡ���ļ���");
	LPITEMIDLIST targetLocation = SHBrowseForFolder(&bi);
	if (targetLocation == NULL)
		return; // error

	TCHAR targetPath[MAX_PATH];
	SHGetPathFromIDList(targetLocation, targetPath);
	CString folderName = targetPath;
	// MessageBox(folderName);

	std::vector<CString> files;
	TraverseDirImage(folderName, files, false);
	// m_CurFolderName = CStringToStdString(folderName + _T("\\"));

	m_ListFiles.ResetContent();
	// assign to m_ListFiles
	for (size_t i = 0; i < files.size(); ++i)
	{
		int nPos = files[i].ReverseFind('.');
		CString extname = files[i].Right(files[i].GetLength() - nPos - 1);
		if (extname == _T("bmp") || 
			extname == _T("jpg") ||
			extname == _T("BMP") || 
			extname == _T("JPG"))
		{
			// get rid of directory, leave only file name
			char driver[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];
			std::string sfile = CStringToStdString(files[i]);
			_splitpath(sfile.c_str(), driver, dir, fname, ext);
			m_CurFolderName = std::string(driver) + std::string(dir);
			std::string filename = std::string(fname) + std::string(ext);
			m_ListFiles.AddString(CString(filename.c_str()));
			// m_ListFiles.AddString(files[i]);
		}
	}
	m_CurFileIndex = 0;
	// open first image
	if (files.size() >= 1)
	{
		CString cfile;
		m_ListFiles.GetText(0, cfile);
		std::string filename = CStringToStdString(cfile);
		m_CurFileName = filename;
		cv::Mat image = cv::imread(m_CurFolderName + m_CurFileName);
		if (image.empty())
			return;
		m_pImageView->SetSrcImage(image, m_CurFolderName + m_CurFileName);
		GetDlgItem(IDC_STATIC_CUR_FILE_NAME)->SetWindowText(CString(m_CurFileName.c_str()));
	}
	else
	{
		m_CurFileName = "";
	}
}

void CImageLabelDlg::OnBnClickedNextImage()
{
	if (m_pImageView->IsModified())
	{
		int nRet = MessageBox(_T("��ǰͼƬ��ע��δ���棬�Ƿ񱣴�?"),  _T("�����ע"), MB_OK | MB_OKCANCEL);
		if (nRet == IDCANCEL)
			return;
		else
			m_pImageView->SaveAnnotations(m_CurFolderName, m_CurFileName);
	}

	int numFiles = m_ListFiles.GetCount();
	if (m_CurFileIndex < numFiles - 1)
	{
		m_CurFileIndex += 1;
		// open file
		CString cfile;
		m_ListFiles.GetText(m_CurFileIndex, cfile);
		std::string filename = CStringToStdString(cfile);
		m_CurFileName = filename;
		cv::Mat image = cv::imread(m_CurFolderName + m_CurFileName);
		if (image.empty())
			return;
		m_pImageView->SetSrcImage(image, m_CurFolderName + m_CurFileName);
		GetDlgItem(IDC_STATIC_CUR_FILE_NAME)->SetWindowText(CString(m_CurFileName.c_str()));
	}
	else
	{
		MessageBox(_T("����һ��ͼƬ��"), _T("��һ��ͼƬ"));
	}
}

void CImageLabelDlg::OnBnClickedPrevImage()
{
	if (m_pImageView->IsModified())
	{
		int nRet = MessageBox(_T("��ǰͼƬ��ע��δ���棬�Ƿ񱣴�?"),  _T("�����ע"), MB_OK | MB_OKCANCEL);
		if (nRet == IDCANCEL)
			return;
		else
			m_pImageView->SaveAnnotations(m_CurFolderName, m_CurFileName);
		// m_pImageView->SaveAnnotations();
	}
	int numFiles = m_ListFiles.GetCount();
	if (m_CurFileIndex > 0)
	{
		m_CurFileIndex -= 1;
		// open file
		CString cfile;
		m_ListFiles.GetText(m_CurFileIndex, cfile);
		std::string filename = CStringToStdString(cfile);
		m_CurFileName = filename;
		cv::Mat image = cv::imread(m_CurFolderName + m_CurFileName);
		if (image.empty())
			return;
		m_pImageView->SetSrcImage(image, m_CurFolderName + m_CurFileName);
		GetDlgItem(IDC_STATIC_CUR_FILE_NAME)->SetWindowText(CString(m_CurFileName.c_str()));
	}
	else
	{
		MessageBox(_T("��ǰһ��ͼƬ��"), _T("��һ��ͼƬ"));
	}
}

void CImageLabelDlg::OnBnClickedSaveImageAnnot()
{
	m_pImageView->SaveAnnotations(m_CurFolderName, m_CurFileName);
}

std::string CImageLabelDlg::CStringToStdString(CString& str)
{
#ifdef _UNICODE
	int len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	char* temp = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str, -1, temp, len, NULL, NULL);
	std::string result(temp);
	delete[] temp;
	return result;
#else
	char* txt = str.GetBuffer(str.GetLength());
	std::string result(txt);
	str.ReleaseBuffer(0);
	return result;
#endif
}

void CImageLabelDlg::TraverseDirImage(const CString& dir, std::vector<CString>& files, bool bRecursive)
{
	CString dir2;
	if (dir == _T(""))
		return;
	else
		dir2 = dir + _T("\\*.*");

	CFileFind ff;
	BOOL ret = ff.FindFile(dir2);
	
	while (ret)
	{
		ret = ff.FindNextFile();
		if (ff.IsDirectory() || ff.IsDots())
			continue;
		files.push_back(ff.GetFilePath());
	}
	ff.Close();
}

CString CImageLabelDlg::GetStateString(MOUSE_MODE mode)
{
	if (mode == MOUSE_MODE_NORMAL)
		return _T("MOUSE_MODE_NORMAL");
	else if (mode == MOUSE_MODE_MOVE)
		return _T("MOUSE_MODE_MOVE");
	else if (mode == MOUSE_MODE_DRAW1)
		return _T("MOUSE_MODE_DRAW_OBJ");
	else if (mode == MOUSE_MODE_DRAW2)
		return _T("MOUSE_MODE_DRAW_FAULT");
	else if (mode == MOUSE_MODE_EDIT1)
		return _T("MOUSE_MODE_EDIT_OBJ");
	else if (mode == MOUSE_MODE_EDIT2)
		return _T("MOUSE_MODE_EDIT_FAULT");
	else if (mode == MOUSE_MODE_MOVE1)
		return _T("MOUSE_MODE_MOVE_OBJ");
	else if (mode == MOUSE_MODE_MOVE2)
		return _T("MOUSE_MODE_MOVE_FAULT");
	else
		return _T("MOUSE_MODE_NORMAL");
}

void CImageLabelDlg::OnMenuAddAnnot()
{
	m_pImageView->SetMouseMode(MOUSE_MODE_DRAW1);
	// SetState(m_pImageView->GetMouseMode());
}

void CImageLabelDlg::OnMenuAddSubAnnot()
{
	m_pImageView->SetMouseMode(MOUSE_MODE_DRAW2);
}

void CImageLabelDlg::OnMenuDelAnnot()
{
	m_pImageView->DelObject();
}

void CImageLabelDlg::OnMenuDelSubAnnot()
{
	m_pImageView->DelFaultFromObject();
}

void CImageLabelDlg::OnMenuEditAnnot()
{
	// m_pImageView->SetMouseMode(MOUSE_MODE_EDIT1);
	// �����Ի���
	m_pImageView->SetObjectName(0);
}

void CImageLabelDlg::OnMenuEditSubAnnot()
{
	// m_pImageView->SetMouseMode(MOUSE_MODE_EDIT2);
	m_pImageView->SetFaultName(0, 0);
}

void CImageLabelDlg::OnMenuCancel()
{
	m_pImageView->SetMouseMode(MOUSE_MODE_NORMAL);
}

LRESULT CImageLabelDlg::OnMsgRefreshStatus(WPARAM wParam, LPARAM lParam)
{
	double zoom = m_pImageView->GetZoomFactor();
	CString zoomString;
	zoomString.Format(_T("%.2lf"), zoom);
	CPoint ptWin;
	CPoint ptImage;
	m_pImageView->GetCursorPosOnWindow(ptImage, ptWin);
	
	MOUSE_MODE mode = m_pImageView->GetMouseMode();
	CString mouse_mode = GetStateString(mode);
	TCHAR buf[1024];
	if (m_pImageView->IsValidFile())
		wsprintf(buf, _T("%s\t Zoom: %s\t Point On Window(%d, %d)\t Point On Image(%d, %d)"), mouse_mode, zoomString, ptWin.x, ptWin.y, ptImage.x, ptImage.y);
	else
		wsprintf(buf, _T("%s"), mouse_mode);

	GetDlgItem(IDC_STATIC_STATE)->SetWindowText(buf);
	return 0;
}

void CImageLabelDlg::OnFileListDbClick()
{
	if (m_pImageView->IsModified())
	{
		int nRet = MessageBox(_T("��ǰͼƬ��ע��δ���棬�Ƿ񱣴�?"),  _T("�����ע"), MB_OK | MB_OKCANCEL);
		if (nRet == IDCANCEL)
			return;
		else
			m_pImageView->SaveAnnotations(m_CurFolderName, m_CurFileName);
		// m_pImageView->SaveAnnotations();
	}

	int index = m_ListFiles.GetCurSel();
	m_CurFileIndex = index;
	CString cfile;
	m_ListFiles.GetText(m_CurFileIndex, cfile);

	std::string filename = CStringToStdString(cfile);
	m_CurFileName = filename;
	cv::Mat image = cv::imread(m_CurFolderName + m_CurFileName);
	if (image.empty())
		return;
	m_pImageView->SetSrcImage(image, m_CurFolderName + m_CurFileName);
	GetDlgItem(IDC_STATIC_CUR_FILE_NAME)->SetWindowText(CString(m_CurFileName.c_str()));
}

BOOL CImageLabelDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}
