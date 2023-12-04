#pragma once

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
	bool Create(HINSTANCE hInstance);
	int MesaageLoop();
	HWND GetHwnd()const { return m_hWnd;}
private:
	std::wstring m_class_name = L"Main window";
	std::wstring m_window_name = L"My application";
	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnCommand(WPARAM wParam);
	LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);

	std::vector<std::wstring> m_column_name = { L"Index", L"X-position", L"Y-position"};
	enum Constants{ kHotKeyId = 1, kFontSize = 14, kButtonWidth = 70};
	enum Controls{record_button = 1, clear_button, save_button, replay_button};
	enum class Mode { Idle, Recording, Replaying };

	HFONT m_hFont = nullptr;
	HWND m_hListView = nullptr;
	HWND m_hListBox = nullptr;
	HWND m_hRecordButton = nullptr;
	HWND m_hClearButton = nullptr;
	HWND m_hSaveButton = nullptr;
	HWND m_hReplayButton = nullptr;

	void GetClientAreaSize(long& width, long& height);
	void ResizeListView();
	void ResizeListBox();
	void ResizeRecordButton();
	void ResizeClearButton();
	void ResizeSaveButton();
	void ResizeReplayButton();
	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	bool m_bHotKeyRegistered = false;
	Mode m_mode = Mode::Idle;
	void SwitchButton();

	void OnRecordButton();
	void OnClearButton();
	void OnSaveButton();
	void OnReplayButton();

	CMouseRecord* m_mouse_record = nullptr;
	CMouseReplay* m_mouse_replay = nullptr;

	void InsertCoordinateToListView(WPARAM wParam, LPARAM lParam);
	void ClearListView();
	void AddMessageToListBox(const char* message);
	void ClearListBox();

};
