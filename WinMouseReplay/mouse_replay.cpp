
#include <process.h>

#include "mouse_replay.h"

CMouseReplay::CMouseReplay(HWND hWnd)
	:m_hRetWnd(hWnd)
{

}

CMouseReplay::~CMouseReplay()
{
	EndReplay();
}
/*再現開始*/
bool CMouseReplay::StartReplay(const char* file_name)
{
	if (m_point.empty())
	{
		if (LoadRecordFile(file_name))
		{
			if (m_hEvent == nullptr)
			{
				m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				if (m_hEvent != nullptr)
				{
					if (m_hThread == INVALID_HANDLE_VALUE)
					{
						m_hThread = reinterpret_cast<HANDLE>(_beginthread(&ThreadLauncher, 0, this));
						if (m_hThread != INVALID_HANDLE_VALUE)
						{
							m_thread_running = true;
							return true;
						}
					}
				}
			}
			/*失敗*/
			EndReplay();
		}
	}

	return false;
}
/*再現終了*/
void CMouseReplay::EndReplay()
{
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		m_thread_running = false;
		if (m_hEvent != nullptr)
		{
			::SetEvent(m_hEvent);
			::WaitForSingleObject(m_hThread, INFINITE);
		}
	}
}
/*ファイル読み込み*/
bool CMouseReplay::LoadRecordFile(const char* file_name)
{
	bool bResult = false;

	HANDLE hFile = ::CreateFileA(file_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = ::GetFileSize(hFile, nullptr);
		if (dwSize != INVALID_FILE_SIZE)
		{
			char* buffer = static_cast<char*>(malloc(static_cast<size_t>(dwSize)));
			if (buffer != nullptr)
			{
				DWORD dwRead = 0;
				BOOL iRet = ::ReadFile(hFile, buffer, dwSize, &dwRead, nullptr);
				if (iRet)
				{
					bool bRet = RestoreRecord(buffer);
					if (bRet)
					{
						bResult = true;
					}
				}
				free(buffer);
			}
		}
		::CloseHandle(hFile);
	}
	return bResult;
}
/*記録復元*/
bool CMouseReplay::RestoreRecord(char* raw)
{
	char* p = nullptr;
	char* pp = raw;
	size_t len = 0;
	char buffer[24]{};

	int iCount = 0;
	for (;;++iCount)
	{
		POINT point{};
		long long delay = 0;

		p = strstr(pp, "X:");
		if (p == nullptr)break;
		p += 2;
		pp = strstr(p, ",");
		if (pp == nullptr)break;
		len = pp - p;
		if (len > sizeof(buffer))break;
		memcpy(buffer, p, len);
		*(buffer + len) = '\0';
		point.x = atol(buffer);

		p = strstr(pp, "Y:");
		if (p == nullptr)break;
		p += 2;
		pp = strstr(p, ",");
		if (pp == nullptr)break;
		len = pp - p;
		if (len > sizeof(buffer))break;
		memcpy(buffer, p, len);
		*(buffer + len) = '\0';
		point.y = atol(buffer);

		p = strstr(pp, "D:");
		if (p == nullptr)break;
		p += 2;
		pp = strstr(p, ";");
		if (pp == nullptr)break;
		len = pp - p;
		if (len > sizeof(buffer))break;
		memcpy(buffer, p, len);
		*(buffer + len) = '\0';
		delay = atoll(buffer);

		m_point.push_back(point);
		m_delay.push_back(delay);
	}

	return iCount > 0 ? true : false;
}

void CMouseReplay::ClearRecord()
{
	m_point.clear();
	m_delay.clear();
}

void CMouseReplay::ThreadLauncher(void* args)
{
	static_cast<CMouseReplay*>(args)->ReplayingThread();
}

void CMouseReplay::ReplayingThread()
{

	if (m_hRetWnd != nullptr)
	{
		::PostMessageW(m_hRetWnd, wm_mouse_replay::out::Start, 0, 0);
	}

	if (!m_point.empty() && !m_delay.empty())
	{
		PTP_TIMER timer = CreateThreadpoolTimer(TimerCallback, this, nullptr);
		if (timer != nullptr)
		{
			FILETIME FileDueTime{};
			ULARGE_INTEGER ulDueTime{};
			ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * m_delay.at(0)));
			FileDueTime.dwHighDateTime = ulDueTime.HighPart;
			FileDueTime.dwLowDateTime = ulDueTime.LowPart;
			SetThreadpoolTimer(timer, &FileDueTime, 0, 0);

			for (;;)
			{
				if(::WaitForSingleObject(m_hEvent, INFINITE) == WAIT_FAILED)break;
				if (!m_thread_running)break;
			}

			::SetThreadpoolTimer(timer, nullptr, 0, 0);
			::WaitForThreadpoolTimerCallbacks(timer, TRUE);
			::CloseThreadpoolTimer(timer);
			m_index = 0;
		}
	}

	if (m_hRetWnd != nullptr)
	{
		::PostMessageW(m_hRetWnd, wm_mouse_replay::out::End, 0, 0);
	}

	ClearRecord();
	if (m_hEvent != nullptr)
	{
		::CloseHandle(m_hEvent);
		m_hEvent = nullptr;
	}
	m_hThread = INVALID_HANDLE_VALUE;
}

void CMouseReplay::TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer)
{
	CMouseReplay* pThis = static_cast<CMouseReplay*>(Context);
	if (pThis != nullptr)
	{
		POINT P = pThis->m_point.at(pThis->m_index);
		SetCursorPos(P.x, P.y);
		INPUT input{};
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
		SendInput(1, &input, sizeof(input));

		++pThis->m_index;
		if (pThis->m_index >= pThis->m_point.size())pThis->m_index = 0;
		FILETIME FileDueTime{};
		ULARGE_INTEGER ulDueTime{};
		ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL *10 * 1000 * pThis->m_delay.at(pThis->m_index)));
		FileDueTime.dwHighDateTime = ulDueTime.HighPart;
		FileDueTime.dwLowDateTime = ulDueTime.LowPart;

		SetThreadpoolTimer(Timer, &FileDueTime, 0, 0);
	}
}
