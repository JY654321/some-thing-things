// CompressDialogWin32.cpp
// 编译：cl /std:c++17 /EHsc CompressDialogWin32.cpp user32.lib gdi32.lib comctl32.lib dwmapi.lib uxtheme.lib

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <string>
#include <vector>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup")

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

// ==================== 控件 ID ====================
#define IDC_ARCHIVE          1001
#define IDC_BROWSE           1002
#define IDC_FORMAT           1003
#define IDC_LEVEL            1004
#define IDC_METHOD           1005
#define IDC_DICTIONARY       1006
#define IDC_ORDER            1007
#define IDC_SOLID            1008
#define IDC_THREADS          1009
#define IDC_MEMUSE           1010
#define IDC_SPLIT            1011
#define IDC_PARAMS           1012
#define IDC_OPTIONS_BTN      1013
#define IDC_UPDATE_MODE      1014
#define IDC_PATH_MODE        1015
#define IDC_SFX              1016
#define IDC_SHARED           1017
#define IDC_DELETE           1018
#define IDC_PASSWORD         1019
#define IDC_SHOW_PWD         1020
#define IDC_ENC_METHOD       1021
#define IDC_ENC_NAMES        1022
#define IDC_MEM_LABEL        1023
#define IDC_MEM_VALUE        1024
#define IDC_THREADS_MAX      1025
#define IDC_MEM_DE_LABEL     1026
#define IDC_MEM_DE_VALUE     1027
#define IDC_OK               1100
#define IDC_CANCEL           1101
#define IDC_HELP             1102

// ==================== 全局深色标志 ====================
bool g_darkMode = false;

// ==================== 深色模式工具函数 ====================
// 尝试开启标题栏深色模式（Win10 1809+ / Win11）
void EnableDarkTitleBar(HWND hwnd)
{
    BOOL dark = TRUE;
    // 20 = DWMWA_USE_IMMERSIVE_DARK_MODE，Win10 早期版本是 19
    const DWORD DWMWA_USE_IMMERSIVE_DARK_MODE_OLD = 19;
    const DWORD DWMWA_USE_IMMERSIVE_DARK_MODE_NEW = 20;
    if (FAILED(DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_NEW, &dark, sizeof(dark))))
    {
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_OLD, &dark, sizeof(dark));
    }
}

// 检测系统当前是否为深色模式（读注册表）
bool IsSystemDarkMode()
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return false;

    DWORD value = 1, size = sizeof(value), type = REG_DWORD;
    RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, &type, (LPBYTE)&value, &size);
    RegCloseKey(hKey);
    return value == 0; // 0 = 深色
}

// 手动设置窗口和控件的深色配色
void ApplyDarkModeToControls(HWND hwnd)
{
    if (!g_darkMode) return;
    // 这里只做一个最简示例：设置窗口背景为深色
    // 实际每个控件的颜色要在 WM_CTLCOLORxxx 里处理
}

// ==================== 全局字体 ====================
HFONT g_font = nullptr;

HFONT GetUIFont()
{
    if (!g_font)
    {
        g_font = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    }
    return g_font;
}

// ==================== 创建控件辅助 ====================
HWND CreateLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id = -1)
{
    HWND h = CreateWindowExW(0, L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, nullptr, nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)GetUIFont(), TRUE);
    return h;
}

HWND CreateCombo(HWND parent, int x, int y, int w, int h, int id,
    const std::vector<std::wstring>& items)
{
    HWND h = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, nullptr, nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)GetUIFont(), TRUE);
    for (auto& s : items)
        SendMessageW(h, CB_ADDSTRING, 0, (LPARAM)s.c_str());
    SendMessageW(h, CB_SETCURSEL, 0, 0);
    return h;
}

HWND CreateCheck(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id, bool checked = false)
{
    HWND h = CreateWindowExW(0, L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, nullptr, nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)GetUIFont(), TRUE);
    if (checked) SendMessageW(h, BM_SETCHECK, BST_CHECKED, 0);
    return h;
}

HWND CreateButton(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id)
{
    HWND h = CreateWindowExW(0, L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, nullptr, nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)GetUIFont(), TRUE);
    return h;
}

HWND CreateEdit(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id,
    DWORD extraStyle = 0)
{
    HWND h = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", text,
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | extraStyle,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, nullptr, nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)GetUIFont(), TRUE);
    return h;
}

HWND CreateGroupBox(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id)
{
    HWND h = CreateWindowExW(0, L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, nullptr, nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)GetUIFont(), TRUE);
    return h;
}

