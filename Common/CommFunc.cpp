#include "pch.h"
#include "CommFunc.h"

#include <afx.h>
#include <winreg.h>
#include <afxdlgs.h>
#include <shellapi.h>

void GetAvailableComPorts(CStringArray& arrPorts)
{
	HKEY hKey = NULL;
	arrPorts.RemoveAll();

	// 打开注册表路径
	if (RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		_T("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
		0,
		KEY_READ,
		&hKey) != ERROR_SUCCESS)
	{
		return;
	}

	TCHAR szValueName[256];
	TCHAR szPortName[256];
	DWORD dwValueNameSize;
	DWORD dwPortNameSize;
	DWORD dwType;
	DWORD dwIndex = 0;

	// 枚举所有注册表值
	while (1)
	{
		dwValueNameSize = sizeof(szValueName) / sizeof(TCHAR);
		dwPortNameSize = sizeof(szPortName);

		// 枚举注册表值
		LONG ret = RegEnumValue(
			hKey,
			dwIndex++,
			szValueName,
			&dwValueNameSize,
			NULL,
			&dwType,
			(LPBYTE)szPortName,
			&dwPortNameSize);

		if (ret == ERROR_NO_MORE_ITEMS)
			break;

		if (ret != ERROR_SUCCESS || dwType != REG_SZ)
			continue;

		// 将端口名添加到数组
		arrPorts.Add(szPortName);
	}

	RegCloseKey(hKey);
}

CString Hex2Str(unsigned char* buffer, int iBufferSize, bool bHasSpace)
{
	CString sTmp;
	for (int i = 0; i < iBufferSize; i++)
	{
		if (bHasSpace)
		{
			sTmp.AppendFormat(_T("%02X "), buffer[i]);
		}
		else
		{
			sTmp.AppendFormat(_T("%02X"), buffer[i]);
		}
	}
	sTmp.TrimRight();
	sTmp.TrimLeft();
	return sTmp;//前后空格不要
}

bool SaveContentToFile(CString sContent, CWnd* pParentWnd)
{

	CFileDialog dlgFileSave(
		FALSE,                       // FALSE表示保存对话框
		_T("txt"),                   // 默认扩展名
		_T("UartRecord.txt"),          // 默认文件名
		OFN_OVERWRITEPROMPT |        // 覆盖确认提示
		OFN_PATHMUSTEXIST,           // 路径必须存在
		_T("文本文件 (*.txt)|*.txt|所有文件 (*.*)|*.*||"), // 文件过滤器
		pParentWnd);                       // 父窗口

	if (dlgFileSave.DoModal() != IDOK)
	{
		return false; // 用户取消操作
	}

	CString sFilePath = dlgFileSave.GetPathName();

	try {
		CFile file(sFilePath, CFile::modeCreate | CFile::modeWrite);
		CStringA sAnsi(sContent); // 转换为ANSI格式
		file.Write(sAnsi, sAnsi.GetLength());
		file.Close();
	}
	catch (CFileException* e) {
		TCHAR szError[1024];
		e->GetErrorMessage(szError, 1024);
		AfxMessageBox(szError);
		e->Delete();
		return false;
	}

	return true;

}

int GetAppPath(CString& sAppPath, CString& sAppName)
{
	CString sFullPath;
	TCHAR szFilePath[MAX_PATH + 1];
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	sFullPath.Format(_T("%s"), szFilePath);
	int index = sFullPath.ReverseFind('\\');
	sAppPath = sFullPath.Left(index + 1);
	sAppName = sFullPath.Right(sFullPath.GetLength() - index - 1);
	return 0;
}

CString GetAppPath()
{
	CString sAppPath, sAppName;
	GetAppPath(sAppPath, sAppName);
	return sAppPath;
}


bool WriteIniFileInfo(const CString& sSection, const CString& sKey, const CString& sValue, const CString& sFilePath)
{
	bool bResult = FALSE;
	bResult = WritePrivateProfileString(
		sSection,
		sKey,
		(sValue.IsEmpty() || sValue == _T("0")) ? _T("00") : (LPCTSTR)sValue,
		sFilePath);
	return bResult;
}

