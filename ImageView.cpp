// ImageView.cpp : implementation file
//

#include "stdafx.h"
#include "ImageLabel.h"
#include "ImageView.h"
#include "Annotation.h"
#include <io.h>
#include <exception>

// CImageView

IMPLEMENT_DYNAMIC(CImageView, CWnd)

CImageView::CImageView()
{
	m_pDisplayBuf = NULL;
	m_iWidth = 0;
	m_iHeight = 0;
	CreateDefaultDisplayBuf();
	RegisterWindowClass();
	m_isValidFile = FALSE;

	m_MovePoint = CPoint(0, 0);
	m_dZoom = 1.0;
	m_dZoomMax = 3.0;
	m_dZoomMin = 0.1;

	m_pRBtnMenu = NULL;
	m_hWndRcvMenuMsg = NULL;
	
	m_iCornerThresh = 8;

	m_AnnotDlg = NULL;
	m_SubAnnotDlg = NULL;
	ClearInfo();
}

CImageView::~CImageView()
{
	if (m_pDisplayBuf != 0)
		delete[] m_pDisplayBuf;

	if (m_AnnotDlg != NULL)
		delete m_AnnotDlg;
	if (m_SubAnnotDlg != NULL)
		delete m_SubAnnotDlg;
	/*
	if (m_CurObject != NULL)
		delete m_CurObject;
	if (m_CurFault != NULL)
		delete m_CurFault;
	*/
}

void CImageView::SetSrcImage(const cv::Mat& src, const std::string& filepath)
{
	CreateDisplayBufFromMat(src);

	double rW = m_iWidth * 1.0 / DEFAULT_WIDTH;
	double rH = m_iHeight * 1.0 / DEFAULT_HEIGHT;
	double r_small = (rW < rH) ? rW : rH;
	double r_large = (rW < rH) ? rH : rW;

	m_dZoom = 1.0 / r_small;
	m_isValidFile = TRUE;

	ClearInfo();

	m_Annotation.Clear();
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath(filepath.c_str(), drive, dir, fname, ext);
	m_Annotation.folder = std::string(drive) + std::string(dir);
	m_Annotation.filename = std::string(fname) + std::string(ext);
	m_Annotation.path = filepath;

	m_Annotation.width = src.cols;
	m_Annotation.height = src.rows;
	m_Annotation.depth = src.elemSize();

	// check if the corresponding xml file exists?
	std::string xmlpath = std::string(drive) + std::string(dir) + std::string(fname) + ".xml";
	FILE* fp = fopen(xmlpath.c_str(), "r");
	if (fp == NULL)
	{
		Invalidate(TRUE);
		return;
	}
	fclose(fp);
	try
	{
		LoadAnnotationFromXML(m_Annotation, xmlpath);
	}
	catch (std::exception&)
	{
		m_Annotation.objects.clear();
		MessageBox(_T("Load xml file error."));
	}
	
	Invalidate(TRUE);
}

BOOL CImageView::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, _T("IMAGE_VIEW"), &wndcls)))
	{
		// otherwise we need to register a new class

		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		wndcls.hbrBackground = (HBRUSH) (COLOR_3DFACE + 1);
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = _T("IMAGE_VIEW");

		if (!AfxRegisterClass(&wndcls))
		{
			AfxThrowResourceException();
			return FALSE;
		}
	}
	return TRUE;
}

BEGIN_MESSAGE_MAP(CImageView, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

void CImageView::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	CDC memDC;
	CBitmap bmp, *pBmp;

	GetWindowRect(&rect);
	CDC* pDC = &dc;
	memDC.CreateCompatibleDC(pDC);
	bmp.CreateBitmap(rect.Width(), rect.Height(), 1, 32, NULL);
	pBmp = memDC.SelectObject(&bmp);

	CheckCoordinate();
	DrawImage2(pDC, m_pDisplayBuf, m_iWidth, m_iHeight, rect, m_MovePoint, m_dZoom);
	DrawObjects(pDC);
	DrawCornerIndicator(pDC);
	DrawRuler(pDC);
	// DrawMask(pDC);

	ReleaseDC(pDC);
	memDC.SelectObject(pBmp);
	bmp.DeleteObject();
	memDC.DeleteDC();

	::PostMessageA(m_hWndRcvMenuMsg, MSG_IMAGEVIEW_REFRESH_STATUS, 0, (LPARAM)this);
}

