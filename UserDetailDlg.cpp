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
	DDX_Control(pDX, IDC_DETAIL_STATIC_NAME, m_Static_Name);
	DDX_Control(pDX, IDC_DETAIL_EDIT_TOTAL, m_Edit_Total);
	DDX_Control(pDX, IDC_DETAIL_EDIT_DATA_BORROW, m_Edit_Borrow);
	DDX_Control(pDX, IDC_DETAIL_EDIT_DATA_REPAY, m_Edit_Repay);
	DDX_Control(pDX, IDC_DETAIL_BUTTON_SAVE, m_Button_Save);
	DDX_Control(pDX, IDC_DETAIL_BUTTON_ALL_SETTLE, m_Button_AllSettle);
	DDX_Control(pDX, IDC_DETAIL_EDIT_DESC, m_Edit_ChangeComment);
	DDX_Control(pDX, IDC_DETAIL_LIST_TOTAL_TABLE, m_List_ChangeLogTable);
}


BEGIN_MESSAGE_MAP(CUserDetailDlg, CDialogEx)
END_MESSAGE_MAP()


// CUserDetailDlg 消息处理程序

BOOL CUserDetailDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_Static_Name.SetWindowText(m_sUserName);  // 显示用户姓名
	m_Edit_Total.SetWindowText(m_sUserTotal);	// 显示用户欠账总额

	// ---------- 初始化总表列表控件 ----------
	// 设置扩展样式：整行选择 + 网格线
	m_List_ChangeLogTable.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// 插入三列：ID、姓名、欠账总额（只做一次）
	if (m_List_ChangeLogTable.GetHeaderCtrl()->GetItemCount() == 0)
	{
		m_List_ChangeLogTable.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		m_List_ChangeLogTable.InsertColumn(0, _T("ID"), LVCFMT_RIGHT, 60);
		m_List_ChangeLogTable.InsertColumn(1, _T("赊账"), LVCFMT_RIGHT, 110);
		m_List_ChangeLogTable.InsertColumn(2, _T("还账"), LVCFMT_RIGHT, 110);
		m_List_ChangeLogTable.InsertColumn(3, _T("净变动"), LVCFMT_RIGHT, 110);
		m_List_ChangeLogTable.InsertColumn(4, _T("时间"), LVCFMT_RIGHT, 250);
        m_List_ChangeLogTable.InsertColumn(5, _T("备注"), LVCFMT_LEFT, 110);
	}

    RefreshDetailTable();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CUserDetailDlg::RefreshDetailTable()
{
    // 清空列表所有行
    m_List_ChangeLogTable.DeleteAllItems();

    // 构造 SQL 查询（用户 ID 是整数，直接拼接，无注入风险）
    CStringA sSql;
    sSql.Format("SELECT id, change_borrow, change_repay, change_val, change_date, change_comment "
        "FROM Change_Log WHERE user_id = %s ORDER BY change_date ASC;",
        m_sUserID.GetString());

    // 回调参数结构
    struct QueryParam {
        CListCtrl* pList;
    } param;
    param.pList = &m_List_ChangeLogTable;

    // 静态回调函数
    static auto callback = [](void* pParam, int argc, char** argv, char** /*azColName*/) -> int {
        QueryParam* p = (QueryParam*)pParam;
        if (argc < 6 || !argv[0] || !argv[1] || !argv[2] || !argv[3] || !argv[4] || !argv[5])
            return 0;

        // 从 UTF-8 转成 CString 显示
        CString strID = FromUtf8(argv[0]);
        CString strBorrow = FromUtf8(argv[1]);
        CString strRepay = FromUtf8(argv[2]);
        CString strVal = FromUtf8(argv[3]);
        CString strDate = FromUtf8(argv[4]);
        CString strComment = FromUtf8(argv[5]);

        // 插入新行
        int nIndex = p->pList->GetItemCount();
        nIndex = p->pList->InsertItem(nIndex, strID);
        if (nIndex >= 0)
        {
            p->pList->SetItemText(nIndex, 1, strBorrow);
            p->pList->SetItemText(nIndex, 2, strRepay);
            p->pList->SetItemText(nIndex, 3, strVal);
            p->pList->SetItemText(nIndex, 4, strDate);
            p->pList->SetItemText(nIndex, 5, strComment);
        }
        return 0;
        };

    // 执行查询
    if (!m_pDBHelper->ExecuteQuery(CString(sSql), callback, &param))
    {
        AfxMessageBox(_T("加载变动明细失败！"));
        return;
    }

    // 自动调整列宽，时间和备注不需要
    for (int i = 0; i < 4; i++)
    {
        m_List_ChangeLogTable.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    }
}