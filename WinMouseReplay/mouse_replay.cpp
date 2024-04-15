
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
bool CMouseReplay::StartReplay(const char* pzFileName)
{
    if (m_records.empty())
    {
        if (LoadRecordFile(pzFileName))
        {
            if (m_hEvent == nullptr)
            {
                m_hEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
                if (m_hEvent != nullptr)
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
        m_bThreadRunning = false;
        if (m_hEvent != nullptr)
        {
            ::SetEvent(m_hEvent);
            ::WaitForSingleObject(m_hThread, INFINITE);
        }
    }
}
/*ファイル読み込み*/
bool CMouseReplay::LoadRecordFile(const char* pzFileName)
{
    bool bResult = false;

    HANDLE hFile = ::CreateFileA(pzFileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwSize = ::GetFileSize(hFile, nullptr);
        if (dwSize != INVALID_FILE_SIZE)
        {
            char* pBuffer = static_cast<char*>(malloc(static_cast<size_t>(dwSize)));
            if (pBuffer != nullptr)
            {
                DWORD dwRead = 0;
                BOOL iRet = ::ReadFile(hFile, pBuffer, dwSize, &dwRead, nullptr);
                if (iRet)
                {
                    bool bRet = RestoreRecord(pBuffer);
                    if (bRet)
                    {
                        bResult = true;
                    }
                }
                free(pBuffer);
            }
        }
        ::CloseHandle(hFile);
    }
    return bResult;
}
/*記録復元*/
bool CMouseReplay::RestoreRecord(const char* pzRecordLines)
{
    const auto ToLong = [](const char* src, const char* szKey)
        -> long
        {
            if (src == nullptr)return -1;

            const char* p = strstr(src, szKey);
            if (p == nullptr)return -1;

            p += strlen(szKey);
            size_t nLen = strspn(p, "0123456789");
            if (nLen == 0)return -1;

            char sBuffer[24]{};
            if (nLen > sizeof(sBuffer))return -1;

            memcpy(sBuffer, p, nLen);
            *(sBuffer + nLen) = '\0';
            
            return strtol(sBuffer, nullptr, 10);
        };

    const char* p = nullptr;
    const char* pp = pzRecordLines;
    for (;;)
    {
        p = strstr(pp, "\r\n");
        if (p == nullptr)break;

        SMouseRecord s{};
        long lValue = -1;

        std::vector<char> bufs;
        size_t nLen = p - pp;
        bufs.reserve(nLen);
        for (size_t i = 0; i < nLen; ++i)
        {
            bufs.push_back(*(pp + i));
        }

        lValue = ToLong(bufs.data(), "X:");
        if (lValue != -1)s.point.x = lValue;

        lValue = ToLong(bufs.data(), "Y:");
        if (lValue != -1)s.point.y = lValue;

        lValue = ToLong(bufs.data(), "D:");
        if (lValue != -1)s.nDelay = lValue;

        lValue = ToLong(bufs.data(), "E:");
        if (lValue != -1)s.ulEvent = lValue;

        m_records.push_back(s);
        pp = p + 2;
        if (pp == nullptr)break;
    }

    return !m_records.empty();
}

void CMouseReplay::ClearRecord()
{
    m_records.clear();
    m_nRecordIndex = 0;
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

    if (!m_records.empty())
    {
        PTP_TIMER pTimer = ::CreateThreadpoolTimer(TimerCallback, this, nullptr);
        if (pTimer != nullptr)
        {
            FILETIME sFileDueTime{};
            ULARGE_INTEGER ulDueTime{};
            ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * m_records.at(0).nDelay));
            sFileDueTime.dwHighDateTime = ulDueTime.HighPart;
            sFileDueTime.dwLowDateTime = ulDueTime.LowPart;
            ::SetThreadpoolTimer(pTimer, &sFileDueTime, 0, 0);

            for (;;)
            {
                if(::WaitForSingleObject(m_hEvent, INFINITE) == WAIT_FAILED)break;
                if (!m_bThreadRunning)break;
            }

            ::SetThreadpoolTimer(pTimer, nullptr, 0, 0);
            ::WaitForThreadpoolTimerCallbacks(pTimer, TRUE);
            ::CloseThreadpoolTimer(pTimer);

            INPUT input{};
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            ::SendInput(1, &input, sizeof(input));
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
        const SMouseRecord& s = pThis->m_records.at(pThis->m_nRecordIndex);
        ::SetCursorPos(s.point.x, s.point.y);

        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

        if (s.ulEvent != replay_event::LeftDrag)
        {
            input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
        }
        ::SendInput(1, &input, sizeof(input));

        ++pThis->m_nRecordIndex;
        if (pThis->m_nRecordIndex > pThis->m_records.size() - 1)pThis->m_nRecordIndex = 0;

        FILETIME sFileDueTime{};
        ULARGE_INTEGER ulDueTime{};
        ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL *10 * 1000 * pThis->m_records.at(pThis->m_nRecordIndex).nDelay));
        sFileDueTime.dwHighDateTime = ulDueTime.HighPart;
        sFileDueTime.dwLowDateTime = ulDueTime.LowPart;

        ::SetThreadpoolTimer(Timer, &sFileDueTime, 0, 0);
    }
}
