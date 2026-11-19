#include "CRegUtils.h"

// 注册表大小限制, 参见 https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits
#define REG_KEY_NAME_CCH_LIMITS            (255)             // 注册表键名大小限制
#define REG_VALUE_NAME_CB_LIMITS          (16383)           // 注册表值名大小限制
#define REG_VALUE_DATA_CB_LIMITS          (1024 * 1024)     // 注册表值数据大小限制(可能超过此大小)

//数据缓冲, 设置为 1 MB, 一般数据的最大长度不会超过此限制
static BYTE s_DataBuf[REG_VALUE_DATA_CB_LIMITS];

// 写入 REG_DWORD 类型数据
bool CRegUtils::SetDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD dwValue)
{
    LSTATUS ls = ERROR_SUCCESS;

    ls = ::RegSetKeyValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), REG_DWORD, &dwValue, sizeof(dwValue));
    if (ERROR_SUCCESS != ls)
    {
        return false;
    }

    return ERROR_SUCCESS == ls;
}

// 读取 REG_DWORD 类型数据
bool CRegUtils::GetDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD& dwValue)
{
    LSTATUS ls = ERROR_SUCCESS;
    DWORD cbSize = sizeof(dwValue);

    ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_DWORD/* | RRF_ZEROONFAILURE*/, NULL, &dwValue, &cbSize);
    if (ERROR_SUCCESS != ls)
    {
        return false;
    }

    return ERROR_SUCCESS == ls;
}

// 写入 REG_QWORD 类型数据
bool CRegUtils::SetQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, ULONGLONG ullValue)
{
    LSTATUS ls = ERROR_SUCCESS;

    ls = ::RegSetKeyValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), REG_QWORD, &ullValue, sizeof(ullValue));
    if (ERROR_SUCCESS != ls)
    {
        return false;
    }

    return ERROR_SUCCESS == ls;
}

// 读取 REG_QWORD 类型数据
bool CRegUtils::GetQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, ULONGLONG& ullValue)
{
    LSTATUS ls = ERROR_SUCCESS;
    DWORD cbSize = sizeof(ullValue);

    ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_QWORD/* | RRF_ZEROONFAILURE*/, NULL, &ullValue, &cbSize);
    if (ERROR_SUCCESS != ls)
    {
        return false;
    }

    return ERROR_SUCCESS == ls;
}

// 写入 REG_SZ 类型数据
bool CRegUtils::SetString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, const _tstring& strValue)
{
    LSTATUS ls = ERROR_SUCCESS;

    ls = ::RegSetKeyValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), REG_SZ, strValue.c_str(), (DWORD)((strValue.length() + 1) * sizeof(TCHAR)));
    if (ERROR_SUCCESS != ls)
    {
        return false;
    }

    return ERROR_SUCCESS == ls;
}

// 读取 REG_SZ 类型数据
bool CRegUtils::GetString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, _tstring& strValue)
{
    LSTATUS ls = ERROR_SUCCESS;
    DWORD dwSize = sizeof(s_DataBuf);
    LPVOID lpBuf = NULL;

    ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_SZ/* | RRF_ZEROONFAILURE*/, NULL, s_DataBuf, &dwSize);
    if (ERROR_SUCCESS == ls)
    {
        strValue = (LPCTSTR)s_DataBuf;
        return true;
    }

    if (ERROR_MORE_DATA != ls)
    {
        return false;
    }

    // 数据长度超过常规大小限制, 则重新分配空间
    lpBuf = HeapAlloc(GetProcessHeap(), 0, dwSize);
    if (lpBuf)
    {
        ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_SZ /*| RRF_ZEROONFAILURE*/, NULL, lpBuf, &dwSize);
        if (ERROR_SUCCESS == ls)
        {
            strValue = (LPCTSTR)lpBuf;
        }

        HeapFree(GetProcessHeap(), 0, lpBuf);
    }

    return ERROR_SUCCESS == ls;
}

