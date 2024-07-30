#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "mouse_record.h"
#include "mouse_replay.h"

class CMainWindow
{
public:
    CMainWindow();
    ~CMainWindow();
    bool Create(HINSTANCE hInstance, const wchar_t* pwzWindowName);
    int MessageLoop();
    HWND GetHwnd()const { return m_hWnd;}
private:
    const wchar_t* m_swzClassName = L"Mouse replay window";
    HINSTANCE m_hInstance = nullptr;
    HWND m_hWnd = nullptr;

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(HWND hWnd);
    LRESULT OnDestroy();
    LRESULT OnClose();
    LRESULT OnPaint();
    LRESULT OnSize();
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);

    enum Constants { kHotKeyId = 1, kFontSize = 14, kButtonWidth = 70 };
    enum Controls { kRecordButton = 1, kClearButton, kSaveButton, kReplayButton };
    enum class Mode { Idle, Recording, Replaying };

    const std::vector<std::wstring> m_columnNames = { L"Index", L"X-pos.", L"Y-pos." };

    HFONT m_hFont = nullptr;
    HWND m_hListView = nullptr;
    HWND m_hListBox = nullptr;
    HWND m_hRecordButton = nullptr;
    HWND m_hClearButton = nullptr;
    HWND m_hSaveButton = nullptr;
    HWND m_hReplayButton = nullptr;

    static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

    bool m_bHotKeyRegistered = false;
    Mode m_mode = Mode::Idle;
    void SwitchButton();

    void OnRecordButton();
    void OnClearButton();
    void OnSaveButton();
    void OnReplayButton();

    CMouseRecord* m_pMouseRecorder = nullptr;
    CMouseReplay* m_pMouseReplayer = nullptr;

    void InsertCoordinateToListView(WPARAM wParam, LPARAM lParam);
    void AddMessageToListBox(const char* message);
};

#endif //MAIN_WINDOW_H_