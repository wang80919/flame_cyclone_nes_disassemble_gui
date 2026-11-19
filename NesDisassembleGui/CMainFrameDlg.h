#pragma once
#include "CWindow/CDialogBase.h"
#include "Win32Utils/CIniHelper.h"

#define CONFIG_REG_KEY                                      HKCU
#define CONFIG_ROOT_PATH                                    _T("Software\\FCDisassemble")
#define CONFIG_LOAD_ADDR                                    _T("LoadAddr")
#define CONFIG_LABEL_NAME                                   _T("LabelName")
#define CONFIG_ADDR_LIST                                    _T("AddrList")
#define CONFIG_DATA_LIST                                    _T("DataList")
#define CONFIG_INPUT_TEXT                                   _T("InputText")
#define CONFIG_OUTPUT_TEXT                                  _T("OutputText")
#define CONFIG_LANG_ID                                      _T("Language")
#define CONFIG_OUTPUT_SYNTAX                                _T("Syntax")
#define CONFIG_SHOW_ADDR                                    _T("ShowAddr")
#define CONFIG_OUT_PATH                                     _T("OutPath")


#define STRING_BUF_SIZE                                     (1024 * 1024 * 64)

class CMainFrameDlg :
    public CDialogBase
{
public:

    CMainFrameDlg();
    ~CMainFrameDlg();

    dlg_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnLanguageChange(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnLangZh(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnLangEn(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnSyntaxNesasm(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnSyntax6502Simulator(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnReset(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnDragFiles(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnDisassemble(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnPasteInput(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnClearInput(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnCopyOutput(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

    dlg_msg LRESULT OnExit(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnColor(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnShowAddr(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnFormat(WPARAM wParam, LPARAM lParam);

private:

    void SaveConfig();

    DECLARE_DLG_MESSAGE_MAP()

private:

    CIniHelper m_cfg;
    HBRUSH m_hBrush = nullptr;
    HBRUSH m_hStaticBrush = nullptr;
    HWND m_hAddrListTip = nullptr;
    HWND m_hDataListTip = nullptr;
    HWND m_hLoadAddrTip = nullptr;
    DWORD m_dwSyntaxType = 0;
    bool m_bShowAddr = false;
};


