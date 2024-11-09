#include "MemHandler.h"
#include <memory>

using namespace Renderer2D;

void MemHandler::MemCopy(void *_Dst, const void *_Src, size_t _Size)
{
    std::memcpy(_Dst, _Src, _Size);
}