
// AccountBookDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "AccountBook.h"
#include "AccountBookDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAccountBookDlg 对话框



CAccountBookDlg::CAccountBookDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ACCOUNTBOOK_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAccountBookDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_USERNAME, m_Edit_UserName);
	DDX_Control(pDX, IDC_EDIT_DESC, m_Edit_ChangeComment);
	DDX_Control(pDX, IDC_LIST_TOTAL_TABLE, m_List_TotalTable);
	DDX_Control(pDX, IDC_BUTTON_QUERY, m_Button_Query);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_Button_Save);
	DDX_Control(pDX, IDC_EDIT_DATA_BORROW, m_Edit_Borrow);
	DDX_Control(pDX, IDC_EDIT_DATA_REPAY, m_Edit_Repay);
	DDX_Control(pDX, IDC_BUTTON_ALL_SETTLE, m_Button_AllSettle);
	DDX_Control(pDX, IDC_BUTTON_ADD, m_Button_Add);
}

BEGIN_MESSAGE_MAP(CAccountBookDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_QUERY, &CAccountBookDlg::OnBnClickedButtonQuery)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CAccountBookDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_ALL_SETTLE, &CAccountBookDlg::OnBnClickedButtonAllSettle)
	ON_NOTIFY(NM_CLICK, IDC_LIST_TOTAL_TABLE, &CAccountBookDlg::OnClickListTotalTable)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TOTAL_TABLE, &CAccountBookDlg::OnDblclkListTotalTable)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CAccountBookDlg::OnBnClickedButtonAdd)
END_MESSAGE_MAP()


// CAccountBookDlg 消息处理程序

