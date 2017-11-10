
// ImageLabelDlg.h : ͷ�ļ�
//

#pragma once

#include "ImageView.h"
#include <string>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#define IDC_IMAGE_VIEW (WM_USER + 1000 + 1)

// CImageLabelDlg �Ի���
class CImageLabelDlg : public CDialogEx
{
// ����
public:
	CImageLabelDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_IMAGELABEL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedOpenImage();
	afx_msg void OnBnClickedSaveImageAnnot();
	afx_msg void OnBnClickedOpenFolder();
	afx_msg void OnBnClickedNextImage();
	afx_msg void OnBnClickedPrevImage();
	
	afx_msg void OnMenuAddAnnot();
	afx_msg void OnMenuAddSubAnnot();
	afx_msg void OnMenuDelAnnot();
	afx_msg void OnMenuDelSubAnnot();
	afx_msg void OnMenuEditAnnot();
	afx_msg void OnMenuEditSubAnnot();
	afx_msg void OnMenuCancel();

	afx_msg BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg LRESULT OnMsgRefreshStatus(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFileListDbClick();

private:
	std::string CStringToStdString(CString& str);
	void TraverseDirImage(const CString& dir, std::vector<CString>& files, bool bRecursive);
	CString GetStateString(MOUSE_MODE mode);
	// void ClearFileList();

private:
	CImageView* m_pImageView;
	CMenu m_RBtnMenu;

public:
	CListBox m_ListFiles;
	int m_CurFileIndex;
	CString m_WindowState;

	std::string m_CurFileName;
	std::string m_CurFolderName;
};
