
#include <process.h>

#include "mouse_record.h"
#include "win_filesystem.h"

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
    EndRecording();

    const auto ThreadLauncher = [](void* args)
        -> void
        {
            static_cast<CMouseRecord*>(args)->RecordingThread();
        };

    m_hThread = reinterpret_cast<HANDLE>(_beginthread(ThreadLauncher, 0, this));
    return m_hThread != INVALID_HANDLE_VALUE;
}

/*記録保存*/
bool CMouseRecord::SaveRecord(const wchar_t* pwzFilePath)
{
    if (pwzFilePath != nullptr && !m_records.empty())
    {
        std::string strFile;
        strFile.reserve(m_records.size() * 64);
        for (const auto& record : m_records)
        {
            char sBuffer[256]{};
            int iLen = sprintf_s(sBuffer, sizeof(sBuffer), "X:%ld, Y:%ld, D:%lld, E:%u;\r\n", record.point.x, record.point.y, record.nDelay, record.ulEvent);
            if (iLen > 0)
            {
                strFile += sBuffer;
            }
        }

        if (!strFile.empty())
        {
            bool bRet =  win_filesystem::SaveStringToFile(pwzFilePath, strFile.data(), static_cast<DWORD>(strFile.size()));
            if (bRet)
            {
                ClearRecord();
                return true;
            }
        }
    }

    return false;
}

/*記録消去*/
void CMouseRecord::ClearRecord()
{
    m_records.clear();
}

void CMouseRecord::EndRecording()
{
    m_bThreadRunning = false;
    if (m_hThread != INVALID_HANDLE_VALUE)
    {
        ::WaitForSingleObject(m_hThread, INFINITE);
    }
}

void CMouseRecord::RecordingThread()
{
    m_bThreadRunning = true;

    if (m_hRetWnd != nullptr)
    {
        ::PostMessageW(m_hRetWnd, wm_mouse_record::out::Start, 0, 0);
    }

    long long last = GetNowTime();

    bool bToBeDragged = false;
    for (;;)
    {
        if (!m_bThreadRunning)break;
        bool bRet = CheckKey(VK_INSERT);
        if (bRet)
        {
            input_base::SMouseRecord record;
            ::GetCursorPos(&record.point);

            long long now = GetNowTime();
            record.nDelay = now - last;

            if (::GetKeyState(VK_RSHIFT) & 0x8000)
            {
                bToBeDragged ^= true;
                record.ulEvent = bToBeDragged ? input_base::replay_event::LeftDragStart : input_base::replay_event::LeftDragEnd;
            }
            else
            {
                record.ulEvent = input_base::replay_event::LeftClick;
            }

            m_records.push_back(record);
            last = now;
            if (m_hRetWnd != nullptr)
            {
                ::PostMessageW(m_hRetWnd, wm_mouse_record::out::Record, m_records.size(), reinterpret_cast<LPARAM>(&m_records.back().point));
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
