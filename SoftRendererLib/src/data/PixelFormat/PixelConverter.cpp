#include "PixelConverter.h"

namespace Renderer2D
{
    PixelConverter::ConvertFunc PixelConverter::GetConversionFunction(PixelFormat from, PixelFormat to) {
        for (const auto& conversion : defaultConversions) {
            if (conversion.from == from && conversion.to == to) {
                return conversion.func;
            }
        }
        throw std::runtime_error("Conversion function not found between these formats.");
    }

    void PixelConverter::Convert(PixelFormat from, PixelFormat to, const uint8_t* src, uint8_t* dst, size_t count) const {
        ConvertFunc func = GetConversionFunction(from, to);
        func(src, dst, count);
    }


} // namespace Renderer2D