CString ReadIniFileInfo(const CString& sSection, const CString& sKey, const CString& sDefault, const CString& sFilePath)
{
	TCHAR szBuffer[4096] = { 0 };
	GetPrivateProfileString(
		sSection,
		sKey,
		sDefault,
		szBuffer,
		_countof(szBuffer),
		sFilePath);

	return CString(szBuffer);
}

CString ConvertStrToHexAscii(const CString& sInput)
{
	CString strResult;
	for (int i = 0; i < sInput.GetLength(); ++i)
	{
		BYTE byteValue = static_cast<BYTE>(sInput.GetAt(i));

		CString strHex;
		strHex.Format(_T("%02X"), byteValue);

		if (i > 0) strResult += _T(" ");
		strResult += strHex;
	}
	return strResult;
}

CString ConvertHexAsciiToStr(const CString& sHexInput)
{
	CString sOrigin, sResult = _T("");
	sOrigin.Format(_T("%s"), sHexInput);

	sOrigin.Trim();
	sOrigin.Replace(_T(" "), _T(""));
	sOrigin.Replace(_T(","), _T(""));

	CString sFiltered = _T("");

	//过滤无效字符
	for (int i = 0; i < sOrigin.GetLength(); i++)
	{
		TCHAR cValue = sOrigin.GetAt(i);
		if ((cValue >= _T('0') && cValue <= _T('9')) || (cValue >= _T('a') && cValue <= _T('f')) || (cValue >= _T('A') && cValue <= _T('F')))
		{
			sFiltered += cValue;
		}
	}

	if (sFiltered.GetLength() % 2 != 0)
	{
		sFiltered = sFiltered + _T('0');
	}

	sFiltered.MakeUpper();

	// 每两个字符转换为一个字节
	for (int i = 0; i < sFiltered.GetLength(); i += 2)
	{
		TCHAR highChar = sFiltered.GetAt(i);
		TCHAR lowChar = sFiltered.GetAt(i + 1);

		// 将字符转换为对应的数值
		BYTE highVal = 0;
		if (highChar >= _T('A'))
		{
			highVal = highChar - _T('A') + 10;
		}
		else
		{
			highVal = highChar - _T('0');
		}

		BYTE lowVal = 0;
		if (lowChar >= _T('A'))
		{
			lowVal = lowChar - _T('A') + 10;
		}
		else
		{
			lowVal = lowChar - _T('0');
		}

		// 合并高低四位并添加到结果
		//BYTE byteVal = (highVal << 4) | lowVal;
		BYTE byteVal = (highVal * 0x10) + lowVal;
		sResult += (TCHAR)byteVal;
	}

	return sResult;


}

bool ValidateDecString(const CString& str)
{
	if (str.IsEmpty())
	{
		return false;
	}

	for (int i = 0; i < str.GetLength(); i++)
	{
		TCHAR c = str.GetAt(i);
		if (!((c >= _T('0') && c <= _T('9'))
			|| (i == 0 && c == _T('-'))))
		{
			return false;
		}
	}
	return true;
}

bool ValidateHexString(const CString& str)
{
	if (str.IsEmpty())
	{
		return true;
	}

	if (str.GetLength() % 2 != 0)
	{
		return false;
	}

	return str.SpanIncluding(_T("0123456789abcdefABCDEF")) == str;
}

