
#include <Windows.h>
#include <CommCtrl.h>

#include "main_window.h"
#include "win_dialogue.h"
#include "win_text.h"

CMainWindow::CMainWindow()
{
    m_hFont = ::CreateFont(Constants::kFontSize, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"DFKai-SB");
}

CMainWindow::~CMainWindow()
{
    DeleteObject(m_hFont);

    if (m_pMouseRecorder != nullptr)
    {
        delete m_pMouseRecorder;
        m_pMouseRecorder = nullptr;
    }

    if (m_pMouseReplayer != nullptr)
    {
        delete m_pMouseReplayer;
        m_pMouseReplayer = nullptr;
    }
}

bool CMainWindow::Create(HINSTANCE hInstance, const wchar_t* pwzWindowName)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    //wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_APP));
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
    //wcex.lpszMenuName = MAKEINTRESOURCE(IDI_ICON_APP);
    wcex.lpszClassName = m_swzClassName;
    //wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_APP));

    if (::RegisterClassExW(&wcex))
    {
        m_hInstance = hInstance;

        m_hWnd = ::CreateWindowW(m_swzClassName, pwzWindowName, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, nullptr, nullptr, hInstance, this);
        if (m_hWnd != nullptr)
        {
            return true;
        }
        else
        {
            std::wstring wstrMessage = L"CreateWindowExW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
        }
    }
    else
    {
        std::wstring wstrMessage = L"RegisterClassW failed; code: " + std::to_wstring(::GetLastError());
        ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    }

    return false;
}

