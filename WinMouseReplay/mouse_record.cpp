
#include <process.h>

#include "mouse_record.h"

CMouseRecord::CMouseRecord(HWND hWnd)
    :m_hRetWnd(hWnd)
{

}

CMouseRecord::~CMouseRecord()
{
    EndRecording();
}

bool CMouseRecord::StartRecording()
{
    if (m_hThread == INVALID_HANDLE_VALUE)
    {
        m_hThread = reinterpret_cast<HANDLE>(_beginthread(&ThreadLauncher, 0, this));
        if (m_hThread != INVALID_HANDLE_VALUE)
        {
            m_bThreadRunning = true;
            return true;
        }
    }
    /*失敗*/
    EndRecording();

    return false;
}

/*記録保存*/
bool CMouseRecord::SaveRecord(const char* pzFileName)
{
    if (pzFileName != nullptr)
    {
        HANDLE hFile = ::CreateFileA(pzFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            ::SetFilePointer(hFile, NULL, nullptr, FILE_END);
            for (size_t i = 0; i < m_points.size(); ++i)
            {
                char sBuffer[256]{};
                sprintf_s(sBuffer, "X:%ld, Y:%ld, D:%lld, E:%u;\r\n", m_points.at(i).x, m_points.at(i).y, m_nDelay.at(i), 0);
                DWORD bytesWrite = 0;
                BOOL bRet = ::WriteFile(hFile, sBuffer, static_cast<DWORD>(strlen(sBuffer)), &bytesWrite, nullptr);
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

/*記録消去*/
void CMouseRecord::ClearRecord()
{
    m_points.clear();
    m_nDelay.clear();
}

void CMouseRecord::EndRecording()
{
    m_bThreadRunning = false;
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
        if (!m_bThreadRunning)break;
        bool bRet = CheckKey(VK_INSERT);
        if (bRet)
        {
            POINT P {};
            ::GetCursorPos(&P);
            m_points.push_back(P);

            long long now = GetNowTime();
            m_nDelay.push_back(now - last);
            last = now;
            if (m_hRetWnd != nullptr)
            {
                ::PostMessageW(m_hRetWnd, wm_mouse_record::out::Record, m_points.size(), reinterpret_cast<LPARAM>(&m_points.back()));
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
    return ::GetAsyncKeyState(key_code) & 0x01;
}

long long CMouseRecord::GetNowTime()
{
    LARGE_INTEGER freq, ticks;
    ::QueryPerformanceFrequency(&freq);
    ::QueryPerformanceCounter(&ticks);
    double tm = 1000LL / static_cast<double>(freq.QuadPart);
    return static_cast<long long>(ticks.QuadPart * tm);
}
