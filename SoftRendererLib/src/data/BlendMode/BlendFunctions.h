#ifndef BLENDFUNCTIONS_H
#define BLENDFUNCTIONS_H

#include "../Color.h"
#include "../PixelFormat/PixelFormat.h"
#include <algorithm>
#include "../PixelFormat/PixelFormatInfo.h"


namespace Renderer2D
{
    class BlendFunctions
    {
    private:
    public:
        static void BlendSimpleSolidColor(const Color &srcColor, uint8_t *dstData, PixelFormat format, size_t pixelCount);

        static void BlendRow(uint8_t *dstRow, const uint8_t *srcRow, size_t rowLength, const PixelFormatInfo &targetInfo, const PixelFormatInfo &sourceInfo);
        
    };

} // namespace Renderer2D


#endif // !BLENDFUNCTIONS_H