BOOL CAccountBookDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	GetAppPath(m_sAppPath, m_sAppName);
	CString sDBPath = m_sAppPath + _T("accountbook.db");
	if (!m_dbHelper.Open(sDBPath))
	{
		AfxMessageBox(_T("无法打开数据库文件:" + sDBPath));
	}

	CString sCreatePersonTable = 
		_T("CREATE TABLE IF NOT EXISTS User (")
		_T("id INTEGER PRIMARY KEY AUTOINCREMENT,")
		_T("user_name TEXT NOT NULL,")
		_T("user_total REAL NOT NULL DEFAULT 0);");

	CString sCreateLogTable =
		_T("CREATE TABLE IF NOT EXISTS Change_Log (")
		_T("id INTEGER PRIMARY KEY AUTOINCREMENT,")
		_T("user_id INTEGER NOT NULL,")
		_T("change_borrow REAL NOT NULL DEFAULT 0,")    //本次借款
		_T("change_repay REAL NOT NULL DEFAULT 0,")     //本次还款
		_T("change_val REAL NOT NULL,")                 //净变动（= borrow - repay）
		_T("change_date TEXT NOT NULL,")
		_T("change_comment TEXT DEFAULT '',")
		_T("FOREIGN KEY(user_id) REFERENCES User(id));");
	
	if (!m_dbHelper.ExecuteSQL(sCreatePersonTable))
	{
		AfxMessageBox(_T("创建数据库表失败：User"));
		return FALSE;
	}
	if (!m_dbHelper.ExecuteSQL(sCreateLogTable))
	{
		AfxMessageBox(_T("创建数据库表失败：Change_Log"));
		return FALSE;
	}

	// ---------- 初始化总表列表控件 ----------
	// 设置扩展样式：整行选择 + 网格线
	m_List_TotalTable.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// 插入三列：ID、姓名、欠账总额（只做一次）
	if (m_List_TotalTable.GetHeaderCtrl()->GetItemCount() == 0)
	{
		m_List_TotalTable.InsertColumn(0, _T("ID"), LVCFMT_LEFT, 60);
		m_List_TotalTable.InsertColumn(1, _T("姓名"), LVCFMT_LEFT, 160);
		m_List_TotalTable.InsertColumn(2, _T("欠账总额（元）"), LVCFMT_RIGHT, 160);
	}

	// 加载数据库数据到列表（如果数据库有数据就显示，没有则显示空表格）
	RefreshTotalTable();
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CAccountBookDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAccountBookDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CAccountBookDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAccountBookDlg::OnBnClickedButtonQuery()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CAccountBookDlg::RefreshTotalTable()
{
	// 清空所有已有行（保留表头）
	m_List_TotalTable.DeleteAllItems();

	// 如果表头还未创建（理论上已在 OnInitDialog 创建，这里做防御性检测）
	if (m_List_TotalTable.GetHeaderCtrl()->GetItemCount() == 0)
	{
		m_List_TotalTable.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		m_List_TotalTable.InsertColumn(0, _T("ID"), LVCFMT_LEFT, 60);
		m_List_TotalTable.InsertColumn(1, _T("姓名"), LVCFMT_LEFT, 160);
		m_List_TotalTable.InsertColumn(2, _T("欠账总额（元）"), LVCFMT_RIGHT, 160);
	}

	// 查询参数（仅需列表指针）
	struct QueryParam {
		CListCtrl* pList;
	};

	// 回调：将查询结果插入列表
	static auto callback = [](void* pParam, int argc, char** argv, char** /*azColName*/) -> int {
		QueryParam* param = (QueryParam*)pParam;
		if (argc < 3 || !argv[0] || !argv[1] || !argv[2])
			return 0;

		CString strID = FromUtf8(argv[0]);
		CString strName = FromUtf8(argv[1]);
		CString strTotal = FromUtf8(argv[2]);

		int nIndex = param->pList->GetItemCount();
		nIndex = param->pList->InsertItem(nIndex, strID);   // 第0列：ID
		if (nIndex >= 0)
		{
			param->pList->SetItemText(nIndex, 1, strName);  // 第1列：姓名
			param->pList->SetItemText(nIndex, 2, strTotal); // 第2列：欠账总额
		}
		return 0;
		};

	QueryParam param;
	param.pList = &m_List_TotalTable;

	// 查询 id, user_name, user_total
	if (!m_dbHelper.ExecuteQuery(_T("SELECT id, user_name, user_total FROM User ORDER BY id;"),
		callback, &param))
	{
		AfxMessageBox(_T("刷新总表查询失败！"));
	}

	// 自适应列宽
	m_List_TotalTable.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	m_List_TotalTable.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
	m_List_TotalTable.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
}

static BOOL IsInputValueValid(CString sValue)
{
	BOOL bHasDot = FALSE;
	BOOL bValid = TRUE;
	for (int i = 0; i < sValue.GetLength(); i++)
	{
		TCHAR ch = sValue[i];
		if (ch == _T('-'))	//不允许用户输入负号，输入整数后由代码来做减法，最终值为Repay-Borrow的结果
		{
			bValid = FALSE;
			break;
		}
		else if (ch == _T('.'))
		{
			if (bHasDot) // 多个小数点
			{
				bValid = FALSE;
				break;
			}
			bHasDot = TRUE;
		}
		else if (ch < _T('0') || ch > _T('9'))
		{
			bValid = FALSE;
			break;
		}
	}
	return bValid;
}

