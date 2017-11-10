#pragma once

// CSubAnnotNameDlg dialog

class CSubAnnotNameDlg : public CDialog
{
	DECLARE_DYNAMIC(CSubAnnotNameDlg)

public:
	CSubAnnotNameDlg(CWnd* pParent = NULL);   // standard constructor
	CSubAnnotNameDlg(CString& name, CWnd* pParent = NULL);
	virtual ~CSubAnnotNameDlg();

	CString GetName() { return m_SubAnnotName; }

// Dialog Data
	enum { IDD = IDD_SUBANNOT_NAME_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnListDbClick();

public:
	CString m_SubAnnotName;
	virtual BOOL OnInitDialog();
	CListBox m_ListSubAnnot;
};
