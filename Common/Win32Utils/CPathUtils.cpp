#include "CPathUtils.h"
#include <windows.h>
#include <io.h>

_tstring CPathUtils::GetCurrentModulePath()
{
    TCHAR szCurPath[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szCurPath, _countof(szCurPath));

    _tstring strResult = szCurPath;
    return strResult;
}

_tstring CPathUtils::GetCurrentModuleDir()
{
    return GetFileDir(GetCurrentModulePath());
}

_tstring CPathUtils::GetCurrentModuleName(bool bHasExt/* = true*/)
{
    return GetFileName(GetCurrentModulePath(), bHasExt);
}

_tstring CPathUtils::GetFileDir(const _tstring& strPath)
{
    _tstring strResult;
    size_t nIndex = strPath.find_last_of(_T('\\'));
    if (nIndex != _tstring::npos)
    {
        strResult = strPath.substr(0, nIndex);
    }

    return strResult;
}

_tstring CPathUtils::GetFileName(const _tstring& strPath, bool bHasExt/* = true*/)
{
    _tstring strResult = strPath;
    size_t nIndex = strResult.find_last_of(_T('\\'));
    if (nIndex != _tstring::npos)
    {
        strResult = strResult.substr(nIndex + 1);
    }

    if (!bHasExt)
    {
        nIndex = strResult.find_last_of(_T('.'));
        if (nIndex != _tstring::npos)
        {
            strResult.resize(nIndex);
        }
    }

    return strResult;
}

_tstring CPathUtils::GetFileExt(const _tstring& strPath, bool bHasDot/* = true*/)
{
    _tstring strResult = strPath;
    size_t nIndex = strResult.find_last_of(_T('.'));
    if (nIndex != _tstring::npos)
    {
        if (bHasDot)
        {
            strResult = strResult.substr(nIndex);
        }
        else
        {
            strResult = strResult.substr(nIndex + 1);
        }
    }
    else
    {
        strResult = _T("");
    }

    return strResult;
}

_tstring CPathUtils::GetShortPath(const _tstring& strPath)
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    ::GetShortPathName(strPath.c_str(), szBuf, _countof(szBuf));
    return szBuf;
}

_tstring CPathUtils::GetLongPath(const _tstring& strPath)
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    ::GetLongPathName(strPath.c_str(), szBuf, _countof(szBuf));
    return szBuf;
}

_tstring CPathUtils::GetFullPath(const _tstring& strPath)
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    TCHAR* pFilePart = nullptr;
    ::GetFullPathName(strPath.c_str(), _countof(szBuf), szBuf, &pFilePart);

    return szBuf;
}

bool CPathUtils::IsArchive(const _tstring& strPath)
{
    WIN32_FILE_ATTRIBUTE_DATA attr = { 0 };
    if (!::GetFileAttributesEx(strPath.c_str(), GetFileExInfoStandard, &attr))
    {
        return false;
    }

    return attr.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE;
}

