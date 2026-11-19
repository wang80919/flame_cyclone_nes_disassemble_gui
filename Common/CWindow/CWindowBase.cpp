#include "CWindowBase.h"
#include <strsafe.h>
#include <Winuser.h>

BEGIN_WND_BASE_MESSAGE_MAP(CWindowBase)
    ON_WND_MESSAGE(WM_CREATE, &CWindowBase::OnCreate)
    ON_WND_MESSAGE(WM_CLOSE, &CWindowBase::OnClose)
    ON_WND_MESSAGE(WM_ERASEBKGND, &CWindowBase::OnErasebkgnd)
    ON_WND_MESSAGE(WM_DESTROY, &CWindowBase::OnDestroy)
END_WND_BASE_MESSAGE_MAP()

CWindowBase::CWindowBase()
    :
    m_hWnd(NULL),
    m_bCenter(FALSE)
{
}

CWindowBase::~CWindowBase()
{
    if (m_hWnd)
    {
        RemoveSubclass();
        ::DestroyWindow(m_hWnd);
    }
}

BOOL CWindowBase::Register(_tstring strClassName)
{
    m_ClassName = strClassName;

    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = ::GetModuleHandle(NULL);
    wcex.hIcon = (HICON)::LoadImage(NULL, MAKEINTRESOURCE(OIC_SAMPLE), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wcex.hCursor = (HCURSOR)::LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wcex.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = m_ClassName.c_str();
    wcex.hIconSm = (HICON)::LoadImage(NULL, MAKEINTRESOURCE(OIC_SAMPLE), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);

    if (::RegisterClassEx(&wcex))
    {
        return TRUE;
    }

    if (ERROR_CLASS_ALREADY_EXISTS == ::GetLastError())
    {
        return TRUE;
    }

    return FALSE;
}

BOOL CWindowBase::Superclass(_tstring strTargetClass, _tstring strNewClassName)
{
    m_ClassName = strNewClassName;

    WNDCLASSEX wcex = { 0 };
    if (!GetClassInfoEx(NULL, strTargetClass.c_str(), &wcex))
    {
        return FALSE;
    }

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    //wcex.lpfnWndProc = WindowProc;
    //wcex.cbClsExtra = 0;
    //wcex.cbWndExtra = 0;
    wcex.hInstance = ::GetModuleHandle(NULL);
    wcex.hIcon = (HICON)::LoadImage(NULL, MAKEINTRESOURCE(OIC_SAMPLE), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wcex.hCursor = (HCURSOR)::LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    wcex.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = m_ClassName.c_str();
    wcex.hIconSm = (HICON)::LoadImage(NULL, MAKEINTRESOURCE(OIC_SAMPLE), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);

    m_SuperProc = wcex.lpfnWndProc;
    wcex.lpfnWndProc = WindowProc;
    if (::RegisterClassEx(&wcex))
    {
        return TRUE;
    }

    if (ERROR_CLASS_ALREADY_EXISTS == ::GetLastError())
    {
        return TRUE;
    }

    return FALSE;
}

BOOL CWindowBase::Subclass(HWND hWnd, UINT_PTR uId)
{
    //已经子类化过
    if (m_hWnd && -1 != m_uIdSubclass)
    {
        return FALSE;
    }

    if (::SetWindowSubclass(hWnd, SubclassProc, uId, (DWORD_PTR)this))
    {
        m_hWnd = hWnd;
        m_uIdSubclass = uId;
        return TRUE;
    }

    return FALSE;
}

BOOL CWindowBase::RemoveSubclass()
{
    if (m_hWnd && -1 != m_uIdSubclass)
    {
        RemoveWindowSubclass(m_hWnd, SubclassProc, m_uIdSubclass);
        m_uIdSubclass = (UINT_PTR)(-1);
    }

    return true;
}

BOOL CWindowBase::Create(
    HWND hParent, 
    _tstring strTitle,
    int nCmdShow/* = SW_SHOW*/,
    DWORD dwStyle/* = WS_OVERLAPPEDWINDOW*/,
    DWORD dwExStyle/* = 0*/,
    int dwX/* = 0*/,
    int dwY/* = 0*/,
    int dwWidth/* = 640*/,
    int dwHeight/* = 480*/,
    bool bCenter/* = true*/,
    HMENU hMenu/* = nullptr*/
)
{
    m_bCenter = bCenter;
    RECT rectWnd = { 0, 0, dwWidth, dwHeight };

    ::AdjustWindowRect(&rectWnd, dwStyle, nullptr != hMenu);

    m_hWnd = ::CreateWindowEx(
        dwExStyle, 
        m_ClassName.c_str(), 
        strTitle.c_str(), 
        dwStyle,
        dwX,
        dwY,
        rectWnd.right - rectWnd.left, 
        rectWnd.bottom - rectWnd.top, 
        hParent,
        hMenu,
        NULL, 
        this);

    if (!m_hWnd)
    {
        return FALSE;
    }

    ::ShowWindow(m_hWnd, nCmdShow);
    ::UpdateWindow(m_hWnd);

    return TRUE;
}

UINT_PTR CWindowBase::Run()
{
    MSG msg = { 0 };

    while (::GetMessage(&msg, nullptr, 0, 0))
    {
        if (PreTranslateMessage(&msg))
        {
            continue;
        }

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    return msg.wParam;
}

BOOL CWindowBase::PreTranslateMessage(LPMSG pMsg)
{
    return FALSE;
}

LRESULT CWindowBase::OnCreate(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT CWindowBase::OnClose(WPARAM wParam, LPARAM lParam)
{
    RemoveSubclass();
    ::DestroyWindow(m_hWnd);
    m_hWnd = nullptr;
    return 0;
}

LRESULT CWindowBase::OnErasebkgnd(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT CWindowBase::OnDestroy(WPARAM wParam, LPARAM lParam)
{
    ::PostQuitMessage(0);
    return 0;
}

LRESULT WINAPI CWindowBase::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWindowBase* pThis = (CWindowBase*)::GetProp(hWnd, _T("this"));
    //CWindowBase* pThis = (CWindowBase*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pThis)
    {
        return pThis->ThisWindowProc(uMsg, wParam, lParam);
    }

    if (WM_NCCREATE == uMsg)
    {
        pThis = (CWindowBase*)((LPCREATESTRUCT)lParam)->lpCreateParams;
        if (pThis)
        {
            pThis->m_hWnd = hWnd;
            //给窗口绑定this指针
            ::SetProp(hWnd, _T("this"), pThis);
            //::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            if (pThis->m_bCenter)
            {
                pThis->MoveToCenter(NULL);
            }

            return pThis->WindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CWindowBase::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    CWindowBase* pThis = (CWindowBase*)dwRefData;
    if (pThis)
    {
        return pThis->ThisWindowProc(uMsg, wParam, lParam);
    }

    return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CWindowBase::ThisWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const WND_MSGMAP* pEntry = GetMessageMap();
    LRESULT lResult = 0;

    while (pEntry)
    {
        const WND_MSGMAP_ENTRY* lpEntries = pEntry->lpEntries;
        while (lpEntries->m_pfn)
        {
            if (uMsg == lpEntries->m_nMessage)
            {
                //处理 WM_COMMAND 消息
                if (WM_COMMAND == uMsg)
                {
                    if (wParam == lpEntries->m_nID)
                    {
                        lResult = (this->*lpEntries->m_pfn)(wParam, lParam);
                        break;
                    }
                }
                else
                {
                    lResult = (this->*lpEntries->m_pfn)(wParam, lParam);
                    break;
                }
            }

            lpEntries++;
        }

        //消息已经处理
        if (lResult)
        {
            //不希望继续路由
            return lResult;
        }

        //获取基类的消息映射
        pEntry = pEntry->pfnGetBaseMap();
    }

    //超类化处理
    if (m_SuperProc)
    {
        return m_SuperProc(m_hWnd, uMsg, wParam, lParam);
    }

    //默认系统处理
    return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

VOID CWindowBase::ShowWindow(int cmdShow)
{
    ::ShowWindow(m_hWnd, cmdShow);
}

BOOL CWindowBase::IsVisiable() const
{
    return ::IsWindowVisible(m_hWnd);
}

void CWindowBase::MoveToCenter(HWND hParent)
{
    RECT rectParent = { 0 };
    RECT rectChild = { 0 };
    LONG dwParentW = 0;
    LONG dwParentH = 0;
    LONG dwChildW = 0;
    LONG dwChildH = 0;

    if (NULL == hParent)
    {
        hParent = ::GetDesktopWindow();
    }

    ::GetWindowRect(m_hWnd, &rectChild);
    ::GetWindowRect(hParent, &rectParent);

    dwParentW = rectParent.right - rectParent.left;
    dwParentH = rectParent.bottom - rectParent.top;
    dwChildW = rectChild.right - rectChild.left;
    dwChildH = rectChild.bottom - rectChild.top;

    ::MoveWindow(m_hWnd, rectParent.left + (dwParentW - dwChildW) / 2,
        rectParent.top + (dwParentH - dwChildH) / 2, dwChildW, dwChildH, TRUE);
}

_tstring CWindowBase::LoadString(UINT uID)
{
    static TCHAR szBuf[1024] = { 0 };
    ::LoadString(GetModuleHandle(NULL), uID, szBuf, _countof(szBuf));
    return szBuf;
}

VOID CWindowBase::ModifyMenuText(HMENU hMnu, UINT uPosition, LPCTSTR lpNewItem, BOOL fByPos)
{
    MENUITEMINFO mii = { 0 };
    TCHAR szBuf[MAX_PATH] = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_DATA;
    mii.fType = MFT_STRING;
    mii.dwTypeData = szBuf;
    mii.cch = _countof(szBuf);

    ::GetMenuItemInfo(hMnu, uPosition, fByPos, &mii);
    ::StringCchCopy(szBuf, _countof(szBuf), lpNewItem);
    ::SetMenuItemInfo(hMnu, uPosition, fByPos, &mii);
}

VOID CWindowBase::ModifyMenuText(HMENU hMnu, UINT uPosition, UINT resourceID, BOOL fByPos)
{
    ModifyMenuText(hMnu, uPosition, LoadString(resourceID).c_str(), fByPos);
}

HWND CWindowBase::GetWndHandle() const
{
    return m_hWnd;
}

BOOL CWindowBase::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

LRESULT CWindowBase::SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
}