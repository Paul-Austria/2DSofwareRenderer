#include "Color.h"
#include "PixelFormat/PixelFormatInfo.h"
#include "PixelFormat/PixelConverter.h"

using namespace Renderer2D;

Color::Color(uint8_t r, uint8_t g, uint8_t b)
{
    data[0] = r;
    data[1] = g;
    data[2] = b;

    format = PixelFormat::RGB24;
    
}

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    data[0] = r;
    data[1] = g;
    data[2] = b;
    data[3] = a;
    format = PixelFormat::RGBA32;
}

Color::Color(uint8_t grayscale)
{
    data[0] = grayscale;
    data[1] = grayscale;
    data[2] = grayscale;
    data[3] = grayscale;
    format = PixelFormat::GRAYSCALE8;

}

void Color::GetColor(PixelFormat targetFormat, uint8_t *outColor) const
{
    if (format == targetFormat) {
        std::memcpy(outColor, data, PixelFormatRegistry::GetInfo(format).bytesPerPixel);
    } else {
        // Convert the internal data to the target format
        ConvertTo(targetFormat, outColor);
    }
}

void Color::SetColor(PixelFormat inputFormat, const uint8_t *colorData)
{
    // Set the format to the input format
    format = inputFormat;
    
    // Get the bytes per pixel for the input format
    int bytes = PixelFormatRegistry::GetInfo(format).bytesPerPixel;

    // Copy the input color data into internal storage
    std::memcpy(data, colorData, bytes);
}

void Color::ConvertTo(PixelFormat targetFormat, uint8_t *outColor) const
{    // If the current format is the same as the target, just copy
    if (format == targetFormat) {
        std::memcpy(outColor, data, PixelFormatRegistry::GetInfo(format).bytesPerPixel);
        return;
    }

    // Use the PixelConverter to perform the conversion
    PixelConverter converter;

    // Register default conversions and perform the conversion
    converter.Convert(format, targetFormat, data, outColor);
}
