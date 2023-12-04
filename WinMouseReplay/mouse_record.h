#ifndef MOUSE_RECORD_H_
#define MOUSE_RECORD_H_

#include <Windows.h>

#include <vector>
#include <string>

class CMouseRecord
{
public:
	CMouseRecord(HWND hWnd);
	~CMouseRecord();
	bool StartRecord();
	bool SaveRecord(const char* file_name);
	void ClearRecord();
private:
	HANDLE m_hThread = INVALID_HANDLE_VALUE;
	bool m_thread_running = false;
	HWND m_hRetWnd = nullptr;

	std::vector<POINT> m_point;
	std::vector<long long> m_delay;

	void EndRecord();

	static void ThreadLauncher(void* args);
	void RecordingThread();

	bool CheckKey(short key_code);
	long long GetNowTime();
};

namespace wm_mouse_record
{
	namespace out
	{
		enum
		{
			Start = WM_USER + 100,
			End,
			Record,
		};
	}
}

#endif // MOUSE_RECORD_H_
