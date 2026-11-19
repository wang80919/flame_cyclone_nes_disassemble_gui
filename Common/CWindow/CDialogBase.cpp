#include "CDialogBase.h"
#include <Windows.h>
#include <strsafe.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

BEGIN_DLG_BASE_MESSAGE_MAP(CDialogBase)
    ON_DLG_MESSAGE(WM_CLOSE, &CDialogBase::OnClose)
END_DLG_BASE_MESSAGE_MAP()

CDialogBase::CDialogBase()
    :
    m_hWnd(NULL),
    m_resID(0),
    m_langId(0),
    m_bModel(FALSE),
    m_bFakeModel(FALSE),
    m_bCenter(FALSE)
{

}

CDialogBase::~CDialogBase()
{
    if (NULL != m_hWnd)
    {
        ::DestroyWindow(m_hWnd);
    }
}

HWND CDialogBase::DoDialog(UINT resourceID, HWND hWndParent, BOOL isShow, bool bCenter/* = true*/)
{
    m_bCenter = bCenter;
    m_bModel = FALSE;
    m_resID = resourceID;
    HWND hWnd = ::CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceID), hWndParent, DialogProc, (LPARAM)this);
    m_hWnd = hWnd;

    if (isShow)
    {
        ::ShowWindow(m_hWnd, SW_SHOW);
    }

    return m_hWnd;
}

INT_PTR CDialogBase::DoModalEx(UINT resourceID, HWND hWndParent, BOOL isShow, bool bCenter/* = true*/)
{
    m_bCenter = bCenter;
    m_bModel = FALSE;
    return DoFakeModal(resourceID, hWndParent, isShow);
}

INT_PTR CDialogBase::DoModal(HWND hWndParent, bool bCenter/* = true*/)
{
    m_bCenter = bCenter;
    m_bModel = TRUE;
    return ::DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(m_resID), hWndParent, DialogProc, (LPARAM)this);
}

INT_PTR CDialogBase::DoModal(UINT resourceID, HWND hWndParent, bool bCenter/* = true*/)
{
    m_bCenter = bCenter;
    m_resID = resourceID;
    m_bModel = TRUE;
    return ::DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(m_resID), hWndParent, DialogProc, (LPARAM)this);
}

BOOL CDialogBase::PreTranslateMessage(LPMSG pMsg)
{
    return FALSE;
}

bool CDialogBase::EndDialog(INT_PTR nResult)
{
    if (NULL == m_hWnd)
    {
        return false;
    }

    if (m_bFakeModel)
    {
        //启用父窗口
        HWND hParent = ::GetParent(m_hWnd);
        if (hParent)
        {
            ::EnableWindow(hParent, TRUE);
        }

        // 投递 WM_QUIT 消息退出消息环 (窗口句柄指定为NULL时, 
        // 该函数的行为类似于对 PostThreadMessage 的调用， 
        // 其中 dwThreadId 参数设置为当前线程的标识符。)
        ::PostMessage(NULL, WM_QUIT, nResult, 0);
    }

    if (m_bModel)
    {
        ::EndDialog(m_hWnd, nResult);
        m_hWnd = NULL;
    }
    else
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = NULL;
    }

    return TRUE;
}

LRESULT CDialogBase::OnClose(WPARAM wParam, LPARAM lParam)
{
    return EndDialog(wParam);
}