// 写入 REG_BINARY 类型数据
bool CRegUtils::SetBinary(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, LPCVOID lpData, DWORD cbSize)
{
    LSTATUS ls = ERROR_SUCCESS;
    ls = ::RegSetKeyValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), REG_BINARY, lpData, cbSize);
    return ERROR_SUCCESS == ls;
}

// 读取REG_BINARY类型数据
bool CRegUtils::GetBinary(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, LPVOID lpData, DWORD cbSize)
{
    LSTATUS ls = ERROR_SUCCESS;
    ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_BINARY/* | RRF_ZEROONFAILURE*/, NULL, lpData, &cbSize);
    return ERROR_SUCCESS == ls;
}

// 读取数据所需字节大小
bool CRegUtils::GetValueSize(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD& cbSize)
{
    LSTATUS ls = ERROR_SUCCESS;
    ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_ANY, NULL, s_DataBuf, &cbSize);
    return (ERROR_SUCCESS == ls) || (ERROR_MORE_DATA == ls);
}

// 创建 指定键
bool CRegUtils::CreateKey(HKEY hKey, const _tstring& lpSubKey)
{
    LSTATUS ls = ERROR_SUCCESS;
    HKEY hNewKey = NULL;
    DWORD dwDisposition = 0;

    ls = ::RegCreateKeyEx(hKey, lpSubKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey, &dwDisposition);

    if (hNewKey)
    {
        ::RegCloseKey(hNewKey);
    }

    return ERROR_SUCCESS == ls;
}

// 删除 指定键
bool CRegUtils::DelKey(HKEY hKey, const _tstring& lpSubKey)
{
    LSTATUS ls = ERROR_SUCCESS;
    ls = ::RegDeleteTree(hKey, lpSubKey.c_str());
    return ERROR_SUCCESS == ls;
}

// 删除 指定值
bool CRegUtils::DelValue(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName)
{
    LSTATUS ls = ERROR_SUCCESS;
    ls = ::RegDeleteKeyValue(hKey, lpSubKey.c_str(), lpValueName.c_str());
    return ERROR_SUCCESS == ls;
}

// 枚举 指定键 的子键
bool CRegUtils::EnumKeyName(HKEY hKey, const _tstring& lpSubKey, std::vector<_tstring>& keyList)
{
    TCHAR szKeyName[MAX_PATH] = { 0 };
    DWORD dwCchName = _countof(szKeyName);
    LSTATUS ls = ERROR_SUCCESS;
    HKEY hResultKey = NULL;
    DWORD dwIndex = 0;

    keyList.clear();

    ls = ::RegOpenKeyEx(hKey, lpSubKey.c_str(), 0, KEY_READ, &hResultKey);
    if (ERROR_SUCCESS != ls)
    {
        return false;
    }

    do
    {
        dwCchName = _countof(szKeyName);
        ls = ::RegEnumKeyEx(hResultKey, dwIndex++, szKeyName, &dwCchName, NULL, NULL, NULL, NULL);
        if (ERROR_SUCCESS == ls)
        {
            keyList.push_back(szKeyName);
        }
    } while (ERROR_SUCCESS == ls);

    ::RegCloseKey(hResultKey);

    return true;
}

// 枚举 指定键 的值名
bool CRegUtils::EnumValueName(HKEY hKey, const _tstring& lpSubKey, std::vector<_tstring>& valueList)
{
    TCHAR szValueName[MAX_PATH] = { 0 };
    DWORD dwCchName = _countof(szValueName);
    LSTATUS ls = ERROR_SUCCESS;
    HKEY hResultKey = NULL;
    DWORD dwIndex = 0;

    valueList.clear();

    ls = ::RegOpenKeyEx(hKey, lpSubKey.c_str(), 0, KEY_READ, &hResultKey);
    if (ERROR_SUCCESS != ls)
    {
        return false;
    }

    do
    {
        dwCchName = _countof(szValueName);
        ls = ::RegEnumValue(hResultKey, dwIndex++, szValueName, &dwCchName, NULL, NULL, NULL, NULL);
        if (ERROR_SUCCESS == ls)
        {
            valueList.push_back(szValueName);
        }
    } while (ERROR_SUCCESS == ls);

    ::RegCloseKey(hResultKey);

    return true;
}