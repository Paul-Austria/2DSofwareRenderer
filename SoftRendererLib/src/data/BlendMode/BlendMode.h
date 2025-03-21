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

    enum class BlendFactor
    {
        Zero,
        One,
        SourceAlpha,
        InverseSourceAlpha,
        DestAlpha,
        InverseDestAlpha,
        SourceColor,
        DestColor,
        InverseSourceColor,
        InverseDestColor,
    };

    enum class BlendOperation
    {
        Add,  // Simplified, most common blend operation
        Min, // not implemented
        Max  //not  implemented
    };


    struct BlendContext
    {
        BlendFactor colorBlendFactorSrc;
        BlendFactor colorBlendFactorDst;
        BlendOperation colorBlendOperation;

        BlendFactor alphaBlendFactorSrc;
        BlendFactor alphaBlendFactorDst;
        BlendOperation alphaBlendOperation;
    };





    struct Coloring
    {
        bool colorEnabled = false;
        Color color;
    };

}

#endif