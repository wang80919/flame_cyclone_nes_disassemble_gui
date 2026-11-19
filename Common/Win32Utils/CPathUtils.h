#pragma once

#include <string>
#include <tchar.h>
#include <windows.h>
#include <vector>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CPathUtils
{
public:

    // 
    // @brief: 获取当前进程完全路径
    // @ret: 当前进程完全路径 如 D:\Software\HxDPortableSetup.exe
    static _tstring GetCurrentModulePath();

    // 
    // @brief: 获取当前进程所在目录
    // @ret: 当前进程所在目录 如 D:\Software
    static _tstring GetCurrentModuleDir();

    // 
    // @brief: 获取当前进程名
    // @ret: 当前进程名 如 HxDPortableSetup.exe
    static _tstring GetCurrentModuleName(bool bHasExt = false);

    // 
    // @brief: 获取文件所在文件夹
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 文件夹        如 D:\Software
    static _tstring GetFileDir(const _tstring& strPath);

    // 
    // @brief: 获取文件名
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @param: bHasExt     是否包含扩展名
    // @ret: 文件夹        如 HxDPortableSetup
    static _tstring GetFileName(const _tstring& strPath, bool bHasExt = false);

    // 
    // @brief: 获取文件扩展名
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @param: bHasDot     是否包含点号
    // @ret: 文件夹        如 exe
    static _tstring GetFileExt(const _tstring& strPath, bool bHasDot = false);

    // 
    // @brief: 获取短路径
    // @param: strPath     输入长路径(必须存在)
    // @ret: 短路径
    static _tstring GetShortPath(const _tstring& strPath);

    // 
    // @brief: 获取长路径
    // @param: strPath     输入短路径(必须存在)
    // @ret: 长路径
    static _tstring GetLongPath(const _tstring& strPath);

    // 
    // @brief: 获取全路径
    // @param: strPath     输入路径
    // @ret: 全路径
    static _tstring GetFullPath(const _tstring& strPath);

    // 
    // @brief: 检查文件是否存在
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在      存在返回 true
    static bool IsArchive(const _tstring& strPath);

    // 
    // @brief: 检查文件是否存在
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在      存在返回 true
    static bool IsDirectory(const _tstring& strPath);

    // 
    // @brief: 检查路径是否存在
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在      存在返回 true
    static bool IsExist(const _tstring& strPath);

    // 
    // @brief: 如果路径存在空格, 则使用引号括起来
    // @param: strPath     文件名, 如: C:\Program Files (x86)
    // @ret: 添加引号      添加引号返回 true
    static bool QuoteSpaces(_tstring& strPath);

    // 
    // @brief: 删除路径开头和末尾的引号
    // @param: strPath     文件名, 如: "C:\Program Files (x86)"
    // @ret: 去除引号      去除引号返回 true
    static bool UnquoteSpaces(_tstring& strPath);

    //
    // @brief: 重命名
    // @param: strSrc      原始文件名
    // @param: strDest     目标文件名(绝对路径或仅目标名)
    // @ret: 成功返回true
    static bool Rename(const _tstring& strSrc, const _tstring& strDest);

    //
    // @brief: 删除文件或文件夹
    // @param: strPath      路径
    // @ret: 成功返回true
    static bool Delete(const _tstring& strPath);

    //
    // @brief: 删除文件
    // @param: strPath      路径
    // @ret: 成功返回true
    static bool DeleteArchive(const _tstring& strPath);

    //
    // @brief: 删除目录(递归删除子目录及文件)
    // @param: strPath      路径
    // @ret: 成功返回true
    static bool DeleteDir(const _tstring& strPath);

    //
    // @brief: 创建目录(递归)
    // @param: strPath      路径
    // @ret: 成功返回true
    static bool CreateDir(const _tstring& strPath);
};