void CImageView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_isValidFile)
		return;
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);
	SetFocus();
	if (point.x > rect.left && point.x < rect.right && point.y > rect.top && point.y < rect.bottom)
	{
		/*
		if (m_MouseMode == MOUSE_MODE_NORMAL && !(GetKeyState(VK_SPACE) & 0x0080))
		{
			// move image
			m_MouseMode = MOUSE_MODE_MOVE;
			m_bLBtnDown = TRUE;
			m_DownPoint = point;
			m_CurPoint = point;
			CheckCoordinate();
		}
		*/
		if (m_MouseMode == MOUSE_MODE_NORMAL)
		{
			// check if mouse is in box
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			CPoint ptImage;
			CoordWinToImage(pt, ptImage);

			// check edit
			int edit_id = -1, edit_subid = -1;
			CPoint edit_corner;
			int edit_cornerid;
			PointAroundCorner(pt, edit_id, edit_subid, edit_corner, edit_cornerid, m_iCornerThresh);
			if (edit_id >= 0 && edit_subid >= 0)
			{
				// edit sub annotation
				m_MouseMode = MOUSE_MODE_EDIT2;
				m_DownPoint = point;
				m_CurPoint = point;
				m_isEditingSub = TRUE;
				m_iEditAnnot = edit_id;
				m_iEditSubAnnot = edit_subid;
				m_iEditCornerId = edit_cornerid;
			}
			else if (edit_id >= 0 && edit_subid == -1)
			{
				m_MouseMode = MOUSE_MODE_EDIT1;
				m_DownPoint = point;
				m_CurPoint = point;
				m_isEditing = TRUE;
				m_iEditAnnot = edit_id;
				// m_iEditSubAnnot = edit_subid;
				m_iEditCornerId = edit_cornerid;
			}
			else
			{
				int id = -1, subid = -1;
				id = PointInObject(ptImage);
				if (id >= 0)
					subid = PointInFault(ptImage, id);

				if (id == -1 && subid == -1)
				{
					// do nothing
					// move whole image
					m_MouseMode = MOUSE_MODE_MOVE;
					m_bLBtnDown = TRUE;
					m_DownPoint = point;
					m_CurPoint = point;
					CheckCoordinate();
				}
				else if (id >= 0 && subid == -1)
				{
					// move annotation
					m_MouseMode = MOUSE_MODE_MOVE1;
					m_bLBtnDown = TRUE;
					m_DownPoint = point;
					m_CurPoint = point;
					m_iEditAnnot = id;
					// m_iEditSubAnnot = subid;
					CheckCoordinate();
				}
				else
				{
					// move subannotation
					m_MouseMode = MOUSE_MODE_MOVE2;
					m_bLBtnDown = TRUE;
					m_DownPoint = point;
					m_CurPoint = point;
					m_iEditAnnot = id;
					m_iEditSubAnnot = subid;
					CheckCoordinate();
				}
			}
		}
		else if (m_MouseMode == MOUSE_MODE_DRAW1 && m_isDrawing == FALSE)
		{
			m_isDrawing = TRUE;
			m_DownPoint = point;
			CPoint pt;
			CoordWinToImage(m_DownPoint, pt);
			cv::Rect rect(pt.x, pt.y, 0, 0);
			m_CurObject.name = "";
			m_CurObject.bndbox = rect;
			m_CurObject.valid = true;
		}
		else if (m_MouseMode == MOUSE_MODE_DRAW2 && m_isDrawingSub == FALSE)
		{
			m_isDrawingSub = TRUE;
			m_DownPoint = point;
			CPoint pt;
			CoordWinToImage(m_DownPoint, pt);
			cv::Rect rect(pt.x, pt.y, 0, 0);
			m_CurFault.name = "";
			m_CurFault.bndbox = rect;
			m_CurFault.valid = true;
		}
		else if (m_MouseMode == MOUSE_MODE_EDIT1 && m_isEditing == FALSE)
		{
			int id, subid, cornerid;
			CPoint ptCorner;
			bool bRet = PointAroundCorner(point, id, subid, ptCorner, cornerid, m_iCornerThresh);
			if (bRet && id >= 0 && subid == -1)
			{
				m_DownPoint = point;
				m_CurPoint = point;
				m_isEditing = TRUE;
				m_iEditAnnot = id;
				m_iEditCornerId = cornerid;
			}
			else
			{
				m_MouseMode = MOUSE_MODE_NORMAL;
			}
		}
		else if (m_MouseMode == MOUSE_MODE_EDIT2 && m_isEditingSub == FALSE)
		{
			int id, subid, cornerid;
			CPoint ptCorner;
			bool bRet = PointAroundCorner(point, id, subid, ptCorner, cornerid, m_iCornerThresh);
			if (bRet && id >= 0 && subid >= 0)
			{
				m_DownPoint = point;
				m_CurPoint = point;
				m_isEditingSub = TRUE;
				m_iEditAnnot = id;
				m_iEditSubAnnot = subid;
				m_iEditCornerId = cornerid;
			}
			else
			{
				m_MouseMode = MOUSE_MODE_NORMAL;
			}
		}
	}
}

void CImageView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_isValidFile)
		return;

	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);
	if (m_MouseMode == MOUSE_MODE_MOVE)
	{
		m_MouseMode = MOUSE_MODE_NORMAL;
		m_bLBtnDown = FALSE;
	}
	else if (m_MouseMode == MOUSE_MODE_MOVE1)
	{
		m_MouseMode = MOUSE_MODE_NORMAL;
		m_bLBtnDown = FALSE;
		m_iEditAnnot = -1;
		m_iEditSubAnnot = -1;
		m_iEditCornerId = -1;
	}
	else if (m_MouseMode == MOUSE_MODE_MOVE2)
	{
		m_MouseMode = MOUSE_MODE_NORMAL;
		m_bLBtnDown = FALSE;
		m_iEditAnnot = -1;
		m_iEditSubAnnot = -1;
		m_iEditCornerId = -1;
	}
	else if (m_MouseMode == MOUSE_MODE_DRAW1 && m_isDrawing == TRUE)
	{
		std::string poses[] = {
			 "Left", "Right", "Frontal", "Rear", "Top", "Bottom", "Unspecified"
		};

		if (m_AnnotDlg == NULL)
			m_AnnotDlg = new CAnnotNameDlg;
		m_AnnotDlg->SetDifficult(FALSE);
		m_AnnotDlg->SetPose(6);
		int nRes = m_AnnotDlg->DoModal();
		if (nRes == IDOK)
		{
			CString name = m_AnnotDlg->GetName();
			m_CurObject.name = CStringToStdString(name);
			BOOL d = m_AnnotDlg->GetDifficult();
			m_CurObject.difficult = ((d == TRUE) ? 1 : 0);
			int p = m_AnnotDlg->GetPose();
			if (p == 6)
				m_CurObject.pose = "Unspecified";
			else
				m_CurObject.pose = poses[p];

			m_Annotation.objects.push_back(m_CurObject);
			m_isModified = TRUE;
		}
		else
		{
			m_CurObject.valid = false;
		}

		/*
		CAnnotNameDlg dlg;
		int nRes = dlg.DoModal();
		if (nRes == IDOK)
		{
			CString name = dlg.GetName();
			m_CurObject.name = CStringToStdString(name);
			m_Annotation.objects.push_back(m_CurObject);
			m_isModified = TRUE;
		}
		else
		{
			m_CurObject.valid = false;
		}
		*/

		m_MouseMode = MOUSE_MODE_NORMAL;
		m_isDrawing = FALSE;
		
	}
	else if (m_MouseMode == MOUSE_MODE_DRAW2 && m_isDrawingSub == TRUE)
	{
		if (m_SubAnnotDlg == NULL)
			m_SubAnnotDlg = new CSubAnnotNameDlg;
		int nRes = m_SubAnnotDlg->DoModal();
		if (nRes == IDOK)
		{
			CString name = m_SubAnnotDlg->GetName();
			m_CurFault.name = CStringToStdString(name);
			m_Annotation.objects[m_iSubAnnot].faults.push_back(m_CurFault);
			m_isModified = TRUE;
		}
		else
		{
			m_CurFault.valid = false;
		}

		/*
		CSubAnnotNameDlg dlg;
		int nRes = dlg.DoModal();
		if (nRes == IDOK)
		{
			CString name = dlg.GetName();
			m_CurFault.name = CStringToStdString(name);
			m_Annotation.objects[m_iSubAnnot].faults.push_back(m_CurFault);
			m_isModified = TRUE;
		}
		else
		{
			m_CurFault.valid = false;
		}
		*/
		m_MouseMode = MOUSE_MODE_NORMAL;
		m_isDrawingSub = FALSE;
	}
	else if (m_MouseMode == MOUSE_MODE_EDIT1 && m_isEditing == TRUE)
	{
		m_MouseMode = MOUSE_MODE_NORMAL;
		m_isEditing = FALSE;
		m_iEditAnnot = -1;
		m_iEditSubAnnot = -1;
		m_iEditCornerId = -1;
	}
	else if (m_MouseMode == MOUSE_MODE_EDIT2 && m_isEditingSub == TRUE)
	{
		m_MouseMode = MOUSE_MODE_NORMAL;
		m_isEditingSub = FALSE;
		m_iEditAnnot = -1;
		m_iEditSubAnnot = -1;
		m_iEditCornerId = -1;
	}
	else
	{
		m_MouseMode = MOUSE_MODE_NORMAL;
	}
}

