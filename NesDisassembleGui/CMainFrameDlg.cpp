#include "CMainFrameDlg.h"
#include "resource.h"
#include "6502/C6502Introduction.h"
#include "Win32Utils/CFileHelper.h"
#include "Win32Utils/CStrUtils.h"
#include "Win32Utils/CBytesUtils.h"
#include "Win32Utils/CPathUtils.h"
#include "Win32Utils/CClipboardUtils.h"
//#include "Win32Utils/CRegUtils.h"
#include <commctrl.h>
#include <strsafe.h>
#include <Windowsx.h>

#define WM_LANGUAGE_CHANGE                                  (WM_USER + 2)

const UINT WM_TASKBARCREATED = ::RegisterWindowMessage(_T("TaskbarCreated"));

BEGIN_DLG_MESSAGE_MAP(CMainFrameDlg, CDialogBase)

    ON_DLG_MESSAGE(WM_INITDIALOG, &CMainFrameDlg::OnInitDialog)
    ON_DLG_MESSAGE(WM_DROPFILES, &CMainFrameDlg::OnDragFiles)
    ON_DLG_MESSAGE(WM_LANGUAGE_CHANGE, &CMainFrameDlg::OnLanguageChange)
    ON_DLG_MESSAGE(WM_CLOSE, &CMainFrameDlg::OnClose)
    ON_DLG_MESSAGE(WM_NOTIFY, &CMainFrameDlg::OnNotify)
    //ON_DLG_MESSAGE(WM_CTLCOLORBTN, &CMainFrameDlg::OnColor)
    //ON_DLG_MESSAGE(WM_CTLCOLOREDIT, &CMainFrameDlg::OnColor)
    //ON_DLG_MESSAGE(WM_CTLCOLORDLG, &CMainFrameDlg::OnColor)
    //ON_DLG_MESSAGE(WM_CTLCOLORLISTBOX, &CMainFrameDlg::OnColor)
    //ON_DLG_MESSAGE(WM_CTLCOLORSCROLLBAR, &CMainFrameDlg::OnColor)
    
    ON_DLG_COMMAND(IDC_BUTTON_DISASSEMBLE, &CMainFrameDlg::OnDisassemble)
    ON_DLG_COMMAND(IDC_BUTTON_PASTE, &CMainFrameDlg::OnPasteInput)
    ON_DLG_COMMAND(IDC_BUTTON_CLEAR, &CMainFrameDlg::OnClearInput)
    ON_DLG_COMMAND(IDC_BUTTON_COPY, &CMainFrameDlg::OnCopyOutput)
    ON_DLG_COMMAND(ID_FILE_EXIT, &CMainFrameDlg::OnExit)
    ON_DLG_COMMAND(ID_LANG_ZH_CN, &CMainFrameDlg::OnLangZh)
    ON_DLG_COMMAND(ID_LANG_EN_US, &CMainFrameDlg::OnLangEn)
    ON_DLG_COMMAND(ID_SYNTAX_NESASM, &CMainFrameDlg::OnSyntaxNesasm)
    ON_DLG_COMMAND(ID_SYNTAX_6502_SIMULATOR, &CMainFrameDlg::OnSyntax6502Simulator)
    ON_DLG_COMMAND(ID_RESET, &CMainFrameDlg::OnReset)
    ON_DLG_COMMAND(IDC_CHECK_SHOW_ADDR, &CMainFrameDlg::OnShowAddr)
    ON_DLG_COMMAND(IDC_BUTTON_FORMAT, &CMainFrameDlg::OnFormat)

END_DLG_MESSAGE_MAP()

CMainFrameDlg::CMainFrameDlg()
{

}

CMainFrameDlg::~CMainFrameDlg()
{
    if (m_hBrush)
    {
        ::DeleteObject(m_hBrush);
    }
}

