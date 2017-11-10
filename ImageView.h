#pragma once

#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <string>

#include "AnnotNameDlg.h"
#include "SubAnnotNameDlg.h"
#include "Annotation.h"

#define DEFAULT_WIDTH 960
#define DEFAULT_HEIGHT 700

#define MSG_IMAGEVIEW_MENU_INIT (WM_USER + 1600 + 16)
#define MSG_IMAGEVIEW_REFRESH_STATUS (WM_USER + 1600 + 17)

enum MOUSE_MODE
{
	MOUSE_MODE_NORMAL = 0,
	MOUSE_MODE_MOVE = 1,	// ͼ��ƽ��

	MOUSE_MODE_DRAW1 = 2,	// ��������ǣ������ǣ�
	MOUSE_MODE_DRAW2 = 3,	// �����ӱ�ǣ�ȱ�ݱ�ǣ�

	MOUSE_MODE_EDIT1 = 6,	// �༭�����
	MOUSE_MODE_EDIT2 = 7,	// �༭�ӱ��

	MOUSE_MODE_MOVE1 = 4,	// �ƶ������
	MOUSE_MODE_MOVE2 = 5,	// �ƶ��ӱ��
};

// CImageView
class CImageView sealed  : public CWnd
{
	DECLARE_DYNAMIC(CImageView)

public:
	CImageView();
	virtual ~CImageView();
	// ����ͼ��Դ
	void SetSrcImage(const cv::Mat& src, const std::string& filepath);
	
	void SetMouseMode(MOUSE_MODE mode) { m_MouseMode = mode; }
	MOUSE_MODE GetMouseMode() const { return m_MouseMode; }

	BOOL IsModified() const { return m_isModified; }
	BOOL IsValidFile() const { return m_isValidFile; }
	double GetZoomFactor() const { return m_dZoom; }

	// drawing functions;
	void DrawImage(CDC* pDC, BYTE* pBuf, int iW, int iH, CRect rc, CPoint move);
	void DrawImage2(CDC* pDC, BYTE* pBuf, int iW, int iH, CRect rc, CPoint move, double zoom);
	void DrawObjects(CDC* pDC);
	void DrawCornerIndicator(CDC* pDC);
	void DrawRuler(CDC* pDC);
	void DrawMask(CDC* pDC);

	// menu functions
	void SetRBtnMenu(CMenu* pMenu);
	void SetHwndRcvMenuMsg(HWND hWnd);

	// annotation functions
	void AddObject(const std::string& name, const cv::Rect& box);
	void AddFaultToObject(const std::string& name, const cv::Rect& box, int index);
	void DelObject();
	void DelFaultFromObject();
	void SaveAnnotations(const std::string& folderName, const std::string& fileName);
	void LoadAnnotations(const std::string& path);

	// open dlg and edit name
	void SetObjectName(int id);
	void SetFaultName(int id, int subid);

	// coordinate functions
	void CheckCoordinate();
	void CoordWinToImage(const CPoint& ptWin, CPoint& ptImage);
	void CoordImageToWin(const CPoint& ptImage, CPoint& ptWin);
	
	int PointInObject(const CPoint& pt);
	int PointInFault(const CPoint& pt, int index_obj);

	bool PointInObjectsAndFaults(const CPoint& pt, int& id_obj, int& id_fault);

	bool PointInRect(const CPoint& pt, const cv::Rect& rect);
	bool PointAroundCorner(const CPoint& pt, int& id, int& subid, CPoint& ptCorner, int& cornerId, int threshold = 3);
	// bool PointInAnnotation(const CPoint& pt, int& id, int& subid);

	bool GetCursorPosOnWindow(CPoint& ptImage, CPoint& ptWin);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

private:
	BOOL RegisterWindowClass();

	void CreateDefaultDisplayBuf();
	void CreateDisplayBufFromMat(const cv::Mat& image);
	std::string CStringToStdString(CString& str);
	void ClearInfo();

private:
	BYTE* m_pDisplayBuf;
	int m_iWidth;
	int m_iHeight;

	std::string m_Filename;
	std::string m_SaveFilename;
	
	BOOL m_isModified;
	BOOL m_isValidFile;

	BOOL	m_bLBtnDown;		// ���������±�־

	BOOL m_isDrawing;
	BOOL m_isDrawingSub;
	BOOL m_isEditing;
	BOOL m_isEditingSub;

	MOUSE_MODE m_MouseMode;

	CPoint m_DownPoint;
	CPoint m_CurPoint;

	CPoint m_MovePoint;	// ͼ�����Ͻ��ڴ����е�����
	double m_dZoom;		// ͼ����Գ�ʼ��С�ķŴ���
	double m_dZoomMax;
	double m_dZoomMin;

	CMenu* m_pRBtnMenu;
	HWND m_hWndRcvMenuMsg;

	// ��ע��Ϣ
	// std::vector<CAnnotation> m_Annotations;
	// CAnnotation* m_CurAnnotation;
	// FaultInfo* m_CurFaultInfo;
	
	Annotation2 m_Annotation;
	AnnotObject m_CurObject;
	Fault m_CurFault;

	int m_iSubAnnot;	// draw sub annot on this annot
	int m_iDelAnnot;	// index of annotation to be deleted.
	int m_iDelSubAnnot;	// index of sub annotation to be deleted.

	int m_iEditAnnot;
	int m_iEditSubAnnot;
	int m_iEditCornerId;

	int m_iCornerThresh;

private:
	CAnnotNameDlg* m_AnnotDlg;
	CSubAnnotNameDlg* m_SubAnnotDlg;
};