void CAccountBookDlg::OnBnClickedButtonSave()
{
	// TODO: 在此添加控件通知处理程序代码
	CString sName, sBorrow, sRepay, sComment;
	CString sDate = CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S"));
	m_Edit_UserName.GetWindowText(sName);
	m_Edit_Borrow.GetWindowText(sBorrow);
	m_Edit_Repay.GetWindowText(sRepay);
	m_Edit_ChangeComment.GetWindowText(sComment);

	sName.Trim();
	sBorrow.Trim();
	sRepay.Trim();
	sComment.Trim();

	if (sName.IsEmpty())
	{
		AfxMessageBox(_T("请输入用户姓名"));
		m_Edit_UserName.SetFocus();
		return;
	}
	if (sBorrow.IsEmpty() && sRepay.IsEmpty())
	{
		AfxMessageBox(_T("请输入欠或还金额"));
		m_Edit_Borrow.SetFocus();
		return;
	}

	if (sBorrow.IsEmpty())
	{
		sBorrow = _T("0");
	}

	if (sRepay.IsEmpty())
	{
		sRepay = _T("0");
	}
	if (IsInputValueValid(sBorrow) == FALSE)
	{
		AfxMessageBox(_T("欠款金额格式不正确，请输入数字，只允许有一个小数点。"));
		m_Edit_Borrow.SetFocus();
		return;
	}
	if (IsInputValueValid(sRepay) == FALSE)
	{
		AfxMessageBox(_T("还款金额格式不正确，请输入数字，只允许有一个小数点。"));
		m_Edit_Repay.SetFocus();
		return;
	}

	double dBorrow = _tstof(sBorrow);   // 借款金额（已保证为数值）
	double dRepay = _tstof(sRepay);    // 还款金额（已保证为数值）
	double dValue = dBorrow - dRepay;

	CStringA sUtf8Name = ToUtf8(sName);
	CStringA sUtf8Comment = ToUtf8(sComment);

	sUtf8Name.Replace("'", "''"); // 转义单引号
	sUtf8Comment.Replace("'", "''");

	int nUserId = -1;
	double dOldTotal = 0.0;
	BOOL bUserExists = FALSE;

	CStringA sSqlQuery;
	sSqlQuery.Format("SELECT id, user_total FROM User WHERE user_name = '%s';", 
		sUtf8Name.GetString()
	);

	struct QueryResult
	{
		int userId;
		double userTotal;
		BOOL found;
	} queryResult = { 0, 0.0, FALSE };

	auto callback = [](void* pParam, int argc, char** argv, char** azColName) -> int
		{
			QueryResult* pResult = (QueryResult*)pParam;
			if (argc > 0 && argv[0] != NULL)
			{
				pResult->userId = atoi(argv[0]);
				pResult->userTotal = atof(argv[1]);
				pResult->found = TRUE;
			}
			return 0;
		};

	if (!m_dbHelper.ExecuteQuery(CString(sSqlQuery), callback, &queryResult))
	{
		AfxMessageBox(_T("查询用户信息失败"));
		return;
	}
	bUserExists = queryResult.found;
	nUserId = queryResult.userId;
	dOldTotal = queryResult.userTotal;

	//计算新的总额
	double dNewTotal = (bUserExists ? dOldTotal : 0.0) + dValue;

	//更新或插入用户信息
	CStringA sSqlUpdate;
	if (bUserExists)
	{
		sSqlUpdate.Format("UPDATE User SET user_total = %.2f WHERE id = %d;", 
			dNewTotal, 
			nUserId
		);

	}
	else
	{
		sSqlUpdate.Format("INSERT INTO User (user_name, user_total) VALUES ('%s', %.2f);", 
			sUtf8Name.GetString(), 
			dNewTotal
		);
	}
	if (!m_dbHelper.ExecuteSQL(CString(sSqlUpdate)))
	{
		AfxMessageBox(_T("保存用户信息失败"));
		return;
	}

	//如果是新插入的用户，获取自增ID
	if (!bUserExists)
	{
		nUserId = (int)m_dbHelper.GetLastInsertRowID();
		if (nUserId == 0)
		{
			AfxMessageBox(_T("获取新用户ID失败"));
			return;
		}
	}

	//插入变动日志
	CStringA sSqlInsertLog;
	sSqlInsertLog.Format("INSERT INTO Change_Log (user_id, change_borrow, change_repay, change_val, change_date, change_comment) VALUES (%d, %.2f, %.2f, %.2f, '%s', '%s');",
		nUserId, 
		dBorrow, 
		dRepay, 
		dValue, 
		CStringA(sDate).GetString(), 
		sUtf8Comment.GetString()
	);

	if (!m_dbHelper.ExecuteSQL(CString(sSqlInsertLog)))
	{
		AfxMessageBox(_T("保存变动日志失败"));
		return;
	}

	//刷新主列表
	RefreshTotalTable();

	//清空各输入框，姓名框保留，用来一个人输入多次
	m_Edit_Borrow.SetWindowText(_T(""));
	m_Edit_Repay.SetWindowText(_T(""));
	m_Edit_ChangeComment.SetWindowText(_T(""));
	m_Edit_Borrow.SetFocus();
}