HWND CreateToolTip(int toolID, HWND hDlg, PTSTR pszText)
{
    if (!toolID || !hDlg || !pszText)
    {
        return FALSE;
    }

    // Get the window of the tool.
    HWND hwndTool = GetDlgItem(hDlg, toolID);

    // Create the tooltip. g_hInst is the global instance handle.
    HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        hDlg, NULL,
        ::GetModuleHandle(NULL), NULL);

    if (!hwndTool || !hwndTip)
    {
        return (HWND)NULL;
    }

    // Associate the tooltip with the tool.
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hDlg;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = LPSTR_TEXTCALLBACK;
    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

    return hwndTip;
}

LRESULT CMainFrameDlg::OnNotify(WPARAM wParam, LPARAM lParam)
{
    LPNMHDR lpNmhdr = (LPNMHDR)lParam;
    if (TTN_GETDISPINFO == lpNmhdr->code)
    {
        SetThreadUILanguage(m_langId);

        LPNMTTDISPINFO lpNmTtd = (LPNMTTDISPINFO)lParam;

        if (m_hAddrListTip == lpNmhdr->hwndFrom)
        {
            StringCchPrintf(lpNmTtd->lpszText, 80,
                LoadString(IDS_STRING_ADDR_LIST_TIPS).c_str()
            );
        }

        if (m_hDataListTip == lpNmhdr->hwndFrom)
        {
            StringCchPrintf(lpNmTtd->lpszText, 80,
                LoadString(IDS_STRING_DATA_LIST_TIPS).c_str()
            );
        }

        if (m_hLoadAddrTip == lpNmhdr->hwndFrom)
        {
            StringCchPrintf(lpNmTtd->lpszText, 80,
                LoadString(IDS_STRING_LOAD_ADDR_TIPS).c_str()
            );
        }
    }

    return TRUE;
}

LRESULT CMainFrameDlg::OnReset(WPARAM wParam, LPARAM lParam)
{
    SetDlgItemText(m_hWnd, IDC_EDIT_LOAD_ADDR, _T("FA8E"));
    SetDlgItemText(m_hWnd, IDC_EDIT_LABEL_NAME, _T("Label_"));
    SetDlgItemText(m_hWnd, IDC_EDIT_ADDR_CFG, _T("FA8E,FA9D,FAA2"));
    SetDlgItemText(m_hWnd, IDC_EDIT_DATA_CFG, _T("FAAD,4;FAB1,6"));
    SetDlgItemText(m_hWnd, IDC_EDIT_INPUT, _T("D8 78 A2 FF 9A A9 00 8D 00 20 8D 01 20 A2 02 2C\r\n02 20 10 FB 2C 02 20 30 FB CA D0 F3 4C AA FA 8E\r\nFA 9D FA A2 FA AA FA FA FF"));
    
    if (0 != wParam)
    {
        OnDisassemble(0, 0);
    }

    return TRUE;
}