CString ConvertDecToHex(const CString& sInputDec, bool bSmallEnd, bool bSigned)
{
	// 输入验证
	CString sDec = sInputDec;

	// 无符号数不能包含负号
	if (!bSigned && sDec.Find(_T('-')) != -1)
	{
		AfxMessageBox(_T("无符号数不能为负"));
		return _T("");
	}

	// 转换为数值
	__int64 nValue = _tcstoll(sDec, nullptr, 10);	//使用int64安全处理所有32bit整数

	if (bSigned)	//有符号整数
	{
		if (nValue > INT_MAX || nValue < INT_MIN)
		{
			AfxMessageBox(_T("数值超出有符号整数可计算范围(32bit)\r\n-2147483648,2147483647\r\n0x80000000,0x7FFFFFFF"));
			return _T("");
		}
	}
	else if (!bSigned)
	{
		if (nValue > UINT_MAX || nValue < 0)
		{
			AfxMessageBox(_T("数值超出无符号整数可计算范围(32bit)\r\n0,4294967295\r\n0x00000000,0xFFFFFFFF"));
			return _T("");
		}
	}
	else
	{
		return _T("");
	}

	// 转换为十六进制字符串
	CString sHex;
	sHex.Format(_T("%08X"), nValue);
	sHex.TrimLeft(_T('0'));
	if (sHex.IsEmpty())
	{
		sHex = _T("0");
	}

	// 补齐偶数长度
	if (sHex.GetLength() % 2 != 0)
	{
		sHex = _T("0") + sHex;
	}

	// 小端转换
	if (bSmallEnd) {
		CString sTemp;
		// 每两个字符反转
		for (int i = sHex.GetLength() - 2; i >= 0; i = i - 2)
		{
			sTemp += sHex.Mid(i, 2);
		}
		sHex = sTemp;
	}

	return sHex.MakeUpper();
}

CString ConvertHexToDec(const CString& sInputHex, bool bSmallEnd, bool bSigned)
{
	// 输入处理
	CString sHex = sInputHex;
	sHex.Trim();
	sHex.Replace(_T(" "), _T(""));
	sHex.Replace(_T(","), _T(""));
	if (sHex.IsEmpty()) return _T("输入不能为空");

	// 验证十六进制格式
	if (sHex.SpanIncluding(_T("0123456789ABCDEFabcdef")) != sHex)
	{
		AfxMessageBox(_T("包含非法十六进制字符"));
		return _T("");
	}


	// 补齐偶数长度
	if (sHex.GetLength() % 2 != 0)
	{
		sHex = _T("0") + sHex;
	}

	// 小端处理
	CString sProcessedHex = sHex;
	if (bSmallEnd)
	{
		CString sTemp;
		// 每两个字符反转
		for (int i = 0; i < sProcessedHex.GetLength(); i += 2)
		{
			sTemp = sProcessedHex.Mid(i, 2) + sTemp;
		}
		sProcessedHex = sTemp;
	}

	// 转换为数值
	unsigned long long nValue = _tcstoll(sProcessedHex, nullptr, 16);	//统一用64bit类型，避免溢出

	if (bSigned)	//有符号整数
	{
		if (sProcessedHex.GetLength() >= 8)
		{
			BYTE bHighByte = (nValue >> 24) & 0xFF;	//提取最高字节
			if (bHighByte & 0x80)	//最高位是1（负数）
			{
				nValue = nValue | 0xFFFFFFFF00000000;	//扩展到64bit数据，方便下一步判断范围
			}
		}

		//判断是否在32位有符号整数的范围内
		if ((__int64)nValue > INT_MAX || (__int64)nValue < INT_MIN)	//转为有符号数后再判断范围
		{
			AfxMessageBox(_T("数值超出有符号整数可计算范围(32bit)\r\n-2147483648,2147483647\r\n0x80000000,0x7FFFFFFF"));
			return _T("");
		}

	}
	else	//无符号整数
	{
		if (nValue > UINT_MAX)
		{
			AfxMessageBox(_T("数值超出无符号整数可计算范围(32bit)\r\n0,4294967295\r\n0x00000000,0xFFFFFFFF"));
			return _T("");
		}
	}

	// 转换为十进制字符串
	CString sDec = _T("");
	if (bSigned)
	{
		//有符号数，直接转为int型
		sDec.Format(_T("%d"), (int)((__int64)nValue));//利用强转自动处理符号位
	}
	else
	{
		//无符号数，用%u
		sDec.Format(_T("%u"), (UINT)nValue);
	}

	return sDec;
}

unsigned short CRC16(const unsigned char* buffer, unsigned int size)
{
	unsigned short crc = 0xFFFF;
	if (NULL != buffer && size > 0) {
		while (size--) {
			crc = (crc >> 8) | (crc << 8);
			crc ^= *buffer++;
			crc ^= ((unsigned char)crc) >> 4;
			crc ^= crc << 12;
			crc ^= (crc & 0xFF) << 5;
		}
	}
	return crc;
}