LRESULT CDialogBase::DoFakeModal(UINT resourceID, HWND hWndParent, BOOL isShow)
{
    m_bModel = FALSE;
    m_resID = resourceID;
    m_bFakeModel = TRUE;

    m_hWnd = ::CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceID), hWndParent, DialogProc, (LPARAM)this);
    if (NULL == m_hWnd)
    {
        return -1;
    }

    if (isShow)
    {
        ::ShowWindow(m_hWnd, SW_SHOW);
    }

    //禁用父窗口
    HWND hParent = GetParent(m_hWnd);
    if (hParent)
    {
        ::EnableWindow(hParent, FALSE);
    }

    MSG msg = { 0 };
    BOOL bRet = FALSE;

    //如果 hWnd 为 NULL， 则 GetMessage 将检索属于当前线程的任何窗口的消息，以及当前线程的消息队列上 hwnd 值为 NULL 的任何消息，
    //因此，如果 hWnd 为 NULL，则同时处理窗口消息和线程消息。
    //如果 hWnd 为 - 1，则 GetMessage 仅检索当前线程的消息队列中 hwnd 值为 NULL 的消息，
    //即当 hWnd 参数为 NULL 或 PostThreadMessage 时，PostMessage 发布的线程消息。

    while (0 != (bRet = GetMessage(&msg, NULL, 0, 0)))
    {
        if (-1 == bRet)
        {
            break;
        }

        //类捕获消息处理与对话框 TAB 按键处理
        if (PreTranslateMessage(&msg))
        {
            continue;
        }

        //类捕获消息处理与对话框 TAB 按键处理
        if (::IsDialogMessage(m_hWnd, &msg))
        {
            continue;
        }

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    return msg.wParam;
}

INT_PTR WINAPI CDialogBase::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDialogBase* pThis = reinterpret_cast<CDialogBase*>(GetProp(hWnd, _T("this")));

    if (pThis)
    {
        return pThis->DialogProc(uMsg, wParam, lParam);
    }

    if (WM_INITDIALOG == uMsg)
    {
        pThis = reinterpret_cast<CDialogBase*>(lParam);
        pThis->m_hWnd = hWnd;
        SetProp(hWnd, _T("this"), pThis);
        pThis->DialogProc(uMsg, wParam, lParam);

        if (pThis->m_bCenter)
        {
            pThis->MoveToCenter(::GetParent(hWnd));
        }

        return (INT_PTR)TRUE;
    }

    return (INT_PTR)FALSE;
}

INT_PTR CDialogBase::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const WND_DLG_MSGMAP* pEntry = GetMessageMap();
    LRESULT lpResult = NULL;

    while (pEntry)
    {
        const WND_DLG_MSGMAP_ENTRY* lpEntries = pEntry->lpEntries;
        while (lpEntries->m_pfn)
        {
            if (uMsg == lpEntries->m_nMessage)
            {
                if (WM_COMMAND == uMsg)
                {
                    WORD wID = LOWORD(wParam);
                    //WORD wNotify = HIWORD(wParam);

                    if (wID == lpEntries->m_nID)
                    {
                        lpResult = (this->*lpEntries->m_pfn)(wParam, lParam);
                        break;
                    }
                }
                else
                {
                    lpResult = (this->*lpEntries->m_pfn)(wParam, lParam);
                    break;
                }
            }

            lpEntries++;
        }

        //子类处理了则返回
        if (lpResult)
        {
            ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, lpResult);
            return lpResult;
        }

        //获取基类的消息映射
        pEntry = pEntry->pfnGetBaseMap();
    }

    return FALSE;
}

void CDialogBase::SetResourceID(UINT resourceID)
{
    m_resID = resourceID;
}

void CDialogBase::SetLanguageID(LANGID langID)
{
    m_langId = langID;
}

void CDialogBase::ShowWindow(int cmdShow)
{
    ::ShowWindow(m_hWnd, cmdShow);
}

bool CDialogBase::IsVisiable() const
{
    return ::IsWindowVisible(m_hWnd);
}

void CDialogBase::MoveToCenter(HWND hParent)
{
    RECT rectParent = { 0 };
    RECT rectChild = { 0 };
    RECT rectWork = { 0 };
    LONG dwParentW = 0;
    LONG dwParentH = 0;
    LONG dwChildW = 0;
    LONG dwChildH = 0;
    LONG dwChildX = 0;
    LONG dwChildY = 0;
    ::GetWindowRect(m_hWnd, &rectChild);

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWork, 0);

    if (NULL == hParent)
    {
        hParent = ::GetDesktopWindow();
    }

    ::GetWindowRect(hParent, &rectParent);

    dwParentW = rectParent.right - rectParent.left;
    dwParentH = rectParent.bottom - rectParent.top;
    dwChildW = rectChild.right - rectChild.left;
    dwChildH = rectChild.bottom - rectChild.top;
    dwChildX = rectParent.left + (dwParentW - dwChildW) / 2;
    dwChildY = rectParent.top + (dwParentH - dwChildH) / 2;

    if (dwChildX < 0)
    {
        dwChildX = 0;
    }

    if (dwChildY < 0)
    {
        dwChildY = 0;
    }

    if ((dwChildX + dwChildW) > rectWork.right)
    {
        dwChildX = rectWork.right - dwChildW;
    }

    if ((dwChildY + dwChildH) > rectWork.bottom)
    {
        dwChildY = rectWork.bottom - dwChildH;
    }

    ::MoveWindow(
        m_hWnd, 
        dwChildX,
        dwChildY,
        dwChildW, 
        dwChildH, 
        TRUE
    );
}