void CImageView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_isValidFile)
		return;
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);

	// SetFocus();

	if (m_MouseMode == MOUSE_MODE_MOVE && m_bLBtnDown == TRUE)
	// if (m_MouseMode == MOUSE_MODE_MOVE && m_bLBtnDown == TRUE && (GetKeyState(VK_SPACE) & 0x0080))
	{
		// SetCursor(m_hHand);
		m_MovePoint.x += point.x - m_CurPoint.x;
		m_MovePoint.y += point.y - m_CurPoint.y;
		m_CurPoint = point;
		CheckCoordinate();
	}
	// else if (m_MouseMode == MOUSE_MODE_MOVE1)
	else if (m_MouseMode == MOUSE_MODE_MOVE1 && (GetKeyState(VK_SPACE) & 0x0080))
	{
		CPoint pt1, center;
		CoordWinToImage(point, pt1);
		CoordWinToImage(m_CurPoint, center);
		m_CurPoint = point;
		int dx = pt1.x - center.x;
		int dy = pt1.y - center.y;
		cv::Rect rect = m_Annotation.objects[m_iEditAnnot].bndbox;
		rect.x += dx;
		rect.y += dy;
		m_Annotation.objects[m_iEditAnnot].bndbox = rect;
	}
	// else if (m_MouseMode == MOUSE_MODE_MOVE2)
	else if (m_MouseMode == MOUSE_MODE_MOVE2 && (GetKeyState(VK_SPACE) & 0x0080))
	{
		CPoint pt1, center;
		CoordWinToImage(point, pt1);
		CoordWinToImage(m_CurPoint, center);
		m_CurPoint = point;
		int dx = pt1.x - center.x;
		int dy = pt1.y - center.y;
		cv::Rect rect = m_Annotation.objects[m_iEditAnnot].faults[m_iEditSubAnnot].bndbox;
		rect.x += dx;
		rect.y += dy;
		m_Annotation.objects[m_iEditAnnot].faults[m_iEditSubAnnot].bndbox = rect;
	}
	else if (m_MouseMode == MOUSE_MODE_DRAW1 && m_isDrawing)
	{
		int x1 = m_DownPoint.x;
		int y1 = m_DownPoint.y;
		int x2 = point.x;
		int y2 = point.y;

		int x = (x1 < x2) ? x1 : x2;
		int y = (y1 < y2) ? y1 : y2;
		int width = abs(x2 - x1);
		int height = abs(y2 - y1);

		CPoint pt1(x, y);
		CPoint pt2(x + width, y + height);
		CPoint pti1;
		CPoint pti2;
		CoordWinToImage(pt1, pti1);
		CoordWinToImage(pt2, pti2);
		x = pti1.x;
		y = pti1.y;
		width = pti2.x - pti1.x;
		height = pti2.y - pti1.y;

		m_CurObject.bndbox = cv::Rect(x, y, width, height);
	}
	else if (m_MouseMode == MOUSE_MODE_DRAW2 && m_isDrawingSub)
	{
		int x1 = m_DownPoint.x;
		int y1 = m_DownPoint.y;
		int x2 = point.x;
		int y2 = point.y;

		int x = (x1 < x2) ? x1 : x2;
		int y = (y1 < y2) ? y1 : y2;
		int width = abs(x2 - x1);
		int height = abs(y2 - y1);

		CPoint pt1(x, y);
		CPoint pt2(x + width, y + height);
		CPoint pti1;
		CPoint pti2;
		CoordWinToImage(pt1, pti1);
		CoordWinToImage(pt2, pti2);
		x = pti1.x;
		y = pti1.y;
		width = pti2.x - pti1.x;
		height = pti2.y - pti1.y;

		m_CurFault.bndbox = cv::Rect(x, y, width, height);
	}
	else if (m_MouseMode == MOUSE_MODE_EDIT1 && m_isEditing)
	{
		if (m_iEditAnnot >= 0 && m_iEditAnnot < int(m_Annotation.objects.size()))
		{
			cv::Rect rect = m_Annotation.objects[m_iEditAnnot].bndbox;
			CPoint pt1, center;
			CoordWinToImage(point, pt1);
			CoordWinToImage(m_CurPoint, center);
			m_CurPoint = point;
			int dx = pt1.x - center.x;
			int dy = pt1.y - center.y;
			cv::Rect new_rect = rect;
			if (m_iEditCornerId == 0)
				new_rect = cv::Rect(rect.x + dx, rect.y + dy, rect.width - dx, rect.height - dy);
			else if (m_iEditCornerId == 1)
				new_rect = cv::Rect(rect.x, rect.y + dy, rect.width + dx, rect.height - dy);
			else if (m_iEditCornerId == 2)
				new_rect = cv::Rect(rect.x, rect.y, rect.width + dx, rect.height + dy);
			else
				new_rect = cv::Rect(rect.x + dx, rect.y, rect.width - dx, rect.height + dy);
			m_Annotation.objects[m_iEditAnnot].bndbox = new_rect;
		}
	}
	else if (m_MouseMode == MOUSE_MODE_EDIT2 && m_isEditingSub)
	{
		if (m_iEditAnnot >= 0 && m_iEditAnnot < int(m_Annotation.objects.size()) &&
			m_iEditSubAnnot >= 0 && m_iEditSubAnnot < int(m_Annotation.objects[m_iEditAnnot].faults.size()))
		{
			cv::Rect rect = m_Annotation.objects[m_iEditAnnot].faults[m_iEditSubAnnot].bndbox;
			CPoint pt1, center;
			CoordWinToImage(point, pt1);
			CoordWinToImage(m_CurPoint, center);
			m_CurPoint = point;
			int dx = pt1.x - center.x;
			int dy = pt1.y - center.y;
			cv::Rect new_rect = rect;
			if (m_iEditCornerId == 0)
				new_rect = cv::Rect(rect.x + dx, rect.y + dy, rect.width - dx, rect.height - dy);
			else if (m_iEditCornerId == 1)
				new_rect = cv::Rect(rect.x, rect.y + dy, rect.width + dx, rect.height - dy);
			else if (m_iEditCornerId == 2)
				new_rect = cv::Rect(rect.x, rect.y, rect.width + dx, rect.height + dy);
			else
				new_rect = cv::Rect(rect.x + dx, rect.y, rect.width - dx, rect.height + dy);
			// m_Annotations[m_iEditAnnot].SetFaultBox(m_iEditSubAnnot, new_rect);
			m_Annotation.objects[m_iEditAnnot].faults[m_iEditSubAnnot].bndbox = new_rect;
		}
	}

	UpdateWindow();
	Invalidate(FALSE);
	CWnd::OnMouseMove(nFlags, point);
}

