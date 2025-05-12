#ifndef MEM_HANDLER_H
#define MEM_HANDLER_H

#include <memory>
#include <cstring>
#if ENABLE_ESP_SUPPORT
#include "esp_attr.h"
#endif

namespace Tergos2D
{
    class MemHandler
    {
    private:
    public:
        #if ENABLE_ESP_SUPPORT
            static IRAM_ATTR inline void MemCopy(void *_Dst, const void *_Src, size_t _Size)
            {
                memcpy(_Dst, _Src, _Size);
            }
        #else
            static inline void MemCopy(void *_Dst, const void *_Src, size_t _Size)
            {
                std::memcpy(_Dst, _Src, _Size);
            }
        #endif
    };

}

#endif // !MEM_HANDLER_H