LRESULT CMainFrameDlg::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
    CHANGEFILTERSTRUCT cfs = { 0 };
    cfs.cbSize = sizeof(cfs);
    ChangeWindowMessageFilterEx(m_hWnd, WM_DROPFILES, MSGFLT_ALLOW, &cfs);
    ChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA, MSGFLT_ALLOW, &cfs);
    ChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA - 1, MSGFLT_ALLOW, &cfs);

    // 设置图标
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
    if (hIcon)
    {
        ::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        ::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    m_hAddrListTip = CreateToolTip(IDC_EDIT_ADDR_CFG, m_hWnd, LPSTR_TEXTCALLBACK);
    m_hDataListTip = CreateToolTip(IDC_EDIT_DATA_CFG, m_hWnd, LPSTR_TEXTCALLBACK);
    m_hLoadAddrTip = CreateToolTip(IDC_EDIT_LOAD_ADDR, m_hWnd, LPSTR_TEXTCALLBACK);

    Edit_LimitText(GetDlgItem(m_hWnd,IDC_EDIT_INPUT), STRING_BUF_SIZE);
    Edit_LimitText(GetDlgItem(m_hWnd, IDC_EDIT_OUTPUT), STRING_BUF_SIZE);

    OnReset(0, 0);

    m_cfg.SetPath(_T("config\\config.ini"), false);

    m_langId = m_cfg.GetNumber(_T("config"), CONFIG_LANG_ID, -1);
    if (UINT16_MAX != m_langId)
    {
        if (LANG_CHINESE == PRIMARYLANGID(m_langId))
        {
            ::PostMessage(m_hWnd, WM_COMMAND, ID_LANG_ZH_CN, 0);
        }
        else
        {
            ::PostMessage(m_hWnd, WM_COMMAND, ID_LANG_EN_US, 0);
        }
    }
    else
    {
        LANGID langID = GetThreadUILanguage();
        if (LANG_CHINESE == PRIMARYLANGID(langID))
        {
            ::PostMessage(m_hWnd, WM_COMMAND, ID_LANG_ZH_CN, 0);
        }
        else
        {
            ::PostMessage(m_hWnd, WM_COMMAND, ID_LANG_EN_US, 0);
        }
    }

    m_dwSyntaxType = m_cfg.GetNumber(_T("config"), CONFIG_OUTPUT_SYNTAX, 0);
    if (0 == m_dwSyntaxType)
    {
        CheckMenuItem(GetMenu(m_hWnd), ID_SYNTAX_NESASM, MF_BYCOMMAND | MF_CHECKED);
    }
    else
    {
        CheckMenuItem(GetMenu(m_hWnd), ID_SYNTAX_6502_SIMULATOR, MF_BYCOMMAND | MF_CHECKED);
    }

    _tstring strCfgDir = CPathUtils::GetCurrentModuleDir() + _T("\\config");
    CPathUtils::CreateDir(strCfgDir);

    _tstring strValue;

    SetItemString(IDC_EDIT_LOAD_ADDR, m_cfg.GetString(_T("config"), CONFIG_LOAD_ADDR));
    SetItemString(IDC_EDIT_LABEL_NAME, m_cfg.GetString(_T("config"), CONFIG_LABEL_NAME));
    SetItemString(IDC_EDIT_OUT_PATH, m_cfg.GetString(_T("config"), CONFIG_OUT_PATH));
    SetItemString(IDC_EDIT_ADDR_CFG, CStrUtils::FromFile(strCfgDir + _T("\\addr_list.txt"), true));
    SetItemString(IDC_EDIT_DATA_CFG, CStrUtils::FromFile(strCfgDir + _T("\\data_list.txt"), true));
    SetItemString(IDC_EDIT_INPUT, CStrUtils::FromFile(strCfgDir + _T("\\input.txt"), true));
    SetItemString(IDC_EDIT_OUTPUT, CStrUtils::FromFile(strCfgDir + _T("\\output.txt"), true));

    m_bShowAddr = m_cfg.GetNumber(_T("config"), CONFIG_SHOW_ADDR, 0);
    if (m_bShowAddr)
    {
        ::CheckDlgButton(m_hWnd, IDC_CHECK_SHOW_ADDR, BST_CHECKED);
    }

    return TRUE;
}

LRESULT CMainFrameDlg::OnLanguageChange(WPARAM wParam, LPARAM lParam)
{
    m_langId = (LANGID)wParam;
    SetThreadUILanguage(m_langId);

    SetItemString(IDC_STATIC_LOAD_ADDR, LoadString(IDS_STRING_LOAD_ADDR));
    SetItemString(IDC_STATIC_LABEL_NAME, LoadString(IDS_STRING_LABEL_NAME));
    SetItemString(IDC_STATIC_ADDR_LIST, LoadString(IDS_STRING_ADDR_LIST));
    SetItemString(IDC_STATIC_DATA_LIST, LoadString(IDS_STRING_DATA_LIST));
    SetItemString(IDC_STATIC_INPUT, LoadString(IDS_STRING_INPUT));
    SetItemString(IDC_STATIC_OUTPUT, LoadString(IDS_STRING_OUTPUT));
    SetItemString(IDC_BUTTON_DISASSEMBLE, LoadString(IDS_STRING_DISASSEMBLE));
    SetItemString(IDC_BUTTON_COPY, LoadString(IDS_STRING_COPY));
    SetItemString(IDC_CHECK_SHOW_ADDR, LoadString(IDS_STRING_SHOW_ADDR));
    SetItemString(IDC_BUTTON_FORMAT, LoadString(IDS_STRING_FORMAT));
    SetItemString(IDC_STATIC_OUT_PATH, LoadString(IDS_STRING_OUT_PATH));

    HMENU hMenu = GetMenu(m_hWnd);
    ModifyMenuText(hMenu, 0, IDS_STRING_FILE, TRUE);
    ModifyMenuText(hMenu, ID_FILE_EXIT, IDS_STRING_EXIT, FALSE);
    ModifyMenuText(hMenu, 1, IDS_STRING_SETTINGS, TRUE);
    ModifyMenuText(GetSubMenu(hMenu, 1), 0, IDS_STRING_LANGUAGE, TRUE);
    ModifyMenuText(GetSubMenu(hMenu, 1), 1, IDS_STRING_OUTPUT_SYNTAX, TRUE);
    ModifyMenuText(hMenu, ID_RESET, IDS_STRING_RESET, FALSE);

    m_cfg.SetNumber(_T("config"), CONFIG_LANG_ID, m_langId);

    ::SetWindowText(m_hWnd, LoadString(IDS_STRING_CAPTION).c_str());

    ::DrawMenuBar(m_hWnd);

    return TRUE;
}