unsigned short GreenTwsCmdEncode(byte* byteData, unsigned short iDataLen, byte* byteOut, unsigned int& iOutLen)
{
	iOutLen = 0;
	byteOut[iOutLen++] = 0xAA;
	byteOut[iOutLen++] = byteData[0];//byteTarget
	byteOut[iOutLen++] = byteData[1];//byteCmdId
	byteOut[iOutLen++] = byteData[2];//byteCmdType
	byteOut[iOutLen++] = byteData[3];//byteStatus
	memcpy(byteOut + iOutLen, &iDataLen, 2);
	iOutLen += 2;
	memcpy(byteOut + iOutLen, byteData + 4, iDataLen);
	iOutLen += iDataLen;
	unsigned short iCrc16 = CRC16(byteOut, iOutLen);
	memcpy(byteOut + iOutLen, &iCrc16, 2);
	iOutLen += 2;
	return iCrc16;
}

unsigned short BlueTwsCmdEncode(byte* byteData, unsigned short iDataLen, byte* byteOut, unsigned int& iOutLen)
{
	iOutLen = 0;
	byteOut[iOutLen++] = 0xFE;
	byteOut[iOutLen++] = byteData[0];//byteTarget
	int iLength = iDataLen + 2;//cmd+data+CRC16(2byte)
	memcpy(byteOut + iOutLen, &iLength, 2);
	iOutLen += 2;
	memcpy(byteOut + iOutLen, byteData + 1, iDataLen);
	iOutLen += iDataLen;
	unsigned short iCrc16 = CRC16(byteOut, iOutLen);
	memcpy(byteOut + iOutLen, &iCrc16, 2);
	iOutLen += 2;
	return iCrc16;
}

bool RunExe(LPCTSTR lpszExePath)
{
	SHELLEXECUTEINFO sei = { sizeof(sei) };
	sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
	sei.lpVerb = _T("open");
	sei.lpFile = lpszExePath;
	sei.nShow = SW_SHOWNORMAL;

	if (!ShellExecuteEx(&sei))
	{
		//DWORD dwError = GetLastError();
		//CString strError;
		//strError.Format(_T("执行程序失败！错误代码: %d"), dwError);
		//AfxMessageBox(strError, MB_ICONERROR);
		return FALSE;
	}

	// 即使成功也立即返回，不等待进程结束
	if (sei.hProcess)
	{
		CloseHandle(sei.hProcess); // 如果不关心进程句柄，直接关闭
	}
	return TRUE;
}


int SplitStr(LPCTSTR sSource, LPCTSTR sSeprator, std::vector<CString>* vStr)
{
	TCHAR* ptr;
	TCHAR* buffer;
	TCHAR* StrPos, * preStrPos;
	int           iStrLen, spSize, tmpSize;
	CString str;
	iStrLen = _tcslen(sSource) + 1;
	spSize = _tcslen(sSeprator);
	buffer = new TCHAR[iStrLen];
	_tcscpy_s(buffer, iStrLen, sSource);
	// 查找有多少个 *seprator 并且 统计最大长度和个数;
	StrPos = buffer;
	preStrPos = buffer;
	while (1)
	{
		StrPos = _tcsstr(StrPos, sSeprator);
		if (StrPos == NULL)
		{
			// 找不到分隔符，那么长度就是最后的字符串
			tmpSize = _tcslen(preStrPos);
		}
		else
		{
			tmpSize = (int)(StrPos - preStrPos);
		}
		ptr = new TCHAR[tmpSize + 1];
		memset(ptr, 0, (tmpSize + 1) * sizeof(TCHAR));
		memcpy(ptr, preStrPos, tmpSize * sizeof(TCHAR));
		str = ptr; // 进行赋值和自动转换
		vStr->push_back(str);
		delete[]ptr;

		if (StrPos == NULL)
		{
			break;
		}
		StrPos += spSize;
		preStrPos = StrPos;
	}
	delete[]buffer;
	return 0;
}