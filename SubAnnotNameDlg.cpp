// SubAnnotNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImageLabel.h"
#include "SubAnnotNameDlg.h"
#include "afxdialogex.h"

// CSubAnnotNameDlg dialog

IMPLEMENT_DYNAMIC(CSubAnnotNameDlg, CDialog)

CSubAnnotNameDlg::CSubAnnotNameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSubAnnotNameDlg::IDD, pParent)
{
	m_SubAnnotName = _T("");
}

CSubAnnotNameDlg::CSubAnnotNameDlg(CString& name, CWnd* pParent)
	: CDialog(CSubAnnotNameDlg::IDD, pParent)
{
	m_SubAnnotName = name;
}

CSubAnnotNameDlg::~CSubAnnotNameDlg()
{
}

void CSubAnnotNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SUBANNOT_NAME, m_SubAnnotName);
	DDX_Control(pDX, IDC_LIST_SUBANNOT_NAMES, m_ListSubAnnot);
}


BEGIN_MESSAGE_MAP(CSubAnnotNameDlg, CDialog)
	ON_LBN_DBLCLK(IDC_LIST_SUBANNOT_NAMES, &CSubAnnotNameDlg::OnListDbClick)
END_MESSAGE_MAP()


// CSubAnnotNameDlg message handlers


BOOL CSubAnnotNameDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	// read pre-defined annotations in text file
	char line[1024];
	FILE* stream;
	char* path = "./sub_annotation_names.txt";

	if ((stream = fopen(path, "r")) == NULL)
		return FALSE;

	while (!feof(stream))
	{
		char* ch = fgets(line, 1024, stream);
		if (ch == NULL)
			break;
		else
		{
			CString name = CString(line);
			m_ListSubAnnot.AddString(name.Trim());
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSubAnnotNameDlg::OnListDbClick()
{
	int i = m_ListSubAnnot.GetCurSel();
	if (i >= 0)
	{
		CString text;
		m_ListSubAnnot.GetText(i, text);
		m_SubAnnotName = text;
		UpdateData(FALSE);
	}
}
