#pragma once
#include "afxdialogex.h"


// CUserDetailDlg 对话框

class CUserDetailDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CUserDetailDlg)

public:
	CUserDetailDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CUserDetailDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_USER_DETAIL_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
