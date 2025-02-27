#ifndef BLENDMODE
#define BLENDMODE

#include "../Color.h"
#include "../PixelFormat/PixelFormat.h"
#include <algorithm>
#include "../PixelFormat/PixelFormatInfo.h"

namespace Tergos2D
{
    enum class BlendMode
    {
        NOBLEND,
        COLORINGONLY,
        SIMPLE,
        MULTIPLY

    };

    enum class SelectedBlendMode
    {
        NOBLEND,
        COLORINGONLY,
        SIMPLE,
        MULTIPLY

    };

    struct Coloring
    {
        bool colorEnabled = false;
        Color color;
    };

}

#endif