
// AccountBookDlg.h: 头文件
//

#pragma once

#include "SQLiteHelper.h"
#include "CommFunc.h"

// CAccountBookDlg 对话框
class CAccountBookDlg : public CDialogEx
{
// 构造
public:
	CAccountBookDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ACCOUNTBOOK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CSQLiteHelper m_dbHelper;  // 数据库助手对象
	CString m_sAppPath;
	CString m_sAppName;
public:
	// 用户姓名
	CEdit m_Edit_UserName;
	// 欠
	CEdit m_Edit_Borrow;
	// 还
	CEdit m_Edit_Repay;
	// 变更备注
	CEdit m_Edit_ChangeComment;
	// 个人欠账总表
	CListCtrl m_List_TotalTable;
	CButton m_Button_Query;
	CButton m_Button_Save;
	// 结清
	CButton m_Button_AllSettle;

	afx_msg void OnBnClickedButtonQuery();
	afx_msg void OnBnClickedButtonSave();

	void RefreshTotalTable();
	afx_msg void OnBnClickedButtonAllSettle();
	virtual void OnOK();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClickListTotalTable(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListTotalTable(NMHDR* pNMHDR, LRESULT* pResult);
};
