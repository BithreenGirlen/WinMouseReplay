#ifndef MOUSE_REPLAY_H_
#define MOUSE_REPLAY_H_

#include <Windows.h>

#include <vector>

class CMouseReplay
{
public:
	CMouseReplay(HWND hWnd);
	~CMouseReplay();
	bool StartReplay(const char* file_name);
	void EndReplay();
private:
	HANDLE m_hEvent = nullptr;
	HANDLE m_hThread = INVALID_HANDLE_VALUE;
	bool m_thread_running = false;
	HWND m_hRetWnd = nullptr;

	std::vector<POINT> m_point;
	std::vector<long long> m_delay;

	bool LoadRecordFile(const char* file_name);
	bool RestoreRecord(char* raw);
	void ClearRecord();

	static void ThreadLauncher(void* args);
	void ReplayingThread();

	size_t m_index = 0;

	static void CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer);
};

namespace wm_mouse_replay
{
	namespace out
	{
		enum
		{
			Start = WM_USER + 200,
			End,
			Shift,
		};
	}
}

#endif //MOUSE_REPLAY_H_