LRESULT CMainFrameDlg::OnSyntaxNesasm(WPARAM wParam, LPARAM lParam)
{
    CheckMenuItem(GetMenu(m_hWnd), ID_SYNTAX_NESASM, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(GetMenu(m_hWnd), ID_SYNTAX_6502_SIMULATOR, MF_BYCOMMAND | MF_UNCHECKED);
    WORD wID = LOWORD(wParam);
    CheckMenuItem(GetMenu(m_hWnd), wID, MF_BYCOMMAND | MF_CHECKED);

    m_dwSyntaxType = 0;
    m_cfg.SetNumber(_T("config"), CONFIG_OUTPUT_SYNTAX, m_dwSyntaxType);
    SendMessage(WM_COMMAND, IDC_BUTTON_DISASSEMBLE, 0);
    return TRUE;
}

LRESULT CMainFrameDlg::OnSyntax6502Simulator(WPARAM wParam, LPARAM lParam)
{
    CheckMenuItem(GetMenu(m_hWnd), ID_SYNTAX_NESASM, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(GetMenu(m_hWnd), ID_SYNTAX_6502_SIMULATOR, MF_BYCOMMAND | MF_UNCHECKED);
    WORD wID = LOWORD(wParam);
    CheckMenuItem(GetMenu(m_hWnd), wID, MF_BYCOMMAND | MF_CHECKED);

    m_dwSyntaxType = 1;
    m_cfg.SetNumber(_T("config"), CONFIG_OUTPUT_SYNTAX, m_dwSyntaxType);
    SendMessage(WM_COMMAND, IDC_BUTTON_DISASSEMBLE, 0);
    return TRUE;
}

LRESULT CMainFrameDlg::OnLangZh(WPARAM wParam, LPARAM lParam)
{
    m_langId = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
    PostMessage(WM_LANGUAGE_CHANGE, m_langId, 0);

    CheckMenuItem(GetMenu(m_hWnd), ID_LANG_ZH_CN, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(GetMenu(m_hWnd), ID_LANG_EN_US, MF_BYCOMMAND | MF_UNCHECKED);
    WORD wID = LOWORD(wParam);
    CheckMenuItem(GetMenu(m_hWnd), wID, MF_BYCOMMAND | MF_CHECKED);
    return TRUE;
}

LRESULT CMainFrameDlg::OnLangEn(WPARAM wParam, LPARAM lParam)
{
    m_langId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    PostMessage(WM_LANGUAGE_CHANGE, m_langId, 0);

    CheckMenuItem(GetMenu(m_hWnd), ID_LANG_ZH_CN, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(GetMenu(m_hWnd), ID_LANG_EN_US, MF_BYCOMMAND | MF_UNCHECKED);
    WORD wID = LOWORD(wParam);
    CheckMenuItem(GetMenu(m_hWnd), wID, MF_BYCOMMAND | MF_CHECKED);
    return TRUE;
}

LRESULT CMainFrameDlg::OnExit(WPARAM wParam, LPARAM lParam)
{
    PostMessage(WM_CLOSE, 0, 0);
    return TRUE;
}

LRESULT CMainFrameDlg::OnClose(WPARAM wParam, LPARAM lParam)
{
    SaveConfig();

    return FALSE;
}

void CMainFrameDlg::SaveConfig()
{
    m_cfg.SetString(_T("config"), CONFIG_LOAD_ADDR, GetItemString(IDC_EDIT_LOAD_ADDR));
    m_cfg.SetString(_T("config"), CONFIG_LABEL_NAME, GetItemString(IDC_EDIT_LABEL_NAME));
    m_cfg.SetString(_T("config"), CONFIG_OUT_PATH, GetItemString(IDC_EDIT_OUT_PATH));
    m_cfg.SetNumber(_T("config"), CONFIG_LANG_ID, m_langId);
    m_cfg.SetNumber(_T("config"), CONFIG_SHOW_ADDR, m_bShowAddr);

    _tstring strCfgDir = CPathUtils::GetCurrentModuleDir() + _T("\\config");
    CStrUtils::ToFile(GetItemString(IDC_EDIT_INPUT), strCfgDir + _T("\\input.txt"));
    CStrUtils::ToFile(GetItemString(IDC_EDIT_OUTPUT), strCfgDir + _T("\\output.txt"));
    CStrUtils::ToFile(GetItemString(IDC_EDIT_ADDR_CFG), strCfgDir + _T("\\addr_list.txt"));
    CStrUtils::ToFile(GetItemString(IDC_EDIT_DATA_CFG), strCfgDir + _T("\\data_list.txt"));
}

dlg_msg LRESULT CMainFrameDlg::OnDragFiles(WPARAM wParam, LPARAM lParam)
{
    HDROP hDrop = (HDROP)wParam;

    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    DragQueryFile(hDrop, 0, szBuf, _countof(szBuf));

    ::DragFinish(hDrop);

    if (CPathUtils::IsArchive(szBuf))
    {
        std::vector<uint8_t> vData;
        if (CBytesUtils::BytesFromFile(vData, szBuf))
        {
            SetItemString(IDC_EDIT_INPUT, CBytesUtils::BytesFormat(vData, 16, _T(" "), _T(""), _T("")));
        }
    }

    return FALSE;
}

LRESULT CMainFrameDlg::OnDisassemble(WPARAM wParam, LPARAM lParam)
{
    _tstring strLoadAddr = GetItemString(IDC_EDIT_LOAD_ADDR);
    _tstring strLabelName = GetItemString(IDC_EDIT_LABEL_NAME);
    _tstring strAddrCfg = GetItemString(IDC_EDIT_ADDR_CFG);
    _tstring strDataCfg = GetItemString(IDC_EDIT_DATA_CFG);
    _tstring strInput = GetItemString(IDC_EDIT_INPUT);

    SaveConfig();
    m_bShowAddr = IsDlgButtonChecked(m_hWnd, IDC_CHECK_SHOW_ADDR);

    std::vector<uint16_t> addrList;
    std::map<uint16_t, DATA_INFO> dataList;
    uint16_t loadAddr = 0x8000;

    _stscanf_s(strLoadAddr.c_str(), _T("%hx"), &loadAddr);

    //解析地址段
    {
        std::vector<_tstring> vStrValue = CStrUtils::SplitStr(strAddrCfg, _T("\r\n"));
        for (const auto& itemAddr : vStrValue)
        {
            std::vector<_tstring> vAddr = CStrUtils::SplitStr(itemAddr, _T(","));
            for (const auto& item : vAddr)
            {
                uint16_t addr = CStrUtils::StrToUInt(item, 16);
                addrList.push_back(addr);
            }
        }

        if (addrList.empty())
        {
            addrList.push_back(loadAddr);
        }
    }

    //解析数据段
    {
        std::vector<_tstring> vStrValue = CStrUtils::SplitStr(strDataCfg, _T("\r\n"));
        for (const auto& itemStr : vStrValue)
        {
            std::vector<_tstring> vAddr = CStrUtils::SplitStr(itemStr, _T(";"));
            for (const auto& itemAddr : vAddr)
            {
                uint32_t addr = 0;
                uint32_t size = 0;
                uint32_t elem = 1;

                _stscanf_s(itemAddr.c_str(), _T("%4x,%4x,%2x"), &addr, &size, &elem);

                if (0 == size)
                {
                    size = 1;
                }

                if (0 == elem || elem > 2)
                {
                    elem = 1;
                }

                dataList.insert(std::make_pair((uint16_t)addr, DATA_INFO((uint16_t)addr, (uint16_t)size, (uint8_t)elem)));
            }
        }
    }

    //分配 64KB 缓冲空间
    std::vector<uint8_t> dataBuffer(1024 * 64, 0xFF);

    LPBYTE lpData = dataBuffer.data();

    std::vector<uint8_t> bytesData = CBytesUtils::BytesFromHexString(strInput);
    memcpy_s(lpData + loadAddr, dataBuffer.size() - loadAddr, bytesData.data(), bytesData.size());

    std::map<uint16_t, C6502Introduction> IntroductionData;
    DisassembleInstructions(IntroductionData, dataList, addrList, lpData, (uint32_t)dataBuffer.size());

    _tstring strOutput;
    if (!IntroductionData.empty())
    {
        strOutput = CStrUtils::Format(_T("    .ORG $%04X\r\n\r\n"), IntroductionData.begin()->first);
    }

    strOutput += GetDisassembleText(IntroductionData, strLabelName, lpData, (uint32_t)dataBuffer.size(), m_bShowAddr, m_dwSyntaxType);

    SetItemString(IDC_EDIT_OUTPUT, strOutput);

    CFileHelper outFile(GetItemString(IDC_EDIT_OUT_PATH), false);
    if (outFile.IsOpen())
    {
        outFile.Clear();
        std::string strAsm = CStrUtils::TStrToU8Str(strOutput);
        outFile.Write(strAsm.c_str(), strAsm.size());
    }

    return TRUE;
}

LRESULT CMainFrameDlg::OnPasteInput(WPARAM wParam, LPARAM lParam)
{
    _tstring strInput;
    if (CClipboardUtils::GetText(strInput))
    {
        SetItemString(IDC_EDIT_INPUT, strInput);
    }

    return TRUE;
}

LRESULT CMainFrameDlg::OnClearInput(WPARAM wParam, LPARAM lParam)
{
    SetItemString(IDC_EDIT_INPUT, _T(""));
    return TRUE;
}

LRESULT CMainFrameDlg::OnCopyOutput(WPARAM wParam, LPARAM lParam)
{
    CClipboardUtils::SetText(GetItemString(IDC_EDIT_OUTPUT));
    return TRUE;
}

LRESULT CMainFrameDlg::OnColor(WPARAM wParam, LPARAM lParam)
{
    HDC hdc = (HDC)wParam;

    if (NULL == m_hBrush)
    {
        m_hBrush = CreateSolidBrush(RGB(77, 77, 77));
    }

    ::SetTextColor(hdc, RGB(250, 250, 250));
    ::SetBkColor(hdc, RGB(77, 77, 77));

    return (LRESULT)m_hBrush;
}

LRESULT CMainFrameDlg::OnShowAddr(WPARAM wParam, LPARAM lParam)
{
    m_bShowAddr = BST_CHECKED == ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_SHOW_ADDR);
    return TRUE;
}

LRESULT CMainFrameDlg::OnFormat(WPARAM wParam, LPARAM lParam)
{
    _tstring strInput = GetItemString(IDC_EDIT_INPUT);
    std::vector<uint8_t> dataBuffer(STRING_BUF_SIZE);
    std::vector<uint8_t> dataString = CBytesUtils::BytesFromHexString(strInput);
    SetItemString(IDC_EDIT_INPUT, CBytesUtils::BytesFormat(dataString, 16, _T(" "), _T(""), _T("")));

    return TRUE;
}
