#include <winreg.h>
#include <vector>
#pragma once
/*自定义消息*/

void GetAvailableComPorts(CStringArray& arrPorts);
CString Hex2Str(unsigned char* buffer, int iBufferSize, bool bHasSpace);
bool SaveContentToFile(CString sContent, CWnd* pParentWnd);

int GetAppPath(CString& sAppPath, CString& sAppName);
CString GetAppPath();
bool WriteIniFileInfo(const CString& sSection, const CString& sKey, const CString& sValue, const CString& sFilePath);
CString ReadIniFileInfo(const CString& sSection, const CString& sKey, const CString& sDefault, const CString& sFilePath);


CString ConvertStrToHexAscii(const CString& sInput);
CString ConvertHexAsciiToStr(const CString& sHexInput);
CString ConvertDecToHex(const CString& sInputDec, bool bSmallEnd, bool bSigned);
CString ConvertHexToDec(const CString& sInputHex, bool bSmallEnd, bool bSigned);
bool ValidateDecString(const CString& str);
bool ValidateHexString(const CString& str);

unsigned short CRC16(const unsigned char* buffer, unsigned int size);

unsigned short GreenTwsCmdEncode(byte* byteData,	//方向+命令字+状态位+数据位
	unsigned short iDataLen,						//数据位长度
	byte* byteOut,									//存放完整指令
	unsigned int& iOutLen);							//完整指令字节数

unsigned short BlueTwsCmdEncode(byte* byteData,	//Type+Word+Data
	unsigned short iDataLen,					//Word+Data的长度
	byte* byteOut,								//存放完整指令
	unsigned int& iOutLen);						//完整指令字节数

bool RunExe(LPCTSTR lpszExePath);
int SplitStr(LPCTSTR sSource, LPCTSTR sSeprator, std::vector<CString>* vStr);
// CString(GBK) -> UTF-8 字节串 (CStringA)
inline CStringA ToUtf8(LPCTSTR lpszSrc)
{
	// 先转为宽字符 Unicode
	int nLen = MultiByteToWideChar(CP_ACP, 0, lpszSrc, -1, NULL, 0);
	if (nLen == 0) return CStringA();
	WCHAR* pWide = new WCHAR[nLen];
	MultiByteToWideChar(CP_ACP, 0, lpszSrc, -1, pWide, nLen);
	// 再转为 UTF-8
	int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, pWide, -1, NULL, 0, NULL, NULL);
	char* pUtf8 = new char[nUtf8Len];
	WideCharToMultiByte(CP_UTF8, 0, pWide, -1, pUtf8, nUtf8Len, NULL, NULL);
	CStringA strResult(pUtf8);
	delete[] pWide;
	delete[] pUtf8;
	return strResult;
}

// UTF-8 字节串 -> CString(GBK)
inline CString FromUtf8(const char* pUtf8)
{
	if (!pUtf8) return CString();
	int nLen = MultiByteToWideChar(CP_UTF8, 0, pUtf8, -1, NULL, 0);
	if (nLen == 0) return CString();
	WCHAR* pWide = new WCHAR[nLen];
	MultiByteToWideChar(CP_UTF8, 0, pUtf8, -1, pWide, nLen);
	int nGBKLen = WideCharToMultiByte(CP_ACP, 0, pWide, -1, NULL, 0, NULL, NULL);
	char* pGBK = new char[nGBKLen];
	WideCharToMultiByte(CP_ACP, 0, pWide, -1, pGBK, nGBKLen, NULL, NULL);
	CString strResult(pGBK);
	delete[] pWide;
	delete[] pGBK;
	return strResult;
}