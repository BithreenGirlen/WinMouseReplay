
#include "mouse_replay.h"
#include "win_filesystem.h"

CMouseReplay::CMouseReplay()
{

}

CMouseReplay::~CMouseReplay()
{
    EndReplay();
}
/*再現開始*/
bool CMouseReplay::StartReplay(const wchar_t* pwzFilePath)
{
    EndReplay();

    bool bRet = LoadRecordFile(pwzFilePath);
    if (bRet)
    {
        StartThreadpoolTimer();
        return true;
    }

    return false;
}
/*再現終了*/
void CMouseReplay::EndReplay()
{
    EndThreadpoolTimer();
    ClearRecord();
}
/*記録読み込み*/
bool CMouseReplay::LoadRecordFile(const wchar_t* pwzFilePath)
{
    std::string strFile = win_filesystem::LoadFileAsString(pwzFilePath);
    if (!strFile.empty())
    {
        return RestoreRecord(strFile.c_str());
    }
    return false;
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

        input_base::SMouseRecord s{};
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
/*記録消去*/
void CMouseReplay::ClearRecord()
{
    m_records.clear();
    m_nRecordIndex = 0;
}
/*押下状態解除*/
void CMouseReplay::ResetMouseState()
{
    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    ::SendInput(1, &input, sizeof(input));
}

void CMouseReplay::StartThreadpoolTimer()
{
    if (m_pTpTimer != nullptr)return;

    m_pTpTimer = ::CreateThreadpoolTimer(TimerCallback, this, nullptr);
    if (m_pTpTimer != nullptr)
    {
        UpdateTimerInterval(m_pTpTimer);
    }
}

void CMouseReplay::EndThreadpoolTimer()
{
    if (m_pTpTimer != nullptr)
    {
        ::SetThreadpoolTimer(m_pTpTimer, nullptr, 0, 0);
        ::WaitForThreadpoolTimerCallbacks(m_pTpTimer, TRUE);
        ::CloseThreadpoolTimer(m_pTpTimer);
        m_pTpTimer = nullptr;

        ResetMouseState();
    }
}

void CMouseReplay::UpdateTimerInterval(PTP_TIMER timer)
{
    if (timer != nullptr)
    {
        FILETIME sFileDueTime{};
        ULARGE_INTEGER ulDueTime{};
        ulDueTime.QuadPart = static_cast<ULONGLONG>(-(1LL * 10 * 1000 * m_records.at(m_nRecordIndex).nDelay));
        sFileDueTime.dwHighDateTime = ulDueTime.HighPart;
        sFileDueTime.dwLowDateTime = ulDueTime.LowPart;
        ::SetThreadpoolTimer(timer, &sFileDueTime, 0, 0);
    }
}

void CMouseReplay::OnTide()
{
    const input_base::SMouseRecord& s = m_records.at(m_nRecordIndex);
    ::SetCursorPos(s.point.x, s.point.y);

    INPUT input{};
    input.type = INPUT_MOUSE;

    if (s.ulEvent == input_base::replay_event::LeftClick)
    {
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP | MOUSEEVENTF_LEFTDOWN;
    }
    else if (s.ulEvent == input_base::replay_event::LeftDragStart)
    {
        /*
        * If no time is put between ::SetCursorPos() and ::SendInput(), dragging does not work stably.
        * But not sure on what parameters the dragging depends;
        * none of the following system parameters seems relevant.
        *
        * UINT uiDoubleClickTime = ::GetDoubleClickTime()
        * UINT dwHoverTime = 0;
        * ::SystemParametersInfoA(SPI_GETMOUSEHOVERTIME, 0, &dwHoverTime, 0);
        * DWORD dwSingleClickTime = 0;
        * ::SystemParametersInfoA(SPI_GETMOUSECLICKLOCKTIME, 0, &dwSingleClickTime, 0);
        *
        * And occasionally minimum interval of threadpool timer or that of ::Sleep() is not sufficient.
        * So here I set a bit longer ::Sleep() than the minimum interval.
        */
        ::Sleep(20);
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    }
    else if (s.ulEvent == input_base::replay_event::LeftDragEnd)
    {
        ::Sleep(20);
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    }

    ::SendInput(1, &input, sizeof(input));

    ++m_nRecordIndex;
    if (m_nRecordIndex > m_records.size() - 1)m_nRecordIndex = 0;
}

void CMouseReplay::TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer)
{
    CMouseReplay* pThis = static_cast<CMouseReplay*>(Context);
    if (pThis != nullptr)
    {
        pThis->OnTide();
        pThis->UpdateTimerInterval(Timer);
    }
}