bool CPathUtils::IsDirectory(const _tstring& strPath)
{
    WIN32_FILE_ATTRIBUTE_DATA attr = { 0 };
    if (!::GetFileAttributesEx(strPath.c_str(), GetFileExInfoStandard, &attr))
    {
        return false;
    }

    return attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

bool CPathUtils::IsExist(const _tstring& strPath)
{
    return 0 == _taccess_s(strPath.c_str(), 00);
}

bool CPathUtils::QuoteSpaces(_tstring& strPath)
{
    if (strPath.empty())
    {
        return false;
    }

    size_t nFindBegin = strPath.find_first_of(_T('\"'));
    size_t nFindEnd = strPath.find_last_of(_T('\"'));

    //两边已经有引号
    if (0 == nFindBegin && (strPath.size() - 1 == nFindEnd))
    {
        return false;
    }

    //检查是否有空格
    if (_tstring::npos == strPath.find(_T(' ')))
    {
        return false;
    }

    strPath = _T("\"") + strPath + _T("\"");
    return true;
}

bool CPathUtils::UnquoteSpaces(_tstring& strPath)
{
    size_t nFindBegin = strPath.find_first_of(_T('\"'));
    size_t nFindEnd = strPath.find_last_of(_T('\"'));

    if (_tstring::npos == nFindBegin || _tstring::npos == nFindEnd)
    {
        return false;
    }

    strPath = strPath.substr(nFindBegin + 1, strPath.size() - 2);

    return true;
}

bool CPathUtils::Rename(const _tstring& strSrc, const _tstring& strDest)
{
    _tstring strOldPath = strSrc;
    _tstring strNewPath = strDest;

    //源路径相对路径转绝对路径
    if (_tstring::npos == strSrc.find(_T(":")))
    {
        strOldPath = GetCurrentModuleDir() + _T("\\") + strSrc;
    }

    //不存在则提前返回
    if (!IsExist(strOldPath))
    {
        return false;
    }

    //目标路径相对路径转绝对路径
    if (_tstring::npos == strDest.find(_T(":")))
    {
        strNewPath = GetFileDir(strOldPath) + _T("\\") + strDest;
    }

    return ::MoveFileEx(strOldPath.c_str(), strNewPath.c_str(), MOVEFILE_COPY_ALLOWED);
}

bool CPathUtils::Delete(const _tstring& strPath)
{
    if (IsArchive(strPath))
    {
        return DeleteArchive(strPath);
    }

    if (IsDirectory(strPath))
    {
        return DeleteDir(strPath);
    }

    return false;
}

bool CPathUtils::DeleteArchive(const _tstring& strPath)
{
    if (strPath.empty())
    {
        return false;
    }

    return ::DeleteFile(strPath.c_str());
}

bool CPathUtils::DeleteDir(const _tstring& strResult)
{
    WIN32_FIND_DATA findData = { 0 };
    HANDLE hFindHandle = INVALID_HANDLE_VALUE;

    if (strResult.empty())
    {
        return false;
    }

    hFindHandle = FindFirstFile((strResult + _T("\\*.*")).c_str(), &findData);
    if (INVALID_HANDLE_VALUE == hFindHandle)
    {
        return false;
    }

    do
    {
        _tstring strFilePath = strResult + _T("\\") + findData.cFileName;

        //上一级目录与当前目录跳过
        if (0 == _tcscmp(findData.cFileName, _T(".")) || 0 == _tcscmp(findData.cFileName, _T("..")))
        {
            continue;
        }

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //删除目录
        {
            DeleteDir(strFilePath);
            continue;
        }
        else    //删除文件
        {
            ::DeleteFile(strFilePath.c_str());
        }

    } while (::FindNextFile(hFindHandle, &findData));
    ::FindClose(hFindHandle);

    return ::RemoveDirectory(strResult.c_str());
}

bool CPathUtils::CreateDir(const _tstring& strPath)
{
    _tstring strDriver;              //驱动器号, 如 D:
    _tstring strSubPath = strPath;   //路径, 如 Test\1\2\3

    if (strPath.empty())
    {
        return false;
    }

    //获取盘符
    do
    {
        size_t nFindIndex = strPath.find_first_of(':');  //检查是否有驱动器号
        if (nFindIndex == _tstring::npos)
        {
            break;
        }

        strDriver = strPath.substr(0, nFindIndex + 1); //得到驱动器号, 如 D:
        nFindIndex = strPath.find(_T("\\"), nFindIndex);
        if (nFindIndex == _tstring::npos)
        {
            break;
        }

        strSubPath = strPath.substr(nFindIndex + 1); //得到路径, 如 Test\1\2\3
    } while (false);

    _tstring strDestDir;
    size_t nFindBegin = 0;
    size_t nFindIndex = 0;
    do
    {
        nFindIndex = strSubPath.find(_T("\\"), nFindBegin);
        if (nFindIndex != _tstring::npos)
        {
            strDestDir = strSubPath.substr(0, nFindIndex);
            nFindBegin = nFindIndex + 1;
        }
        else
        {
            strDestDir = strSubPath;
        }

        if (!strDriver.empty())
        {
            strDestDir = strDriver + _T("\\") + strDestDir;
        }

        if (!::CreateDirectory(strDestDir.c_str(), NULL) && ERROR_ALREADY_EXISTS != ::GetLastError())
        {
            return false;
        }

    } while (nFindIndex != _tstring::npos);

    return true;
}
