#pragma once
#include "afxwin.h"


// CAnnotNameDlg dialog

class CAnnotNameDlg : public CDialog
{
	DECLARE_DYNAMIC(CAnnotNameDlg)

public:
	CAnnotNameDlg(CWnd* pParent = NULL);   // standard constructor
	CAnnotNameDlg(CString& name, CWnd* pParent = NULL);
	virtual ~CAnnotNameDlg();

	CString GetName() { return m_AnnotName; }
	void SetName(const CString& name)
	{
		m_AnnotName = name;
	}

	BOOL GetDifficult() { return m_bDifficult; }
	void SetDifficult(BOOL difficult) { m_bDifficult = difficult; }

	int GetPose() { return m_RadioPoses; }
	void SetPose(int pose) { m_RadioPoses = pose; }

// Dialog Data
	enum { IDD = IDD_ANNOT_NAME_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnListDbClick();

public:
	CString m_AnnotName;

	virtual BOOL OnInitDialog();
	CListBox m_ListAnnot;
	BOOL m_bDifficult;
	int m_RadioPoses;
	afx_msg void OnPoseRadioBnClicked();
};
