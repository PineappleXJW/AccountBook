#pragma once
#include "afxdialogex.h"
#include "SQLiteHelper.h"
#include "CommFunc.h"

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
public:
	virtual BOOL OnInitDialog();

	CSQLiteHelper* m_pDBHelper;  // 数据库助手对象指针
	CString m_sUserID;   // 用户ID
	CString m_sUserName; // 用户姓名
	CString m_sUserTotal; // 用户欠账总额
	CStatic m_Static_Name;
	CEdit m_Edit_Total;
	CEdit m_Edit_Borrow;
	CEdit m_Edit_Repay;
	CButton m_Button_Save;
	CButton m_Button_AllSettle;
	CEdit m_Edit_ChangeComment;
	CListCtrl m_List_ChangeLogTable;

	void RefreshDetailTable();
};