void CImageView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (!m_isValidFile)
		return;

	if (m_pRBtnMenu != NULL)
	{
		if (m_MouseMode == MOUSE_MODE_NORMAL)
			m_pRBtnMenu->EnableMenuItem(ID_MENU_CANCEL, MF_DISABLED);
		else
			m_pRBtnMenu->EnableMenuItem(ID_MENU_CANCEL, MF_ENABLED);

		CPoint point1;
		GetCursorPos(&point1);

		CPoint point2 = point1;
		ScreenToClient(&point2);
		CPoint pt;
		CoordWinToImage(point2, pt);
		int id = PointInObject(pt);
		if (id == -1)
		{
			m_pRBtnMenu->EnableMenuItem(ID_MENU_ADD_ANNOT, MF_ENABLED);
			m_pRBtnMenu->EnableMenuItem(ID_MENU_ADD_SUB_ANNOT, MF_DISABLED);
			m_pRBtnMenu->EnableMenuItem(ID_MENU_DEL_ANNOT, MF_DISABLED);
			m_pRBtnMenu->EnableMenuItem(ID_MENU_DEL_SUB_ANNOT, MF_DISABLED);
			m_pRBtnMenu->EnableMenuItem(ID_MENU_EDIT_ANNOT, MF_DISABLED);
			m_pRBtnMenu->EnableMenuItem(ID_MENU_EDIT_SUB_ANNOT, MF_DISABLED);
		}
		else
		{
			m_pRBtnMenu->EnableMenuItem(ID_MENU_ADD_ANNOT, MF_ENABLED);
			m_pRBtnMenu->EnableMenuItem(ID_MENU_ADD_SUB_ANNOT, MF_ENABLED);
			m_pRBtnMenu->EnableMenuItem(ID_MENU_DEL_ANNOT, MF_ENABLED);
			// m_pRBtnMenu->EnableMenuItem(ID_MENU_DEL_SUB_ANNOT, MF_DISABLED);
			m_pRBtnMenu->EnableMenuItem(ID_MENU_EDIT_ANNOT, MF_ENABLED);
			// m_pRBtnMenu->EnableMenuItem(ID_MENU_EDIT_SUB_ANNOT, MF_DISABLED);
			
			m_iSubAnnot = id;
			m_iDelAnnot = id;

			m_iEditAnnot = id;
			int id_sub = PointInFault(pt, id);
			if (id_sub == -1)
			{
				m_pRBtnMenu->EnableMenuItem(ID_MENU_DEL_SUB_ANNOT, MF_DISABLED);
				m_pRBtnMenu->EnableMenuItem(ID_MENU_EDIT_SUB_ANNOT, MF_DISABLED);
			}
			else
			{
				m_iEditSubAnnot = id_sub;
				m_pRBtnMenu->EnableMenuItem(ID_MENU_DEL_SUB_ANNOT, MF_ENABLED);
				m_pRBtnMenu->EnableMenuItem(ID_MENU_EDIT_SUB_ANNOT, MF_ENABLED);
				m_iDelSubAnnot = id_sub;
			}
			// check if any sub annotation can be deleted
		}
		SetFocus();
		// ::PostMessageA(m_hWndRcvMenuMsg, MSG_IMAGEVIEW_MENU_INIT, 0, (LPARAM)this);
		m_pRBtnMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, GetParent(), NULL);
	}
}

BOOL CImageView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (!m_isValidFile)
		return CWnd::OnMouseWheel(nFlags, zDelta, pt);

	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		CPoint pt;
		GetCursorPos(&pt);
		CPoint ptImage;
		CoordWinToImage(pt, ptImage);
		double z1 = m_dZoom;
		if (zDelta > 0)
			m_dZoom = m_dZoom * 1.2;
		else if (zDelta < 0)
			m_dZoom = m_dZoom / 1.2;

		if (m_dZoom > m_dZoomMax)
			m_dZoom = m_dZoomMax;
		else if (m_dZoom < m_dZoomMin)
			m_dZoom = m_dZoomMin;

		double z2 = m_dZoom;

		int dx = int((z1 - z2) * ptImage.x);
		int dy = int((z1 - z2) * ptImage.y);
		m_MovePoint.x += dx;
		m_MovePoint.y += dy;

		CheckCoordinate();
		Invalidate(TRUE);
	}
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CImageView::DrawImage(CDC* pDC, BYTE* pBuf, int iW, int iH, CRect rc, CPoint move)
{
	CDC imgDC;
	imgDC.CreateCompatibleDC(pDC);
	CBitmap imgBmp, *pImgBmp;
	imgBmp.CreateBitmap(iW, iH, 4, 8, pBuf);
	pImgBmp = imgDC.SelectObject(&imgBmp);

	int iLeft, iTop, iWidth, iHeight;
	int iLeft1, iTop1, iWidth1, iHeight1;

	if (iW <= rc.Width())
	{
		// x方向全部显示
		iLeft = 0;
		iWidth = iW;
		iLeft1 = (rc.Width() - iW) / 2;
		iWidth1 = iW;
	}
	else
	{
		int move_x;
		if (move.x < -(iW - rc.Width()))
			move_x = -(iW - rc.Width());
		else
			move_x = MIN(move.x, 0);
		iLeft = -move_x;
		iWidth = rc.Width();
		iLeft1 = 0;
		iWidth1 = rc.Width();
	}

	if (iH <= rc.Height())
	{
		// y方向全部显示
		iTop = 0;
		iHeight = iH;
		iTop1 = (rc.Height() - iH) / 2;
		iHeight1 = iH;
	}
	else
	{
		int move_y;
		if (move.y < -(iH - rc.Width()))
			move_y = -(iH - rc.Width());
		else
			move_y = MIN(move.y, 0);
		iTop = -move_y;
		iHeight = rc.Height();
		iTop1 = 0;
		iHeight1 = rc.Height();
	}

	int sbMode = pDC->SetStretchBltMode(HALFTONE);
	pDC->StretchBlt(iLeft1, iTop1, iWidth1, iHeight1, &imgDC, iLeft, iTop, iWidth, iHeight, SRCCOPY);
	pDC->SetStretchBltMode(sbMode);

	imgDC.SelectObject(pImgBmp);
	imgDC.DeleteDC();
	imgBmp.DeleteObject();
}

