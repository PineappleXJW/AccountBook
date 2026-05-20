#pragma once
#include "sqlite3.h"   // SQLite 的头文件
#include <afx.h>       // MFC 基础（CString 等）

class CSQLiteHelper
{
private:
    sqlite3* m_pDB;    // 数据库连接对象

public:
    CSQLiteHelper() : m_pDB(nullptr) {}
    ~CSQLiteHelper() { Close(); }

    // 打开数据库文件（不存在则自动创建）
    bool Open(LPCTSTR lpszDBPath)
    {
        // 多字节字符集下 CString 可转为 const char*，但路径最好用 ANSI
        CStringA sPath(lpszDBPath);
        int rc = sqlite3_open(sPath, &m_pDB);
        return (rc == SQLITE_OK);
    }

    void Close()
    {
        if (m_pDB)
        {
            sqlite3_close(m_pDB);
            m_pDB = nullptr;
        }
    }

    // 执行不返回结果的 SQL（建表、插入、更新、删除）
    bool ExecuteSQL(LPCTSTR lpszSQL)
    {
        if (!m_pDB) return false;
        CStringA sSQL(lpszSQL);
        char* errMsg = nullptr;
        int rc = sqlite3_exec(m_pDB, sSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            // 错误信息可用 OutputDebugString 输出查看
            if (errMsg)
            {
                OutputDebugStringA(errMsg);
                sqlite3_free(errMsg);
            }
            return false;
        }
        return true;
    }

    // 执行查询，并返回结果（简单起见，用自定义回调把数据存起来）
    // 更易用的方式之后可以扩展，现在先用这个演示
    bool ExecuteQuery(LPCTSTR lpszSQL,
        int (*callback)(void*, int, char**, char**),
        void* pParam)
    {
        if (!m_pDB) return false;
        CStringA sSQL(lpszSQL);
        char* errMsg = nullptr;
        int rc = sqlite3_exec(m_pDB, sSQL, callback, pParam, &errMsg);
        if (rc != SQLITE_OK && errMsg)
        {
            OutputDebugStringA(errMsg);
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

    // 获取最近一次 INSERT 生成的自增 ID
    sqlite3_int64 GetLastInsertRowID()
    {
        if (m_pDB)
            return sqlite3_last_insert_rowid(m_pDB);
        return 0;
    }
};