#ifndef INPUT_BASE_H_
#define INPUT_BASE_H_

#include <Windows.h>

namespace input_base
{
    namespace replay_event
    {
        enum
        {
            LeftClick,
            LeftDragStart,
            LeftDragEnd,
        };
    }

    struct SMouseRecord
    {
        POINT point{};
        long long nDelay = 900;
        unsigned int ulEvent = 0;
    };
}
#endif // !INPUT_BASE_H_

