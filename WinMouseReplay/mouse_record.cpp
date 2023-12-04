
#include <process.h>

#include "mouse_record.h"

CMouseRecord::CMouseRecord(HWND hWnd)
	:m_hRetWnd(hWnd)
{

}

CMouseRecord::~CMouseRecord()
{
	EndRecord();
}

bool CMouseRecord::StartRecord()
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
	/*é∏îs*/
	EndRecord();

	return false;
}

/*ãLò^ï€ë∂*/
bool CMouseRecord::SaveRecord(const char* file_name)
{
	if (file_name != nullptr)
	{
		HANDLE hFile = ::CreateFileA(file_name, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::SetFilePointer(hFile, NULL, nullptr, FILE_END);
			for (size_t i = 0; i < m_point.size(); ++i)
			{
				char buffer[256]{};
				sprintf_s(buffer, "X:%ld, Y:%ld, D:%lld;\r\n", m_point.at(i).x, m_point.at(i).y, m_delay.at(i));
				DWORD bytesWrite = 0;
				BOOL bRet = ::WriteFile(hFile, buffer, static_cast<DWORD>(strlen(buffer)), &bytesWrite, nullptr);
				if (!bRet)
				{
					::CloseHandle(hFile);
					return false;
				}
			}
			::CloseHandle(hFile);
			ClearRecord();
			return true;
		}
	}
	return false;
}

/*ãLò^è¡ãé*/
void CMouseRecord::ClearRecord()
{
	m_point.clear();
	m_delay.clear();
}

void CMouseRecord::EndRecord()
{
	m_thread_running = false;
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		::WaitForSingleObject(m_hThread, INFINITE);
	}
}

void CMouseRecord::ThreadLauncher(void* args)
{
	static_cast<CMouseRecord*>(args)->RecordingThread();
}

void CMouseRecord::RecordingThread()
{

	if (m_hRetWnd != nullptr)
	{
		::PostMessageW(m_hRetWnd, wm_mouse_record::out::Start, 0, 0);
	}

	long long last = GetNowTime();

	for (;;)
	{
		if (!m_thread_running)break;
		bool bRet = CheckKey(VK_INSERT);
		if (bRet)
		{
			POINT P {};
			GetCursorPos(&P);
			m_point.push_back(P);

			long long now = GetNowTime();
			m_delay.push_back(now - last);
			last = now;
			if (m_hRetWnd != nullptr)
			{
				::PostMessageW(m_hRetWnd, wm_mouse_record::out::Record, m_point.size(), reinterpret_cast<LPARAM>(&m_point.back()));
			}
		}
		bRet = CheckKey(VK_DELETE);
		if (bRet)break;
	}

	if (m_hRetWnd != nullptr)
	{
		::PostMessageW(m_hRetWnd, wm_mouse_record::out::End, 0, 0);
	}

	m_hThread = INVALID_HANDLE_VALUE;
}

bool CMouseRecord::CheckKey(short key_code)
{
	return GetAsyncKeyState(key_code) & 0x01;
}

long long CMouseRecord::GetNowTime()
{
	LARGE_INTEGER freq, ticks;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&ticks);
	double tm = 1000LL / static_cast<double>(freq.QuadPart);
	return static_cast<long long>(ticks.QuadPart * tm);
}
