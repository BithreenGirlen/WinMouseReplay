
#include <Windows.h>
#include <CommCtrl.h>

#include "main_window.h"
#include "win_dialogue.h"
#include "win_text.h"
#include "common_control_utility.h"

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
        return OnCommand(wParam, lParam);
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
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
    m_hWnd = hWnd;

    common_control_utility::CreateListView(&m_hListView, m_hWnd, m_columnNames);
    common_control_utility::CreateListBox(&m_hListBox, m_hWnd);
    common_control_utility::CreateButton(&m_hRecordButton, L"Record", m_hWnd, reinterpret_cast<HMENU>(Controls::kRecordButton));
    common_control_utility::CreateButton(&m_hClearButton, L"Clear", m_hWnd, reinterpret_cast<HMENU>(Controls::kClearButton));
    common_control_utility::CreateButton(&m_hSaveButton, L"Save", m_hWnd, reinterpret_cast<HMENU>(Controls::kSaveButton));
    common_control_utility::CreateButton(&m_hReplayButton, L"Replay", m_hWnd, reinterpret_cast<HMENU>(Controls::kReplayButton));

    ::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));

    m_pMouseRecorder = new CMouseRecord(m_hWnd);
    m_pMouseReplayer = new CMouseReplay();

    BOOL iRet = ::RegisterHotKey(m_hWnd, Constants::kHotKeyId, MOD_SHIFT, VK_DELETE);
    if (iRet)
    {
        m_bHotKeyRegistered = true;
    }
    else
    {
        std::wstring wstrMessage = L"RegisterHotKey failed; code: " + std::to_wstring(::GetLastError());
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
    RECT rect;
    ::GetClientRect(m_hWnd, &rect);
    int clientWidth = rect.right - rect.left;
    int clientHeight = rect.bottom - rect.top;

    int spaceX = clientWidth / 96;
    int spaceY = clientHeight / 96;

    int x = spaceX;
    int y = clientHeight / 2 - spaceY;
    int w = clientWidth / 2 - spaceX;
    int h = clientHeight / 2;
    if (m_hListView != nullptr)
    {
        ::MoveWindow(m_hListView, x, y, w, h, TRUE);

        list_view_utility::AdjustListViewWidth(m_hListView, static_cast<int>(m_columnNames.size()), false);
    }
    x = clientWidth / 2 + spaceX;
    w = clientWidth / 2 - spaceX * 2;
    if (m_hListBox != nullptr)
    {
        ::MoveWindow(m_hListBox, x, y, w, h, TRUE);
    }
    x = spaceX;
    y = spaceY;
    w = Constants::kButtonWidth;
    h = Constants::kFontSize * 2;
    if (m_hRecordButton != nullptr)
    {
        ::MoveWindow(m_hRecordButton, x, y, w, h, TRUE);
    }
    y += spaceY + Constants::kFontSize * 2;
    if (m_hClearButton != nullptr)
    {
        ::MoveWindow(m_hClearButton, x, y, w, h, TRUE);
    }
    x = spaceX * 2 + Constants::kButtonWidth;
    y = spaceY;
    if (m_hSaveButton != nullptr)
    {
        ::MoveWindow(m_hSaveButton, x, y, w, h, TRUE);
    }
    y += spaceY + Constants::kFontSize * 2;
    if (m_hReplayButton != nullptr)
    {
        ::MoveWindow(m_hReplayButton, x, y, w, h, TRUE);
    }

    return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
    int wmId = LOWORD(wParam);
    int iControlWnd = LOWORD(lParam);
    if (iControlWnd == 0)
    {
        /*Menus*/
    }
    else
    {
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

            if (m_mode == Mode::Replaying)
            {
                AddMessageToListBox("Stopped replaying.");
                m_mode = Mode::Idle;
                SwitchButton();
            }
        }
    }
    return 0;
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
    list_view_utility::ClearListView(m_hListView);
    list_box_utility::ClearListBox(m_hListBox);

    if (m_pMouseRecorder != nullptr)
    {
        m_pMouseRecorder->ClearRecord();
    }
}
/*保存ボタン*/
void CMainWindow::OnSaveButton()
{
    if (m_pMouseRecorder != nullptr)
    {
        std::wstring wstrFileToSave = win_dialogue::SelectSaveFile(L"文書形式", L"*.txt;", L"1.txt", m_hWnd);
        if (!wstrFileToSave.empty())
        {
            bool bRet = m_pMouseRecorder->SaveRecord(wstrFileToSave.c_str());
            if (bRet)
            {
                AddMessageToListBox("Saved the record successfully");
                list_view_utility::ClearListView(m_hListView);
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
    if (m_pMouseReplayer != nullptr)
    {
        std::wstring wstrFileToOpen = win_dialogue::SelectOpenFile(L"文書形式", L"*.txt;", m_hWnd);
        if (!wstrFileToOpen.empty())
        {
            std::string str = win_text::NarrowANSI(wstrFileToOpen);
            AddMessageToListBox(std::string("Opens file: ").append(str).c_str());

            bool bRet = m_pMouseReplayer->StartReplay(wstrFileToOpen.c_str());
            if (bRet)
            {
                m_mode = Mode::Replaying;
                SwitchButton();
                AddMessageToListBox("Started replaying; SHIFT + DELETE: stop.");
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
        int iIndex = static_cast<int>(wParam);
        POINT* pPoint = reinterpret_cast<POINT*>(lParam);
        if (pPoint != nullptr)
        {
            std::vector<std::wstring> row;
            row.emplace_back(std::to_wstring(iIndex));
            row.emplace_back(std::to_wstring(pPoint->x));
            row.emplace_back(std::to_wstring(pPoint->y));
            list_view_utility::AddItemToListView(m_hListView, row);
        }
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
        list_box_utility::AddListBoxText(m_hListBox, std::string(stamp).append(message).c_str());
    }
}
