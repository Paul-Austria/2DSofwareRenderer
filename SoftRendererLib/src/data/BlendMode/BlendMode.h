#ifndef BLENDMODE
#define BLENDMODE

#include "../Color.h"
#include "../PixelFormat/PixelFormat.h"
#include <algorithm>
#include "../PixelFormat/PixelFormatInfo.h"

namespace Renderer2D
{
    enum class BlendMode
    {
        NOBLEND,
        BLEND,
    };

    enum class SelectedBlendMode
    {
        SIMPLE,
        MULTIPLY

    };

}

#endif