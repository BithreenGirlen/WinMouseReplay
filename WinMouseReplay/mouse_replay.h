#ifndef MOUSE_REPLAY_H_
#define MOUSE_REPLAY_H_

#include <Windows.h>

#include <vector>

namespace replay_event
{
    enum
    {
        LeftClick,
        LeftDrag
    };
}

struct SMouseRecord
{
    POINT point{};
    long long nDelay = 900;
    unsigned int ulEvent = 0;
};

class CMouseReplay
{
public:
    CMouseReplay(HWND hWnd);
    ~CMouseReplay();
    bool StartReplay(const char* pzFileName);
    void EndReplay();
private:
    HANDLE m_hEvent = nullptr;
    HANDLE m_hThread = INVALID_HANDLE_VALUE;
    bool m_bThreadRunning = false;
    HWND m_hRetWnd = nullptr;

    std::vector<SMouseRecord> m_records;
    size_t m_nRecordIndex = 0;

    bool LoadRecordFile(const char* pzFileName);
    bool RestoreRecord(const char* pzRecordLines);
    void ClearRecord();

    static void ThreadLauncher(void* args);
    void ReplayingThread();

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