void CImageView::DrawImage2(CDC* pDC, BYTE* pBuf, int iW, int iH, CRect rc, CPoint move, double zoom)
{
	CDC imgDC;
	imgDC.CreateCompatibleDC(pDC);
	CBitmap imgBmp, *pImgBmp;
	imgBmp.CreateBitmap(iW, iH, 4, 8, pBuf);
	pImgBmp = imgDC.SelectObject(&imgBmp);

	int iW1 = int(iW * zoom);
	int iH1 = int(iH * zoom);
	int iLeft, iTop, iWidth, iHeight;		// source
	int iLeft1, iTop1, iWidth1, iHeight1;	// destination
	if (iW1 < rc.Width())
	{
		// x方向全部显示
		iLeft = 0;
		iWidth = iW;
		iLeft1 = (rc.Width() - iW1) / 2;
		iWidth1 = iW1;
	}
	else
	{
		int move_x;
		if (move.x < -(iW1 - rc.Width()))
			move_x = -(iW1 - rc.Width());
		else
			move_x = MIN(move.x, 0);
		iLeft = -int(move_x / zoom);
		iWidth = int(rc.Width() / zoom);
		iLeft1 = 0;
		iWidth1 = rc.Width();
	}

	if (iH1 < rc.Height())
	{
		iTop = 0;
		iHeight = iH;
		iTop1 = (rc.Height() - iH1) / 2;
		iHeight1 = iH1;
	}
	else
	{
		int move_y;
		if (move.y < -(iH1 - rc.Height()))
			move_y = -(iH1 - rc.Height());
		else
			move_y = MIN(move.y, 0);
		iTop = -int(move_y / zoom);
		iHeight = int(rc.Height() / zoom);
		iTop1 = 0;
		iHeight1 = rc.Height();
	}

	int sbMode = pDC->SetStretchBltMode(HALFTONE);
	pDC->StretchBlt(iLeft1, iTop1, iWidth1, iHeight1, &imgDC, iLeft, iTop, iWidth, iHeight, SRCCOPY);
	pDC->SetStretchBltMode(sbMode);
	// pDC->BitBlt(0, 0, rc.Width(), rc.Height(), &imgDC, 0, 0, SRCCOPY);

	imgDC.SelectObject(pImgBmp);
	imgDC.DeleteDC();
	imgBmp.DeleteObject();
}

void CImageView::DrawObjects(CDC* pDC)
{
	CPen green_pen, red_pen;
	CPen* pPen;
	green_pen.CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
	red_pen.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	pPen = pDC->SelectObject(&green_pen);

	CheckCoordinate();
	// draw saved objects
	for (size_t i = 0; i < m_Annotation.objects.size(); ++i)
	{
		pDC->SelectObject(green_pen);
		cv::Rect box = m_Annotation.objects[i].bndbox;
		int x1 = box.x;
		int x2 = box.x + box.width;
		int y1 = box.y;
		int y2 = box.y + box.height;

		CPoint pt;
		CoordImageToWin(CPoint(x1, y1), pt);
		pDC->MoveTo(pt);
		CoordImageToWin(CPoint(x2, y1), pt);
		pDC->LineTo(pt);
		CoordImageToWin(CPoint(x2, y2), pt);
		pDC->LineTo(pt);
		CoordImageToWin(CPoint(x1, y2), pt);
		pDC->LineTo(pt);
		CoordImageToWin(CPoint(x1, y1), pt);
		pDC->LineTo(pt);

		// draw faults for the object.
		pDC->SelectObject(red_pen);
		for (size_t k = 0; k < m_Annotation.objects[i].faults.size(); ++k)
		{
			cv::Rect box = m_Annotation.objects[i].faults[k].bndbox;
			int x1 = box.x;
			int x2 = box.x + box.width;
			int y1 = box.y;
			int y2 = box.y + box.height;
			CPoint pt;
			CoordImageToWin(CPoint(x1, y1), pt);
			pDC->MoveTo(pt);
			CoordImageToWin(CPoint(x2, y1), pt);
			pDC->LineTo(pt);
			CoordImageToWin(CPoint(x2, y2), pt);
			pDC->LineTo(pt);
			CoordImageToWin(CPoint(x1, y2), pt);
			pDC->LineTo(pt);
			CoordImageToWin(CPoint(x1, y1), pt);
			pDC->LineTo(pt);
		}
	}

	// draw object labels
	int sbMode = pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(0, 0, 255));
	for (size_t i = 0; i < m_Annotation.objects.size(); ++i)
	{
		cv::Rect box = m_Annotation.objects[i].bndbox;
		int x1 = box.x;
		int x2 = box.x + box.width;
		int y1 = box.y;
		int y2 = box.y + box.height;
		CPoint pt;
		CoordImageToWin(CPoint(x1, y1), pt);
		pDC->TextOutW(pt.x, pt.y, CString(m_Annotation.objects[i].name.c_str()));
	}
	pDC->SetBkMode(sbMode);

	// draw current annotation
	if (m_CurObject.valid && m_isDrawing)
	{
		pDC->SelectObject(green_pen);
		cv::Rect box1 = m_CurObject.bndbox;
		int x1 = box1.x;
		int x2 = box1.x + box1.width;
		int y1 = box1.y;
		int y2 = box1.y + box1.height;
		CPoint pt;
		CoordImageToWin(CPoint(x1, y1), pt);
		pDC->MoveTo(pt);
		CoordImageToWin(CPoint(x2, y1), pt);
		pDC->LineTo(pt);
		CoordImageToWin(CPoint(x2, y2), pt);
		pDC->LineTo(pt);
		CoordImageToWin(CPoint(x1, y2), pt);
		pDC->LineTo(pt);
		CoordImageToWin(CPoint(x1, y1), pt);
		pDC->LineTo(pt);
	}

	// draw current fault info
	if (m_CurFault.valid && m_isDrawingSub)
	{
		pDC->SelectObject(&red_pen);
		cv::Rect box1 = m_CurFault.bndbox;
		int x1 = box1.x;
		int x2 = box1.x + box1.width;
		int y1 = box1.y;
		int y2 = box1.y + box1.height;
		CPoint pt;
		CoordImageToWin(CPoint(x1, y1), pt);
		pDC->MoveTo(pt);
		CoordImageToWin(CPoint(x2, y1), pt);
		pDC->LineTo(pt);
		CoordImageToWin(CPoint(x2, y2), pt);
		pDC->LineTo(pt);
		CoordImageToWin(CPoint(x1, y2), pt);
		pDC->LineTo(pt);
		CoordImageToWin(CPoint(x1, y1), pt);
		pDC->LineTo(pt);
	}
}

