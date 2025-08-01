#include "PixelFormatInfo.h"

namespace Tergos2D
{
    static const PixelFormatInfo formatInfoArray[] = {
     {PixelFormat::RGB24, 3, 24, false, 3, false,  0xFF, 0, 0xFF, 8, 0xFF, 16},
       {PixelFormat::BGR24, 3, 24, false, 3, false,  0xFF, 16, 0xFF, 8, 0xFF, 0},
        {PixelFormat::ARGB8888, 4, 32, false, 4, true,0xFF, 16, 0xFF, 8, 0xFF, 0, 0xFF, 0},
        {PixelFormat::RGBA8888, 4, 32, false, 4, true,  0xFF, 24, 0xFF, 16, 0xFF, 8, 0xFF, 24},
        {PixelFormat::ARGB1555, 2, 16, false, 4, true,  0x7C00, 10, 0x03E0, 5, 0x001F, 0, 0x8000, 15},
        {PixelFormat::RGB565, 2, 16, false, 3, false, 0xF800, 11, 0x07E0, 5, 0x001F, 0},
        {PixelFormat::RGBA4444, 2, 16, false, 4, true,  0xF000, 12, 0x0F00, 8, 0x00F0, 4, 0x000F, 0},
        {PixelFormat::GRAYSCALE8, 1, 8, false, 1, true,  0xFF, 0, 0x00, 0, 0x00, 0,0x00,0},
    };

    const PixelFormatInfo PixelFormatRegistry::GetInfo(PixelFormat format)
    {
        int cast = static_cast<int>(format);
        int max = static_cast<int>(PixelFormat::COUNT);
        if (cast >= 0 && cast < max)
        {
              PixelFormatInfo arrRet = formatInfoArray[cast];
              return arrRet;

        }
        static const PixelFormatInfo invalid{};
        return invalid;
    }
}
