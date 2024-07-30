#ifndef MOUSE_REPLAY_H_
#define MOUSE_REPLAY_H_

#include <Windows.h>

#include <vector>

#include "input_base.h"

class CMouseReplay
{
public:
    CMouseReplay();
    ~CMouseReplay();
    bool StartReplay(const wchar_t* pwzFilePath);
    void EndReplay();
private:
    std::vector<input_base::SMouseRecord> m_records;
    size_t m_nRecordIndex = 0;

    bool LoadRecordFile(const wchar_t* pwzFilePath);
    bool RestoreRecord(const char* pzRecordLines);
    void ClearRecord();

    void ResetMouseState();

    PTP_TIMER m_pTpTimer = nullptr;

    void StartThreadpoolTimer();
    void EndThreadpoolTimer();
    void UpdateTimerInterval(PTP_TIMER timer);
    void OnTide();
    static void CALLBACK TimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer);
};

#endif //MOUSE_REPLAY_H_
