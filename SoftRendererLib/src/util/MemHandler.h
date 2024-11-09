#ifndef MEM_HANDLER_H
#define MEM_HANDLER_H

#include <memory>
namespace Renderer2D
{
    class MemHandler
    {
    private:
    public:
        static void MemCopy(void *_Dst, const void *_Src, size_t _Size);
    };

}



#endif // !MEM_HANDLER_H