// ==================== 控件创建 ====================
void CreateControls(HWND hwnd)
{
    const int m = 12;
    const int labelX = m;
    const int comboX = 100;
    const int comboW = 180;
    int y = 20;
    const int gap = 24;

    // ---- 左侧 ----
    CreateLabel(hwnd, L"压缩包(A):", labelX, y + 3, 80, 18);
    CreateEdit(hwnd, L"D:\\Desktop\\repo.7z", comboX, y, comboW + 60, 22, IDC_ARCHIVE);
    CreateButton(hwnd, L"...", comboX + comboW + 65, y - 1, 32, 24, IDC_BROWSE);
    y += gap + 8;

    CreateLabel(hwnd, L"压缩格式(F):", labelX, y + 3, 80, 18);
    CreateCombo(hwnd, comboX, y, comboW, 200, IDC_FORMAT,
        { L"7z", L"Zip", L"Tar", L"GZip", L"BZip2", L"xz" });
    y += gap;

    CreateLabel(hwnd, L"压缩等级(L):", labelX, y + 3, 80, 18);
    CreateCombo(hwnd, comboX, y, comboW, 200, IDC_LEVEL,
        { L"0 - 仅存储", L"1 - 最快", L"3 - 快速", L"5 - 标准", L"7 - 最大", L"9 - 极限压缩" });
    y += gap;

    CreateLabel(hwnd, L"压缩方法(M):", labelX, y + 3, 80, 18);
    CreateCombo(hwnd, comboX, y, comboW, 200, IDC_METHOD,
        { L"* LZMA2", L"LZMA", L"PPMd", L"BZip2", L"Copy" });
    y += gap;

    CreateLabel(hwnd, L"字典大小(D):", labelX, y + 3, 80, 18);
    CreateCombo(hwnd, comboX, y, comboW, 200, IDC_DICTIONARY,
        { L"* 256 MB", L"64 MB", L"128 MB", L"512 MB", L"1 GB", L"2 GB" });
    y += gap;

    CreateLabel(hwnd, L"单词大小(W):", labelX, y + 3, 80, 18);
    CreateCombo(hwnd, comboX, y, comboW, 200, IDC_ORDER,
        { L"273", L"128", L"64", L"32", L"16" });
    y += gap;

    CreateLabel(hwnd, L"固实数据大小:", labelX, y + 3, 80, 18);
    CreateCombo(hwnd, comboX, y, comboW, 200, IDC_SOLID,
        { L"* 16 GB", L"4 GB", L"8 GB", L"32 GB", L"64 GB", L"Solid" });
    y += gap;

    CreateLabel(hwnd, L"CPU 线程数:", labelX, y + 3, 80, 18);
    CreateCombo(hwnd, comboX, y, comboW - 40, 200, IDC_THREADS,
        { L"* 8", L"1", L"2", L"4", L"16", L"20" });
    CreateLabel(hwnd, L"/ 8", comboX + comboW - 35, y + 3, 40, 18, IDC_THREADS_MAX);
    y += gap;

    CreateLabel(hwnd, L"压缩所需内存:", labelX, y + 3, 110, 18, IDC_MEM_LABEL);
    CreateCombo(hwnd, comboX + 130, y, 100, 200, IDC_MEMUSE,
        { L"* 80%", L"10%", L"20%", L"50%", L"100%" });
    CreateLabel(hwnd, L"11581 MB / 12947 MB / 16184 MB", labelX, y + 16, 220, 18, IDC_MEM_VALUE);
    y += gap + 8;

    CreateLabel(hwnd, L"解压所需内存:", labelX, y + 3, 110, 18, IDC_MEM_DE_LABEL);
    CreateLabel(hwnd, L"258 MB", comboX + comboW - 40, y + 3, 100, 18, IDC_MEM_DE_VALUE);
    y += gap + 8;

    CreateLabel(hwnd, L"分卷大小，字节(V):", labelX, y + 3, 150, 18);
    CreateCombo(hwnd, labelX, y + 16, comboW + 160, 200, IDC_SPLIT, { L"（空）" });
    y += gap + 24;

    CreateLabel(hwnd, L"参数(P):", labelX, y + 3, 80, 18);
    CreateEdit(hwnd, L"", labelX, y + 16, comboW + 160, 22, IDC_PARAMS);
    y += gap + 24;

    CreateButton(hwnd, L"选项", labelX, y, 88, 26, IDC_OPTIONS_BTN);

    // ---- 右侧 ----
    int rx = 310;
    int rComboX = 380;
    int rComboW = 180;
    int ry = 20;

    CreateLabel(hwnd, L"更新方式(U):", rx, ry + 3, 80, 18);
    CreateCombo(hwnd, rComboX, ry, rComboW, 200, IDC_UPDATE_MODE,
        { L"添加并替换文件", L"更新并添加文件", L"仅更新现有文件", L"同步文件" });
    ry += gap;

    CreateLabel(hwnd, L"路径模式:", rx, ry + 3, 80, 18);
    CreateCombo(hwnd, rComboX, ry, rComboW, 200, IDC_PATH_MODE,
        { L"相对路径", L"完整路径", L"绝对路径" });
    ry += gap + 20;

    // 选项分组
    CreateGroupBox(hwnd, L"选项", rx, ry, rComboW + 80, 100, -1);
    CreateCheck(hwnd, L"创建自解压程序(X)", rx + 15, ry + 25, 200, 20, IDC_SFX);
    CreateCheck(hwnd, L"压缩共享文件", rx + 15, ry + 48, 200, 20, IDC_SHARED);
    CreateCheck(hwnd, L"操作完成后删除源文件", rx + 15, ry + 71, 200, 20, IDC_DELETE);
    ry += 115;

    // 加密分组
    CreateGroupBox(hwnd, L"加密", rx, ry, rComboW + 80, 200, -1);
    CreateLabel(hwnd, L"输入密码:", rx + 15, ry + 25, 100, 18);
    CreateEdit(hwnd, L"", rx + 15, ry + 45, rComboW + 50, 22, IDC_PASSWORD, ES_PASSWORD);
    CreateCheck(hwnd, L"显示密码(S)", rx + 15, ry + 75, 150, 20, IDC_SHOW_PWD, true);
    CreateLabel(hwnd, L"加密算法:", rx + 15, ry + 105, 100, 18);
    CreateCombo(hwnd, rx + 100, ry + 102, 160, 200, IDC_ENC_METHOD,
        { L"AES-256", L"ZipCrypto" });
    CreateCheck(hwnd, L"加密文件名(N)", rx + 15, ry + 135, 150, 20, IDC_ENC_NAMES);

    // ---- 底部按钮 ----
    CreateButton(hwnd, L"确定", 300, 590, 88, 28, IDC_OK);
    CreateButton(hwnd, L"取消", 400, 590, 88, 28, IDC_CANCEL);
    CreateButton(hwnd, L"帮助", 500, 590, 88, 28, IDC_HELP);
}

