// UserDetailDlg.cpp: 实现文件
//

#include "pch.h"
#include "AccountBook.h"
#include "afxdialogex.h"
#include "UserDetailDlg.h"


// CUserDetailDlg 对话框

IMPLEMENT_DYNAMIC(CUserDetailDlg, CDialogEx)

CUserDetailDlg::CUserDetailDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_USER_DETAIL_DIALOG, pParent)
{

}

CUserDetailDlg::~CUserDetailDlg()
{
}

void CUserDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUserDetailDlg, CDialogEx)
END_MESSAGE_MAP()


// CUserDetailDlg 消息处理程序