_tstring CDialogBase::LoadString(UINT uID)
{
    _tstring strResult;
    LPTSTR lpData = nullptr;

    do
    {
        //获取资源长度
        LPCTSTR lpCStr = nullptr;
        int nLength = ::LoadString(GetModuleHandle(NULL), uID, (LPWSTR)&lpCStr, 0);
        if (0 == nLength)
        {
            break;
        }

        //分配字符串缓冲
        nLength += 1;
        lpData = (LPTSTR)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, nLength * sizeof(TCHAR));
        if (NULL == lpData)
        {
            break;
        }

        //拷贝
        ::StringCchCopy(lpData, nLength, lpCStr);
        strResult = lpData;

    } while (false);

    if (NULL != lpData)
    {
        ::HeapFree(::GetProcessHeap(), 0, lpData);
    }

    return strResult;
}

_tstring CDialogBase::GetItemString(DWORD dwID)
{
    _tstring strResult;
    LPTSTR lpData = nullptr;

    do
    {
        DWORD dwLength = GetWindowTextLength(GetDlgItem(m_hWnd, dwID));
        if (0 == dwLength)
        {
            break;
        }

        dwLength += 1;
        lpData = (LPTSTR)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
        if (NULL == lpData)
        {
            break;
        }

        ::GetDlgItemText(m_hWnd, dwID, lpData, dwLength);
        strResult = lpData;

    } while (false);


    if (NULL != lpData)
    {
        ::HeapFree(::GetProcessHeap(), 0, lpData);
    }

    return strResult;
}

void CDialogBase::SetItemString(DWORD dwID, const _tstring& strText)
{
    ::SetDlgItemText(m_hWnd, dwID, strText.c_str());
}

LRESULT CDialogBase::OnLanguageChange(WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}

VOID CDialogBase::ModifyMenuText(HMENU hMnu, UINT uPosition, LPCTSTR lpNewItem, BOOL fByPos)
{
    MENUITEMINFO mii = { 0 };
    TCHAR szBuf[MAX_PATH] = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_DATA;
    mii.fType = MFT_STRING;
    mii.dwTypeData = szBuf;
    mii.cch = _countof(szBuf);
    GetMenuItemInfo(hMnu, uPosition, fByPos, &mii);
    StringCchCopy(szBuf, _countof(szBuf), lpNewItem);
    SetMenuItemInfo(hMnu, uPosition, fByPos, &mii);
}

VOID CDialogBase::ModifyMenuText(HMENU hMnu, UINT uPosition, UINT resourceID, BOOL fByPos)
{
    ModifyMenuText(hMnu, uPosition, LoadString(resourceID).c_str(), fByPos);
}

VOID CDialogBase::ModifyMenuState(HMENU hMnu, UINT uPosition, UINT fState/* = MFS_ENABLED*/, BOOL fByPos/* = TRUE*/)
{
    MENUITEMINFO mii = { 0 };
    TCHAR szBuf[MAX_PATH] = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_DATA;
    mii.fType = MFT_STRING;
    mii.dwTypeData = szBuf;
    mii.cch = _countof(szBuf);
    GetMenuItemInfo(hMnu, uPosition, fByPos, &mii);
    mii.fState = fState;
    SetMenuItemInfo(hMnu, uPosition, fByPos, &mii);
}

HWND CDialogBase::GetWndHandle() const
{
    if (::IsWindow(m_hWnd))
    {
        return m_hWnd;
    }

    return NULL;
}

bool CDialogBase::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

LRESULT CDialogBase::SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
}