void CImageView::DrawCornerIndicator(CDC* pDC)
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	CPoint corner_point;
	int obj_id, fault_id, corner_id;

	bool bRet = PointAroundCorner(pt, obj_id, fault_id, corner_point, corner_id, m_iCornerThresh);
	if (bRet)
	{
		CBrush brush, *pBrush;
		brush.CreateSolidBrush(RGB(0, 0, 255));
		pBrush = pDC->SelectObject(&brush);
		CPen pen, *pPen;
		pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		pPen = pDC->SelectObject(&pen);
		pDC->Rectangle(corner_point.x - m_iCornerThresh, corner_point.y - m_iCornerThresh, 
			corner_point.x + m_iCornerThresh, corner_point.y + m_iCornerThresh);
		pDC->SelectObject(pBrush);
		pDC->SelectObject(pPen);
	}
}

void CImageView::DrawRuler(CDC* pDC)
{
	if (m_MouseMode == MOUSE_MODE_DRAW1 || m_MouseMode == MOUSE_MODE_DRAW2)
	{
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		CRect rect;
		GetClientRect(&rect);

		CPen pen, *pPen;
		pen.CreatePen(PS_DASH, 1, RGB(0, 0, 0));
		pPen = pDC->SelectObject(&pen);
		
		// only draw on image
		CPoint ptLT(0, 0);
		CPoint ptRB(m_iWidth, m_iHeight);
		CPoint ptWinLT, ptWinRB;
		CoordImageToWin(ptLT, ptWinLT);
		CoordImageToWin(ptRB, ptWinRB);
		int left = (ptWinLT.x > 0) ? ptWinLT.x : 0;
		int right = (ptWinRB.x < rect.Width()) ? ptWinRB.x : rect.Width();
		int top = (ptWinLT.y > 0) ? ptWinLT.y : 0;
		int bottom = (ptWinRB.y < rect.Height()) ? ptWinRB.y : rect.Height();
		pDC->MoveTo(left, pt.y);
		pDC->LineTo(right, pt.y);
		pDC->MoveTo(pt.x, top);
		pDC->LineTo(pt.x, bottom);
		/*
		pDC->MoveTo(0, pt.y);
		pDC->LineTo(rect.Width(), pt.y);
		pDC->MoveTo(pt.x, 0);
		pDC->LineTo(pt.x, rect.Height());
		*/
		pDC->SelectObject(pPen);
	}
}

void CImageView::DrawMask(CDC* pDC)
{
	if (!(GetKeyState(VK_SPACE) & 0x0080))
	{
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		int obj_id = -1;
		int fault_id = -1;
		PointInObjectsAndFaults(pt, obj_id, fault_id);
		if (obj_id >= 0 && fault_id == -1)
		{
			// draw object region
			cv::Rect rect = m_Annotation.objects[obj_id].bndbox;
			CPoint pt1, pt2;
			pt1.x = rect.x; pt1.y = rect.y;
			pt2.x = rect.x + rect.width, pt2.y = rect.y + rect.height;
			CPoint pt1Win, pt2Win;
			CoordImageToWin(pt1, pt1Win);
			CoordImageToWin(pt2, pt2Win);
			CBrush brush(HS_BDIAGONAL, RGB(128, 255, 255));
			CBrush* pBrush = pDC->SelectObject(&brush);
			pDC->Rectangle(pt1Win.x, pt1Win.y, pt2Win.x, pt2Win.y);
			pDC->SelectObject(pBrush);
		}
		else if (obj_id >= 0 && fault_id >= 0)
		{
			// draw fault region

		}
	}
}

void CImageView::CreateDefaultDisplayBuf()
{
	m_iWidth = DEFAULT_WIDTH;
	m_iHeight = DEFAULT_HEIGHT;
	if (m_pDisplayBuf != NULL)
		delete[] m_pDisplayBuf;

	m_pDisplayBuf = new BYTE[m_iWidth * m_iHeight * 4];
	for (int j = 0; j < m_iHeight; ++j)
	{
		int iLine = j * m_iWidth;
		for (int i = 0; i < m_iWidth; ++i)
		{
			int index = iLine + i;
			m_pDisplayBuf[index * 4 + 0] = (i + j) % 255;
			m_pDisplayBuf[index * 4 + 1] = (i + j) % 255;
			m_pDisplayBuf[index * 4 + 2] = (i + j) % 255;
			m_pDisplayBuf[index * 4 + 3] = (i + j) % 255;
		}
	}
}

void CImageView::CreateDisplayBufFromMat(const cv::Mat& image)
{
	if (m_pDisplayBuf != NULL)
		delete[] m_pDisplayBuf;
	m_pDisplayBuf = NULL;

	m_iWidth = image.cols;
	m_iHeight = image.rows;
	m_pDisplayBuf = new BYTE[m_iWidth * m_iHeight * 4];

	int d = image.elemSize();

	for (int j = 0; j < m_iHeight; ++j)
	{
		const unsigned char* row = image.ptr<unsigned char>(j);
		for (int i = 0; i < m_iWidth; ++i)
		{
			if (d == 1)
			{
				m_pDisplayBuf[(j * m_iWidth + i) * 4 + 0] = row[i];
				m_pDisplayBuf[(j * m_iWidth + i) * 4 + 1] = row[i];
				m_pDisplayBuf[(j * m_iWidth + i) * 4 + 2] = row[i];
				m_pDisplayBuf[(j * m_iWidth + i) * 4 + 3] = 255;
			}
			else
			{
				m_pDisplayBuf[(j * m_iWidth + i) * 4 + 0] = row[i * d];
				m_pDisplayBuf[(j * m_iWidth + i) * 4 + 1] = row[i * d + 1];
				m_pDisplayBuf[(j * m_iWidth + i) * 4 + 2] = row[i * d + 2];
				m_pDisplayBuf[(j * m_iWidth + i) * 4 + 3] = 255;
			}
		}
	}
}

void CImageView::SetRBtnMenu(CMenu* pMenu)
{
	m_pRBtnMenu = pMenu;
}

void CImageView::SetHwndRcvMenuMsg(HWND hWnd)
{
	m_hWndRcvMenuMsg = hWnd;
}

void CImageView::AddObject(const std::string& name, const cv::Rect& box)
{
	AnnotObject obj;
	obj.name = name;
	obj.bndbox = box;
	m_Annotation.objects.push_back(obj);
	m_isModified = TRUE;
}

