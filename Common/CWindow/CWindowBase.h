#pragma once

#define OEMRESOURCE

#include <Windows.h>
#include <tchar.h>
#include <string>
#include <commctrl.h>
#include <vector>

#pragma warning(disable:4100)

#ifdef UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CWindowBase;

typedef LRESULT(CWindowBase::* pMapFn)(WPARAM wParam, LPARAM lParam);

struct WND_MSGMAP_ENTRY
{
    pMapFn m_pfn;
    UINT m_nMessage;
    UINT m_nID;
    WND_MSGMAP_ENTRY(UINT uMsg, pMapFn pfn) : m_nMessage(uMsg), m_pfn(pfn), m_nID(0) {}
    WND_MSGMAP_ENTRY(UINT uMsg, UINT nID, pMapFn pfn) : m_nMessage(uMsg), m_nID(nID), m_pfn(pfn) {}
};

struct WND_MSGMAP
{
    const WND_MSGMAP* (*pfnGetBaseMap)();
    const WND_MSGMAP_ENTRY* lpEntries;
};

#ifndef wnd_msg
#define wnd_msg
#endif

#define DECLARE_WND_MESSAGE_MAP()                                   \
protected:                                                      \
static const WND_MSGMAP* GetThisMessageMap();                   \
virtual const WND_MSGMAP* GetMessageMap() const;                \

#define BEGIN_WND_MESSAGE_MAP(theClass, baseClass)                  \
const WND_MSGMAP* theClass::GetMessageMap() const               \
{                                                               \
    return GetThisMessageMap();                                 \
}                                                               \
const WND_MSGMAP* theClass::GetThisMessageMap()                 \
{                                                               \
    typedef baseClass TheBaseClass;					            \
static const WND_MSGMAP_ENTRY _messageEntries[] =               \
{                                                               \

#define ON_WND_MESSAGE(message, memberFxn)                          \
WND_MSGMAP_ENTRY(message, static_cast< LRESULT (CWindowBase::*)(WPARAM, LPARAM) >(memberFxn)),


#define ON_WND_COMMAND(_id, memberFxn)                          \
WND_MSGMAP_ENTRY(WM_COMMAND, _id, static_cast< LRESULT (CWindowBase::*)(WPARAM, LPARAM) >(memberFxn)),

#define END_WND_MESSAGE_MAP()  WND_MSGMAP_ENTRY(0, NULL) };         \
static const WND_MSGMAP messageMap =                            \
{&TheBaseClass::GetThisMessageMap , &_messageEntries[0] };      \
return &messageMap;                                             \
}                                                               \

#define DECLARE_WND_BASE_MESSAGE_MAP()                              \
protected:                                                      \
static const WND_MSGMAP* GetThisMessageMap();                   \
virtual const WND_MSGMAP* GetMessageMap() const;                \
static const WND_MSGMAP* GetBaseMessageMap();                   \

#define BEGIN_WND_BASE_MESSAGE_MAP(theClass)                        \
typedef theClass TheThisClass;					                \
const WND_MSGMAP* TheThisClass::GetMessageMap() const           \
{                                                               \
    return GetThisMessageMap();                                 \
}                                                               \
const WND_MSGMAP* TheThisClass::GetBaseMessageMap()             \
{                                                               \
    return NULL;                                                \
}                                                               \
const WND_MSGMAP* TheThisClass::GetThisMessageMap()             \
{                                                               \
static const WND_MSGMAP_ENTRY _messageEntries[] =               \
{                                                               \

#define END_WND_BASE_MESSAGE_MAP()  WND_MSGMAP_ENTRY(0, NULL) };    \
static const WND_MSGMAP messageMap =                            \
{&TheThisClass::GetBaseMessageMap, &_messageEntries[0] };       \
return &messageMap;                                             \
}                                                               \

typedef struct _SUBCLASS_WND_INFO
{
    HWND hWnd = NULL;
    UINT_PTR uID = (UINT_PTR)(-1);

    _SUBCLASS_WND_INFO(HWND hWnd, UINT_PTR uID)
    {
        this->hWnd = hWnd;
        this->uID = uID;
    }

}SUBCLASS_WND_INFO;

class CWindowBase
{
public:

    CWindowBase();
    CWindowBase(const CWindowBase& r) = delete;
    ~CWindowBase();

    //注册窗口类
    virtual BOOL Register(_tstring strClassName);

    //超类化窗口
    virtual BOOL Superclass(_tstring strTargetClass, _tstring strClassName);

    //子类化窗口
    virtual BOOL Subclass(HWND hWnd, UINT_PTR uId);
    BOOL RemoveSubclass();

    //创建窗口
    virtual BOOL Create(
        HWND hParent,
        _tstring strTitle,
        int nCmdShow = SW_SHOW,
        DWORD dwStyle = WS_OVERLAPPEDWINDOW,
        DWORD dwExStyle = 0,
        int dwX = 0,
        int dwY = 0,
        int dwWidth = 640,
        int dwHeight = 480,
        bool bCenter = true,
        HMENU hMenu = nullptr
    );

    //执行消息环
    UINT_PTR Run();

public:

    HWND GetWndHandle() const;
    BOOL PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    VOID ShowWindow(int cmdShow = SW_SHOW);
    BOOL IsVisiable() const;
    VOID MoveToCenter(HWND hParent);
    _tstring LoadString(UINT resourceID);

    VOID ModifyMenuText(HMENU hMnu, UINT uPosition, LPCTSTR lpNewItem, BOOL fByPos = TRUE);
    VOID ModifyMenuText(HMENU hMnu, UINT uPosition, UINT resourceID, BOOL fByPos = TRUE);

    operator HWND() const
    {
        return m_hWnd;
    }

protected:

    wnd_msg LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    wnd_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);
    wnd_msg LRESULT OnErasebkgnd(WPARAM wParam, LPARAM lParam);
    wnd_msg LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);

private:

    static LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    LRESULT ThisWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:

    virtual BOOL PreTranslateMessage(LPMSG pMsg);

protected:
    _tstring    m_ClassName;
    HWND        m_hWnd;
    BOOL        m_bCenter;
    WNDPROC     m_SuperProc = nullptr;
    UINT_PTR    m_uIdSubclass = (UINT_PTR)(-1);

    DECLARE_WND_BASE_MESSAGE_MAP()
};