void CAccountBookDlg::OnBnClickedButtonAllSettle()
{
	//获取姓名
	CString sName;
	m_Edit_UserName.GetWindowText(sName);
	sName.Trim();

	if (sName.IsEmpty())
	{
		AfxMessageBox(_T("请先输入用户姓名！"));
		m_Edit_UserName.SetFocus();
		return;
	}

	//转换为 UTF-8 并转义单引号
	CStringA sUtf8Name = ToUtf8(sName);
	sUtf8Name.Replace("'", "''");

	//查询用户 id 和当前总额
	CStringA sSqlQuery;
	sSqlQuery.Format("SELECT id, user_total FROM User WHERE user_name='%s';", 
		sUtf8Name.GetString()
	);

	struct QueryResult {
		int userId;
		double userTotal;
		BOOL found;
	} result = { 0, 0.0, FALSE };

	auto callback = [](void* pParam, int argc, char** argv, char** /*azColName*/) -> int {
		QueryResult* pRes = (QueryResult*)pParam;
		if (argc > 0 && argv[0] != NULL)
		{
			pRes->userId = atoi(argv[0]);
			pRes->userTotal = (argv[1] != NULL) ? atof(argv[1]) : 0.0;
			pRes->found = TRUE;
		}
		return 0;
		};

	if (!m_dbHelper.ExecuteQuery(CString(sSqlQuery), callback, &result))
	{
		AfxMessageBox(_T("查询用户信息失败！"));
		return;
	}

	if (!result.found)
	{
		AfxMessageBox(_T("用户“") + sName + _T("”不存在！"));
		m_Edit_UserName.SetFocus();
		return;
	}

	CString sMsg;
	sMsg.Format(_T("确认要结清【%s】的所有【%.2f】元款项吗？"), sName, result.userTotal);
	if (AfxMessageBox(sMsg, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;  // 用户取消
	}

	//开始事务
	if (!m_dbHelper.ExecuteSQL(_T("BEGIN;")))
	{
		AfxMessageBox(_T("数据库操作失败！"));
		return;
	}

	//将用户总额清零
	CStringA sSqlUpdate;
	sSqlUpdate.Format("UPDATE User SET user_total = 0 WHERE id = %d;", 
		result.userId
	);
	if (!m_dbHelper.ExecuteSQL(CString(sSqlUpdate)))
	{
		m_dbHelper.ExecuteSQL(_T("ROLLBACK;"));
		AfxMessageBox(_T("清零用户总额失败！"));
		return;
	}

	//插入一条结清记录
	CString sDate = CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S"));
	CStringA sUtf8Comment = ToUtf8(_T("结清一次"));  // 固定备注
	sUtf8Comment.Replace("'", "''");

	CStringA sSqlInsertLog;
	sSqlInsertLog.Format(
		"INSERT INTO Change_Log (user_id, change_borrow, change_repay, change_val, change_date, change_comment) "
		"VALUES (%d, 0, 0, 0, '%s', '%s');",
		result.userId,
		CStringA(sDate).GetString(),
		sUtf8Comment.GetString()
	);

	if (!m_dbHelper.ExecuteSQL(CString(sSqlInsertLog)))
	{
		m_dbHelper.ExecuteSQL(_T("ROLLBACK;"));
		AfxMessageBox(_T("插入结清记录失败！"));
		return;
	}

	//提交事务
	if (!m_dbHelper.ExecuteSQL(_T("COMMIT;")))
	{
		m_dbHelper.ExecuteSQL(_T("ROLLBACK;"));
		AfxMessageBox(_T("提交事务失败！"));
		return;
	}

	//刷新总表显示
	RefreshTotalTable();

	//清空姓名输入框（也可保留，按习惯调整）
	m_Edit_UserName.SetWindowText(_T(""));
	m_Edit_UserName.SetFocus();
}

void CAccountBookDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	OnBnClickedButtonSave();
	//CDialogEx::OnOK();
}

