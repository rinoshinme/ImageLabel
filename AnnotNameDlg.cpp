// AnnotNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImageLabel.h"
#include "AnnotNameDlg.h"
#include "afxdialogex.h"

// CAnnotNameDlg dialog

IMPLEMENT_DYNAMIC(CAnnotNameDlg, CDialog)

CAnnotNameDlg::CAnnotNameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnnotNameDlg::IDD, pParent)
{
	m_AnnotName = _T("");
	m_RadioPoses = 6; // Unspecified
}

CAnnotNameDlg::CAnnotNameDlg(CString& name, CWnd* pParent)
	: CDialog(CAnnotNameDlg::IDD, pParent)
{
	m_AnnotName = name;
	m_RadioPoses = 6; // Unspecified
}

CAnnotNameDlg::~CAnnotNameDlg()
{
}

void CAnnotNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ANNOT_NAME, m_AnnotName);
	DDX_Control(pDX, IDC_LIST_ANNOT_NAMES, m_ListAnnot);
	DDX_Check(pDX, IDC_CHECK_DIFFICULT, m_bDifficult);
	DDX_Radio(pDX, IDC_RADIO_LEFT, m_RadioPoses);
	DDV_MinMaxInt(pDX, m_RadioPoses, 0, 6);
}


BEGIN_MESSAGE_MAP(CAnnotNameDlg, CDialog)
	ON_LBN_DBLCLK(IDC_LIST_ANNOT_NAMES, &CAnnotNameDlg::OnListDbClick)
	ON_BN_CLICKED(IDC_RADIO_LEFT, &CAnnotNameDlg::OnPoseRadioBnClicked)
	ON_BN_CLICKED(IDC_RADIO_RIGHT, &CAnnotNameDlg::OnPoseRadioBnClicked)
	ON_BN_CLICKED(IDC_RADIO_FRONTAL, &CAnnotNameDlg::OnPoseRadioBnClicked)
	ON_BN_CLICKED(IDC_RADIO_REAR, &CAnnotNameDlg::OnPoseRadioBnClicked)
	ON_BN_CLICKED(IDC_RADIO_UP, &CAnnotNameDlg::OnPoseRadioBnClicked)
	ON_BN_CLICKED(IDC_RADIO_DOWN, &CAnnotNameDlg::OnPoseRadioBnClicked)
	ON_BN_CLICKED(IDC_RADIO_UNSPECIFIED, &CAnnotNameDlg::OnPoseRadioBnClicked)
END_MESSAGE_MAP()


// CAnnotNameDlg message handlers


BOOL CAnnotNameDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	// read pre-defined annotations in text file
	char line[1024];
	FILE* stream;
	char* path = "./annotation_names.txt";

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
			m_ListAnnot.AddString(name.Trim());
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAnnotNameDlg::OnListDbClick()
{
	int i = m_ListAnnot.GetCurSel();
	if (i >= 0)
	{
		CString text;
		m_ListAnnot.GetText(i, text);
		m_AnnotName = text;
		UpdateData(FALSE);
	}
}


void CAnnotNameDlg::OnPoseRadioBnClicked()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	switch (m_RadioPoses)
	{
	case 0:
		//MessageBox(_T("left clicked"));
		break;
	case 1:
		//MessageBox(_T("right clicked"));
		break;
	case 2:
		//MessageBox(_T("front clicked"));
		break;
	case 3:
		//MessageBox(_T("rear clicked"));
		break;
	case 4:
		//MessageBox(_T("up clicked"));
		break;
	case 5:
		//MessageBox(_T("down clicked"));
		break;
	case 6:
		// MessageBox(_T("Unspecified"));
		break;
	default:
		break;
	}
}
