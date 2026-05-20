
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
	DDX_Control(pDX, IDC_EDIT_DATA_CHANGE, m_Edit_ChangeValue);
	DDX_Control(pDX, IDC_EDIT_DESC, m_Edit_ChangeComment);
	DDX_Control(pDX, IDC_LIST_TOTAL_TABLE, m_List_TotalTable);
	DDX_Control(pDX, IDC_BUTTON_QUERY, m_Button_Query);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_Button_Save);
}

BEGIN_MESSAGE_MAP(CAccountBookDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_QUERY, &CAccountBookDlg::OnBnClickedButtonQuery)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CAccountBookDlg::OnBnClickedButtonSave)
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
		_T("change_val REAL NOT NULL,")
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

	// 插入列（只做一次，因为头控件没有列时才插入）
	if (m_List_TotalTable.GetHeaderCtrl()->GetItemCount() == 0)
	{
		m_List_TotalTable.InsertColumn(0, _T("姓名"), LVCFMT_LEFT, 160);
		m_List_TotalTable.InsertColumn(1, _T("欠账总额（元）"), LVCFMT_RIGHT, 160);
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
	//清空列表中所有旧数据
	m_List_TotalTable.DeleteAllItems();

	//确保列表控件已经设置好列（只需设置一次，可放在 OnInitDialog 中）
	//    但为了防止未设置，这里做一个简单检测：若无列则添加列
	if (m_List_TotalTable.GetHeaderCtrl()->GetItemCount() == 0)
	{
		//设置报表风格扩展样式
		m_List_TotalTable.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

		//添加两列：“姓名” 和 “欠账总额”
		m_List_TotalTable.InsertColumn(0, _T("姓名"), LVCFMT_LEFT, 150);
		m_List_TotalTable.InsertColumn(1, _T("欠账总额"), LVCFMT_RIGHT, 150);
	}

	//从数据库查询所有 User 记录
	//    使用结构体 + 静态回调来收集数据（避免 lambda 可能引起的编译问题）
	struct QueryParam {
		CListCtrl* pList;            //指向列表控件
		CAccountBookDlg* pDlg;       //可通过它访问 m_dbHelper，但回调是静态的
		CSQLiteHelper* pDB;          //直接传数据库对象指针
	};

	//静态回调函数（不能在函数内声明 lambda 时要求 C++11，用静态更保险）
	static auto callback = [](void* pParam, int argc, char** argv, char** azColName) -> int {
		QueryParam* param = (QueryParam*)pParam;
		if (argc < 2 || !argv[0] || !argv[1])
			return 0;  // 无效行

		// 从 UTF-8 转为可显示的 CString
		CString strName = FromUtf8(argv[0]);
		CString strTotal = FromUtf8(argv[1]);   // 总额是数字，转换也没副作用

		// 插入列表新行
		int nIndex = param->pList->GetItemCount();
		nIndex = param->pList->InsertItem(nIndex, strName);
		if (nIndex >= 0)
		{
			param->pList->SetItemText(nIndex, 1, strTotal);
		}
		return 0;
		};

	QueryParam param;
	param.pList = &m_List_TotalTable;
	param.pDB = &m_dbHelper;

	//执行查询 SQL（简单 SELECT）
	if (!m_dbHelper.ExecuteQuery(_T("SELECT user_name, user_total FROM User ORDER BY id;"),
		callback, &param))
	{
		AfxMessageBox(_T("刷新总表查询失败！"));
	}

	//调整列宽以适应内容
	m_List_TotalTable.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	m_List_TotalTable.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
}

void CAccountBookDlg::OnBnClickedButtonSave()
{
	// TODO: 在此添加控件通知处理程序代码
	CString sName, sValue, sComment;
	m_Edit_UserName.GetWindowText(sName);
	m_Edit_ChangeValue.GetWindowText(sValue);
	m_Edit_ChangeComment.GetWindowText(sComment);

	sName.Trim();
	sValue.Trim();
	sComment.Trim();

	if (sName.IsEmpty())
	{
		AfxMessageBox(_T("请输入用户姓名"));
		m_Edit_UserName.SetFocus();
		return;
	}
	if (sValue.IsEmpty())
	{
		AfxMessageBox(_T("请输入变更值"));
		m_Edit_ChangeValue.SetFocus();
		return;
	}

	BOOL bHasDot = FALSE;
	BOOL bValid = TRUE;
	for (int i = 0; i < sValue.GetLength(); i++)
	{
		TCHAR ch = sValue[i];
		if (ch == _T('-'))
		{
			if (i != 0) // 负号不在首位
			{
				bValid = FALSE;
				break;
			}
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

	if (bValid == FALSE)
	{
		AfxMessageBox(_T("变动金额格式不正确，请输入数字，允许有一个小数点和一个负号，负号仅允许在第一位。"));
		m_Edit_ChangeValue.SetFocus();
		return;
	}

	double dValue = _tstof(sValue);
	CString sDate = CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S"));

	CStringA sUtf8Name = ToUtf8(sName);
	CString sUtf8Comment = ToUtf8(sComment);

	sUtf8Name.Replace("'", "''"); // 转义单引号
	sUtf8Comment.Replace("'", "''");

	int nUserId = -1;
	double dOldTotal = 0.0;
	BOOL bUserExists = FALSE;

	CStringA sSqlQuery;
	sSqlQuery.Format("SELECT id, user_total FROM User WHERE user_name = '%s';", sUtf8Name.GetString());

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
		sSqlUpdate.Format("UPDATE User SET user_total = %.2f WHERE id = %d;", dNewTotal, nUserId);

	}
	else
	{
		sSqlUpdate.Format("INSERT INTO User (user_name, user_total) VALUES ('%s', %.2f);", sUtf8Name.GetString(), dNewTotal);
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
	sSqlInsertLog.Format("INSERT INTO Change_Log (user_id, change_val, change_date, change_comment) VALUES (%d, %.2f, '%s', '%s');",
		nUserId, dValue, CStringA(sDate).GetString(), sUtf8Comment.GetString());

	if (!m_dbHelper.ExecuteSQL(CString(sSqlInsertLog)))
	{
		AfxMessageBox(_T("保存变动日志失败"));
		return;
	}

	//刷新主列表
	RefreshTotalTable();

	//清空各输入框，姓名框保留，用来一个人输入多次
	m_Edit_ChangeValue.SetWindowText(_T(""));
	m_Edit_ChangeComment.SetWindowText(_T(""));
	m_Edit_ChangeValue.SetFocus();
}