// ==================== 处理复选框：显示密码 ====================
void OnShowPasswordChanged(HWND hwnd)
{
    HWND hPwd = GetDlgItem(hwnd, IDC_PASSWORD);
    HWND hChk = GetDlgItem(hwnd, IDC_SHOW_PWD);
    bool checked = SendMessageW(hChk, BM_GETCHECK, 0, 0) == BST_CHECKED;

    // 切换 ES_PASSWORD 样式
    LONG style = GetWindowLongW(hPwd, GWL_STYLE);
    if (checked)
        style &= ~ES_PASSWORD;
    else
        style |= ES_PASSWORD;
    SetWindowLongW(hPwd, GWL_STYLE, style);

    // 强制重绘
    InvalidateRect(hPwd, nullptr, TRUE);
}

// ==================== 窗口过程 ====================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        CreateControls(hwnd);
        if (g_darkMode)
        {
            EnableDarkTitleBar(hwnd);
        }
        return 0;

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        if (id == IDC_SHOW_PWD && code == BN_CLICKED)
        {
            OnShowPasswordChanged(hwnd);
            return 0;
        }
        if (id == IDC_OK || id == IDC_CANCEL || id == IDC_HELP)
        {
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORLISTBOX:
        if (g_darkMode)
        {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(240, 240, 240));
            SetBkColor(hdc, RGB(32, 32, 32));
            static HBRUSH darkBrush = CreateSolidBrush(RGB(32, 32, 32));
            return (LRESULT)darkBrush;
        }
        break;

    case WM_ERASEBKGND:
        if (g_darkMode)
        {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH brush = CreateSolidBrush(RGB(32, 32, 32));
            FillRect(hdc, &rc, brush);
            DeleteObject(brush);
            return 1;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ==================== 入口 ====================
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    // 初始化通用控件
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    // 检测系统深色模式
    g_darkMode = IsSystemDarkMode();

    // 注册窗口类
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = g_darkMode ? CreateSolidBrush(RGB(32, 32, 32)) : (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"CompressDialogWnd";
    RegisterClassExW(&wc);

    // 创建窗口（固定大小，不可伸缩）
    HWND hwnd = CreateWindowExW(
        0, L"CompressDialogWnd", L"添加到压缩包（Win32 样板）",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 620, 680,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_font) DeleteObject(g_font);
    return (int)msg.wParam;
}