BOOL CAccountBookDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			// 可类似判断，这里暂全部拦截
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CAccountBookDlg::OnClickListTotalTable(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	int nItem = pNMItemActivate->iItem;          // 被点击的行索引
	if (nItem != -1)                             // 确保点击有效行
	{
		CString strName = m_List_TotalTable.GetItemText(nItem, 1); // 姓名在第1列
		m_Edit_UserName.SetWindowText(strName);
	}
	*pResult = 0;
}

void CAccountBookDlg::OnDblclkListTotalTable(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	int nItem = pNMItemActivate->iItem;
	if (nItem != -1)   // 确保双击的是有效行
	{
		// 暂时只获取用户ID和姓名，以备后续传递给详情对话框
		CString sUserID = m_List_TotalTable.GetItemText(nItem, 0);   // 第一列：ID
		CString sUserName = m_List_TotalTable.GetItemText(nItem, 1); // 第二列：姓名
		CString sTotal = m_List_TotalTable.GetItemText(nItem, 2);    // 第三列：欠账总额（可选）

		// 创建并弹出模态对话框（目前为空，不加载数据）
		CUserDetailDlg dlg;
		dlg.m_sUserID = sUserID;
		dlg.m_sUserName = sUserName;
		dlg.m_sUserTotal = sTotal;
		dlg.m_pDBHelper = &m_dbHelper;  // 传递数据库助手对象
		dlg.DoModal();
	}
	*pResult = 0;
}

void CAccountBookDlg::OnBnClickedButtonAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	CString sName;
	m_Edit_UserName.GetWindowText(sName);
	sName.Trim();

	if (sName.IsEmpty())
	{
		AfxMessageBox(_T("请输入用户姓名！"));
		m_Edit_UserName.SetFocus();
		return;
	}

	CStringA sUtf8Name = ToUtf8(sName);
	sUtf8Name.Replace("'", "''");

	CStringA sSqlQuery;
	sSqlQuery.Format("SELECT id FROM User WHERE user_name='%s';",
		sUtf8Name.GetString()
	);

	struct QueryResult {
		int userId;
		BOOL found;
	} result = { 0, FALSE };

	auto callback = [](void* pParam, int argc, char** argv, char** /*azColName*/) -> int {
		QueryResult* pRes = (QueryResult*)pParam;
		if (argc > 0 && argv[0] != NULL)
		{
			pRes->userId = atoi(argv[0]);
			pRes->found = TRUE;
		}
		return 0;
		};

	if (!m_dbHelper.ExecuteQuery(CString(sSqlQuery), callback, &result))
	{
		AfxMessageBox(_T("查询用户信息失败！"));
		return;
	}

	if (result.found)
	{
		AfxMessageBox(_T("用户【") + sName + _T("】已存在！"));
		m_Edit_UserName.SetFocus();
		return;
	}

	CStringA sSqlInsert;
	sSqlInsert.Format("INSERT INTO User (user_name, user_total) VALUES ('%s', 0);",
		sUtf8Name.GetString()
	);
	if (!m_dbHelper.ExecuteSQL(CString(sSqlInsert)))
	{
		AfxMessageBox(_T("插入用户信息失败！"));
		return;
	}

	RefreshTotalTable();

	m_Edit_UserName.SetWindowText(_T(""));
	m_Edit_UserName.SetFocus();
}