int CMainWindow::MessageLoop()
{
    MSG msg;

    for (;;)
    {
        BOOL bRet = ::GetMessageW(&msg, 0, 0, 0);
        if (bRet > 0)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        else if (bRet == 0)
        {
            /*ループ終了*/
            return static_cast<int>(msg.wParam);
        }
        else
        {
            /*ループ異常*/
            std::wstring wstrMessage = L"GetMessageW failed; code: " + std::to_wstring(::GetLastError());
            ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
            return -1;
        }
    }
    return 0;
}
/*C CALLBACK*/
LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMainWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }

    pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pThis != nullptr)
    {
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return OnCreate(hWnd);
    case WM_DESTROY:
        return OnDestroy();
    case WM_PAINT:
        return OnPaint();
    case WM_SIZE:
        return OnSize();
    case WM_COMMAND:
        return OnCommand(wParam);
    case WM_HOTKEY:
        return OnHotKey(wParam, lParam);
    case wm_mouse_record::out::Start:
        AddMessageToListBox("Started recording; INSERT: get pos., DELETE: stop.");
        break;
    case wm_mouse_record::out::Record:
        InsertCoordinateToListView(wParam, lParam);
        break;
    case wm_mouse_record::out::End:
        AddMessageToListBox("Stopped recording.");
        m_mode = Mode::Idle;
        SwitchButton();
        break;
    case wm_mouse_replay::out::Start:
        AddMessageToListBox("Started replaying; SHIFT + DELETE: stop.");
        break;
    case wm_mouse_replay::out::End:
        AddMessageToListBox("Stopped replaying.");
        m_mode = Mode::Idle;
        SwitchButton();
        break;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    m_hListView = ::CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_ALIGNLEFT | WS_TABSTOP | LVS_SINGLESEL, 0, 0, 0, 0, m_hWnd, nullptr, m_hInstance, nullptr);
    if (m_hListView != nullptr)
    {
        ListView_SetExtendedListViewStyle(m_hListView, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        LVCOLUMNW lvColumn{};
        lvColumn.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;
        lvColumn.fmt = LVCFMT_LEFT;
        for (int i = 0; i < m_columnNames.size(); ++i)
        {
            lvColumn.iSubItem = i;
            lvColumn.pszText = const_cast<LPWSTR>(m_columnNames.at(i).data());
            ListView_InsertColumn(m_hListView, i, &lvColumn);
        }
    }

    m_hListBox = ::CreateWindowExW(0, WC_LISTBOX, L"Message", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_SORT | LBS_NOINTEGRALHEIGHT | WS_VSCROLL, 0, 0, 0, 0, m_hWnd, nullptr, m_hInstance, nullptr);

    m_hRecordButton = ::CreateWindowExW(0, WC_BUTTONW, L"Record", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, m_hWnd, reinterpret_cast<HMENU>(Controls::kRecordButton), m_hInstance, nullptr);

    m_hClearButton = ::CreateWindowExW(0, WC_BUTTONW, L"Clear", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, m_hWnd, reinterpret_cast<HMENU>(Controls::kClearButton), m_hInstance, nullptr);

    m_hSaveButton = ::CreateWindowExW(0, WC_BUTTONW, L"Save", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, m_hWnd, reinterpret_cast<HMENU>(Controls::kSaveButton), m_hInstance, nullptr);

    m_hReplayButton = ::CreateWindowExW(0, WC_BUTTONW, L"Replay", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, m_hWnd, reinterpret_cast<HMENU>(Controls::kReplayButton), m_hInstance, nullptr);

    ::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

    m_pMouseRecorder = new CMouseRecord(m_hWnd);
    m_pMouseReplayer = new CMouseReplay(m_hWnd);

    BOOL bRet = ::RegisterHotKey(m_hWnd, Constants::kHotKeyId, MOD_SHIFT, VK_DELETE);
    if (bRet)
    {
        m_bHotKeyRegistered = true;
    }
    else
    {
        std::wstring wstrMessage = L"RegisterHotKey failed; code: " + std::to_wstring(GetLastError());
        ::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
    }

    return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
    if (m_bHotKeyRegistered)
    {
        ::UnregisterHotKey(m_hWnd, Constants::kHotKeyId);
        m_bHotKeyRegistered = false;
    }

    ::PostQuitMessage(0);
    return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
    ::DestroyWindow(m_hWnd);
    ::UnregisterClassW(m_swzClassName, m_hInstance);

    return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = ::BeginPaint(m_hWnd, &ps);
    ::EndPaint(m_hWnd, &ps);
    return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize()
{
    ResizeListView();
    ResizeListBox();

    ResizeRecordButton();
    ResizeClearButton();
    ResizeSaveButton();
    ResizeReplayButton();

    return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam)
{
    int wmId = LOWORD(wParam);
    switch (wmId)
    {
    case Controls::kRecordButton:
        OnRecordButton();
        break;
    case Controls::kClearButton:
        OnClearButton();
        break;
    case Controls::kSaveButton:
        OnSaveButton();
        break;
    case Controls::kReplayButton:
        OnReplayButton();
        break;
    }
    return 0;
}
/*WM_HOTKEY*/
LRESULT CMainWindow::OnHotKey(WPARAM wParam, LPARAM lParam)
{
    if (wParam == Constants::kHotKeyId && lParam & MOD_SHIFT)
    {
        if (m_pMouseReplayer != nullptr)
        {
            m_pMouseReplayer->EndReplay();
        }
    }
    return 0;
}
/*自身の窓枠寸法取得*/
void CMainWindow::GetClientAreaSize(long& width, long& height)
{
    RECT rect;
    ::GetClientRect(m_hWnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
}
/*ListView 位置・大きさ調整*/
void CMainWindow::ResizeListView()
{
    if (m_hListView != nullptr)
    {
        long w, h;
        GetClientAreaSize(w, h);
        long x_space = w / 100;
        long y_space = h / 100;
        ::MoveWindow(m_hListView, x_space, h / 2 - y_space, w / 2 - x_space, h / 2, TRUE);

        RECT rect;
        ::GetWindowRect(m_hListView, &rect);
        LVCOLUMNW lvColumn{};
        lvColumn.mask = LVCF_WIDTH;
        lvColumn.cx = (rect.right - rect.left) / static_cast<int>(m_columnNames.size());
        for (int i = 0; i < m_columnNames.size(); ++i)
        {
            ListView_SetColumn(m_hListView, i, &lvColumn);
        }
    }
}
/*通知欄 位置・大きさ調整*/
void CMainWindow::ResizeListBox()
{
    if (m_hListBox != nullptr)
    {
        long w, h;
        GetClientAreaSize(w, h);
        long x_space = w / 100;
        long y_space = h / 100;
        ::MoveWindow(m_hListBox, w / 2 + x_space, h / 2 - y_space, w / 2 - x_space * 2, h / 2, TRUE);
    }
}
/*記録ボタン 位置・大きさ調整*/
void CMainWindow::ResizeRecordButton()
{
    if (m_hRecordButton != nullptr)
    {
        long w, h;
        GetClientAreaSize(w, h);
        long x_space = w / 100;
        long y_space = h / 100;
        ::MoveWindow(m_hRecordButton, x_space, y_space, Constants::kButtonWidth, Constants::kFontSize * 2, TRUE);
    }
}
void CMainWindow::ResizeClearButton()
{
    if (m_hClearButton != nullptr)
    {
        long w, h;
        GetClientAreaSize(w, h);
        long x_space = w / 100;
        long y_space = h / 100;
        ::MoveWindow(m_hClearButton, x_space, y_space * 2 + Constants::kFontSize * 2, Constants::kButtonWidth, Constants::kFontSize * 2, TRUE);
    }
}
void CMainWindow::ResizeSaveButton()
{
    if (m_hSaveButton != nullptr)
    {
        long w, h;
        GetClientAreaSize(w, h);
        long x_space = w / 100;
        long y_space = h / 100;
        ::MoveWindow(m_hSaveButton, x_space * 2 + Constants::kButtonWidth, y_space, Constants::kButtonWidth, Constants::kFontSize * 2, TRUE);
    }
}
void CMainWindow::ResizeReplayButton()
{
    if (m_hReplayButton != nullptr)
    {
        long w, h;
        GetClientAreaSize(w, h);
        long x_space = w / 100;
        long y_space = h / 100;
        ::MoveWindow(m_hReplayButton, x_space * 2 + Constants::kButtonWidth, y_space * 2 + +Constants::kFontSize * 2, Constants::kButtonWidth, Constants::kFontSize * 2, TRUE);
    }
}
/*EnumChildWindows CALLBACK*/
BOOL CMainWindow::SetFontCallback(HWND hWnd, LPARAM lParam)
{
    ::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
    /*TRUE: 続行, FALSE: 終了*/
    return TRUE;
}
/*ボタン有効・無効切り替え*/
void CMainWindow::SwitchButton()
{
    switch (m_mode)
    {
    case Mode::Idle:
        ::EnableWindow(m_hRecordButton, TRUE);
        ::EnableWindow(m_hSaveButton, TRUE);
        ::EnableWindow(m_hClearButton, TRUE);
        ::EnableWindow(m_hReplayButton, TRUE);
        break;
    case Mode::Recording:
    case Mode::Replaying:
        ::EnableWindow(m_hRecordButton, FALSE);
        ::EnableWindow(m_hSaveButton, FALSE);
        ::EnableWindow(m_hClearButton, FALSE);
        ::EnableWindow(m_hReplayButton, FALSE);
        break;
    }
}
/*記録ボタン*/
void CMainWindow::OnRecordButton()
{
    if (m_pMouseRecorder != nullptr)
    {
        bool bRet = m_pMouseRecorder->StartRecording();
        if (bRet)
        {
            m_mode = Mode::Recording;
            SwitchButton();
        }
    }
}
/*消去ボタン*/
void CMainWindow::OnClearButton()
{
    ClearListView();
    ClearListBox();
    if (m_pMouseRecorder != nullptr)
    {
        m_pMouseRecorder->ClearRecord();
    }
}
/*保存ボタン*/
void CMainWindow::OnSaveButton()
{
    std::wstring wstrFileToSave = win_dialogue::SelectSaveFile(L"文書形式", L"*.txt;*.bin;*.dat;", L"1.txt", m_hWnd);
    if (!wstrFileToSave.empty())
    {
        std::string str = win_text::NarrowUtf8(wstrFileToSave);
        if (m_pMouseRecorder != nullptr)
        {
            bool bRet = m_pMouseRecorder->SaveRecord(str.c_str());
            if (bRet)
            {
                AddMessageToListBox("Saved the record successfully");
                ClearListView();
            }
            else
            {
                AddMessageToListBox("Failed to save record.");
            }
        }

    }
}
/*再生ボタン*/
void CMainWindow::OnReplayButton()
{
    std::wstring wstrFileToOpen = win_dialogue::SelectOpenFile(L"文書形式", L"*.txt;*.bin;*.dat;", m_hWnd);
    if (!wstrFileToOpen.empty())
    {
        std::string str = win_text::NarrowUtf8(wstrFileToOpen);
        AddMessageToListBox(std::string("Opens file: ").append(str).c_str());
        if (m_pMouseReplayer != nullptr)
        {
            bool bRet = m_pMouseReplayer->StartReplay(str.c_str());
            if (bRet)
            {
                m_mode = Mode::Replaying;
                SwitchButton();
            }
            else
            {
                AddMessageToListBox("Failed to load record file.");
            }
        }
    }
}
/*記録座標挿入*/
void CMainWindow::InsertCoordinateToListView(WPARAM wParam, LPARAM lParam)
{
    if (m_hListView != nullptr)
    {
        int i = static_cast<int>(wParam);
        wchar_t buf[32]{};
        swprintf_s(buf, L"%d", i);
        LVITEM lvItem{};
        lvItem.mask = LVIF_TEXT | LVIF_PARAM;
        lvItem.iItem = i > 0 ? i - 1 : 0;
        lvItem.iSubItem = 0;
        lvItem.pszText = buf;
        int index = ListView_InsertItem(m_hListView, &lvItem);
        if (index != -1)
        {
            POINT* P = reinterpret_cast<POINT*>(lParam);
            if (P != nullptr)
            {
                swprintf_s(buf, L"%ld", P->x);
                ListView_SetItemText(m_hListView, index, 1, buf);
                swprintf_s(buf, L"%ld", P->y);
                ListView_SetItemText(m_hListView, index, 2, buf);
            }
        }
    }
}
/*リスト項目消去*/
void CMainWindow::ClearListView()
{
    if (m_hListView != nullptr)
    {
        ListView_DeleteAllItems(m_hListView);
    }
}
/*画面通知*/
void CMainWindow::AddMessageToListBox(const char* message)
{
    if (m_hListBox != nullptr)
    {
        char stamp[16];
        SYSTEMTIME tm;
        ::GetLocalTime(&tm);
        sprintf_s(stamp, "%02d:%02d:%02d:%03d ", tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
        ::SendMessageA(m_hListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(std::string(stamp).append(message).c_str()));
    }
}
/*通知削除*/
void CMainWindow::ClearListBox()
{
    if (m_hListBox != nullptr)
    {
        ::SendMessage(m_hListBox, LB_RESETCONTENT, 0, 0);
    }
}