void CImageView::AddFaultToObject(const std::string& name, const cv::Rect& box, int index)
{
	size_t num = m_Annotation.objects.size();
	if (index < 0 || index >= int(num))
		return;

	Fault new_fault(name, box);
	m_Annotation.objects[index].faults.push_back(new_fault);
	m_isModified = TRUE;
}

void CImageView::DelObject()
{
	int index_obj = m_iDelAnnot;
	int num_obj = int(m_Annotation.objects.size());
	if (index_obj < 0 || index_obj >= num_obj)
		return;

	std::vector<AnnotObject>::iterator it = m_Annotation.objects.begin();
	for (int i = 0; i < index_obj; ++i)
		it++;
	m_Annotation.objects.erase(it);
	m_isModified = TRUE;
}

void CImageView::DelFaultFromObject()
{
	int index_obj = m_iDelAnnot;
	if (index_obj < 0 || index_obj >= int(m_Annotation.objects.size()))
		return;
	std::vector<Fault>::iterator it = m_Annotation.objects[index_obj].faults.begin();
	int index_fault = m_iDelSubAnnot;
	for (int i = 0; i < index_fault; ++i)
		it++;
	m_Annotation.objects[index_obj].faults.erase(it);
	m_isModified = TRUE;
}

void CImageView::CheckCoordinate()
{
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);

	int iW1 = int(m_iWidth * m_dZoom);
	int iH1 = int(m_iHeight * m_dZoom);

	if (iW1 < rect.Width())
	{
		m_MovePoint.x = (rect.Width() - iW1) / 2;
	}
	else
	{
		if (m_MovePoint.x > 0)
			m_MovePoint.x = 0;
		else if (m_MovePoint.x < -(iW1 - rect.Width()))
			m_MovePoint.x = -(iW1 - rect.Width());
	}

	if (iH1 < rect.Height())
	{
		m_MovePoint.y = (rect.Height() - iH1) / 2;
	}
	else
	{
		if (m_MovePoint.y > 0)
			m_MovePoint.y = 0;
		else if (m_MovePoint.y < -(iH1 - rect.Height()))
			m_MovePoint.y = -(iH1 - rect.Height());
	}
}

void CImageView::CoordImageToWin(const CPoint& ptImage, CPoint& ptWin)
{
	ptWin.x = int(ptImage.x * m_dZoom + m_MovePoint.x);
	ptWin.y = int(ptImage.y * m_dZoom + m_MovePoint.y);
}

void CImageView::CoordWinToImage(const CPoint& ptWin, CPoint& ptImage)
{
	ptImage.x = int((ptWin.x - m_MovePoint.x) / m_dZoom);
	ptImage.y = int((ptWin.y - m_MovePoint.y) / m_dZoom);
}

bool CImageView::PointInRect(const CPoint& pt, const cv::Rect& rect)
{
	if (pt.x > rect.x && pt.x < rect.x + rect.width &&
		pt.y > rect.y && pt.y < rect.y + rect.height)
		return true;
	else
		return false;
}

int CImageView::PointInObject(const CPoint& pt)
{
	for (size_t i = 0; i < m_Annotation.objects.size(); ++i)
	{
		if (PointInRect(pt, m_Annotation.objects[i].bndbox))
			return i;
	}
	return -1;
}

int CImageView::PointInFault(const CPoint& pt, int index_obj)
{
	if (index_obj < 0 || index_obj >= int(m_Annotation.objects.size()))
		return -1;
	for (size_t i = 0; i < m_Annotation.objects[index_obj].faults.size(); ++i)
	{
		if (PointInRect(pt, m_Annotation.objects[index_obj].faults[i].bndbox))
			return i;
	}
	return -1;
}

bool CImageView::PointInObjectsAndFaults(const CPoint& pt, int& id_obj, int& id_fault)
{
	for (size_t i = 0; i < m_Annotation.objects.size(); ++i)
	{
		cv::Rect box_obj = m_Annotation.objects[i].bndbox;
		if (PointInRect(pt, box_obj))
		{
			id_obj = i;
			// check in faults
			for (size_t k = 0; k < m_Annotation.objects[i].faults.size(); ++k)
			{
				cv::Rect fault_box = m_Annotation.objects[i].faults[k].bndbox;
				if (PointInRect(pt, fault_box))
				{
					id_fault = k;
				}
			}
			return true;
		}
	}
	return false;
}

bool CImageView::PointAroundCorner(const CPoint& pt, int& id, int& subid, CPoint& ptCorner, int& cornerId, int threshold)
{
	for (size_t i = 0; i < m_Annotation.objects.size(); ++i)
	{
		// check annotation
		cv::Rect rect = m_Annotation.objects[i].bndbox;
		CPoint corners[4];
		corners[0].x = rect.x; corners[0].y = rect.y;
		corners[1].x = rect.x + rect.width; corners[1].y = rect.y;
		corners[2].x = rect.x + rect.width; corners[2].y = rect.y + rect.height;
		corners[3].x = rect.x; corners[3].y = rect.y + rect.height;

		for (int k = 0; k < 4; ++k)
		{
			CPoint cInWin;
			CoordImageToWin(corners[k], cInWin);
			int dx = abs(pt.x - cInWin.x);
			int dy = abs(pt.y - cInWin.y);
			if (dx <= threshold && dy <= threshold)
			{
				id = i;
				subid = -1;
				ptCorner = cInWin;
				cornerId = k;
				return true;
			}
		}
		// check sub annotation

		for (size_t j = 0; j < m_Annotation.objects[i].faults.size(); ++j)
		{
			cv::Rect sub_rect = m_Annotation.objects[i].faults[j].bndbox;
			CPoint corners[4];
			corners[0].x = sub_rect.x; corners[0].y = sub_rect.y;
			corners[1].x = sub_rect.x + sub_rect.width; corners[1].y = sub_rect.y;
			corners[2].x = sub_rect.x + sub_rect.width; corners[2].y = sub_rect.y + sub_rect.height;
			corners[3].x = sub_rect.x; corners[3].y = sub_rect.y + sub_rect.height;
			for (int subk = 0; subk < 4; ++subk)
			{
				CPoint cInWin;
				CoordImageToWin(corners[subk], cInWin);
				int dx = abs(pt.x - cInWin.x);
				int dy = abs(pt.y - cInWin.y);
				if (dx <= threshold && dy <= threshold)
				{
					id = i;
					subid = j;
					ptCorner = cInWin;
					cornerId = subk;
					return true;
				}
			}
		}
	}
	return false;
}

bool CImageView::GetCursorPosOnWindow(CPoint& ptImage, CPoint& ptWin)
{
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	if (pt.x > rect.left && pt.x < rect.right && pt.y > rect.top && pt.y < rect.bottom)
	{
		ptWin = pt;
		CoordWinToImage(ptWin, ptImage);
		return true;
	}
	else
		return false;
}

std::string CImageView::CStringToStdString(CString& str)
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

void CImageView::SaveAnnotations(const std::string& folderName, const std::string& fileName)
{
	// 不提示，直接保存
	if (m_isValidFile)
	{
		/*
		std::string filepath = m_Annotation.path;
		int index = filepath.find_last_of('.');
		std::string name = filepath.substr(0, index) + ".xml";
		*/
		int index = fileName.find_last_of('.');
		std::string saveName = fileName.substr(0, index) + ".xml";
		// change bad path names
		m_Annotation.folder = folderName;
		m_Annotation.filename = fileName;
		m_Annotation.path = folderName + fileName;
		SaveAnnotationToXML(m_Annotation, folderName + saveName);
		MessageBox(_T("保存完成"), _T("保存图片标注"));
		m_isModified = FALSE;
	}
	/*
	if (!m_isModified)
	{
		MessageBox(_T("没有修改"), _T("保存图片标注"));
		return;
	}

	if (m_SaveFilename.empty())
	{
		// 添加文件浏览，让用户选择保存文件路径
		char driver[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		_splitpath(m_Annotation.path.c_str(), driver, dir, fname, ext);

		CString defaultDir = _T("C:\\");
		CString filename = CString(fname) + ".xml";
		CString filter = _T("XML文件|*.xml");
		BOOL isOpen = FALSE; // 保存文件
		CFileDialog openFileDlg(isOpen, defaultDir, filename, 
			OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, filter);
		int nRes = openFileDlg.DoModal();
		if (nRes == IDOK)
		{
			CString path = openFileDlg.GetPathName();
			m_SaveFilename = CStringToStdString(path);
			// SaveToXML(m_Annotations, m_Filename, m_SaveFilename);
			SaveAnnotationToXML(m_Annotation, m_SaveFilename);
			MessageBox(_T("保存完成"), _T("保存图片标注"));
			m_isModified = FALSE;
		}
	}
	else
	{
		// SaveToXML(m_Annotations, m_Filename, m_SaveFilename);
		SaveAnnotationToXML(m_Annotation, m_SaveFilename);
		MessageBox(_T("保存完成"), _T("保存图片标注"));
		m_isModified = FALSE;
	}
	*/
}

void CImageView::LoadAnnotations(const std::string& path)
{
	// std::string path = m_Annotation.path;
	LoadAnnotationFromXML(m_Annotation, path);
}

void CImageView::SetObjectName(int id)
{
	id = m_iEditAnnot;
	if (id < 0 || id >= int(m_Annotation.objects.size()))
		return;
	std::string name = m_Annotation.objects[id].name;
	CString cname = CString(name.c_str());

	if (m_AnnotDlg == NULL)
		m_AnnotDlg = new CAnnotNameDlg(cname);

	m_AnnotDlg->SetName(cname);
	int d = m_Annotation.objects[id].difficult;
	std::string p = m_Annotation.objects[id].pose;
	std::string poses[] = {
		"Left", "Right", "Frontal", "Rear", "Top", "Bottom", "Unspecified"
	};

	int i = 6;
	for (int k = 0; k < 6; ++k)
	{
		if (p == poses[k])
		{
			i = k;
			break;
		}
	}

	m_AnnotDlg->SetDifficult((d == 1) ? TRUE : FALSE);
	m_AnnotDlg->SetPose(i);

	int nRes = m_AnnotDlg->DoModal();
	if (nRes == IDOK)
	{
		CString new_cname = m_AnnotDlg->GetName();
		std::string new_name = CStringToStdString(new_cname);
		// m_Annotations[id].SetName(new_name);
		m_Annotation.objects[id].name = new_name;
		BOOL new_d = m_AnnotDlg->GetDifficult();
		m_Annotation.objects[id].difficult = ((new_d == TRUE) ? 1 : 0);
		int new_p = m_AnnotDlg->GetPose();
		m_Annotation.objects[id].pose = poses[new_p];

		if (new_name != name || new_d != d || new_p != i)
			m_isModified = TRUE;
	}
}

void CImageView::SetFaultName(int id, int subid)
{
	id = m_iEditAnnot;
	subid = m_iEditSubAnnot;
	if (id < 0 || id >= int(m_Annotation.objects.size()))
		return;
	if (subid < 0 || subid >= int(m_Annotation.objects[id].faults.size()))
		return;
	std::string name = m_Annotation.objects[id].faults[subid].name;
	CString cname = CString(name.c_str());

	if (m_SubAnnotDlg == NULL)
		m_SubAnnotDlg = new CSubAnnotNameDlg(cname);
	int nRes = m_SubAnnotDlg->DoModal();
	if (nRes == IDOK)
	{
		CString new_cname = m_SubAnnotDlg->GetName();
		std::string new_name = CStringToStdString(new_cname);
		// m_Annotations[id].SetFaultName(subid, new_name);
		m_Annotation.objects[id].faults[subid].name = new_name;
		if (new_name != name)
			m_isModified = TRUE;
	}
	/*
	CSubAnnotNameDlg dlg(cname);
	int nRes = dlg.DoModal();
	if (nRes == IDOK)
	{
		CString new_cname = dlg.GetName();
		std::string new_name = CStringToStdString(new_cname);
		// m_Annotations[id].SetFaultName(subid, new_name);
		m_Annotation.objects[id].faults[subid].name = new_name;
		if (new_name != name)
			m_isModified = TRUE;
	}
	*/
}

void CImageView::ClearInfo()
{
	m_Filename = "";
	m_SaveFilename = "";
	m_isModified = FALSE;

	m_DownPoint = CPoint(0, 0);	// 鼠标按下时的位置
	m_CurPoint = CPoint(0, 0);	// 鼠标移动时当前位置

	m_MouseMode = MOUSE_MODE_NORMAL;
	m_bLBtnDown = FALSE;

	// m_CurAnnotation = NULL;
	// m_CurFaultInfo = NULL;

	m_CurObject = AnnotObject();
	m_CurFault = Fault();

	m_isDrawing = FALSE;
	m_isDrawingSub = FALSE;

	m_isEditing = FALSE;
	m_isEditingSub = FALSE;
	m_iEditAnnot = -1;
	m_iEditSubAnnot = -1;
	m_iEditCornerId = -1;

	m_iSubAnnot = -1;

	m_Annotation.Clear();
}
