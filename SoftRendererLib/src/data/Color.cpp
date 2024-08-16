#include "Color.h"



Color::Color(uint8_t r, uint8_t g, uint8_t b)
{
    data[0] = r;
    data[1] = g;
    data[2] = b;
    
}

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    data[0] = r;
    data[1] = g;
    data[2] = b;
    data[3] = a;
}

Color::Color(uint8_t grayscale)
{
    data[0] = grayscale;
    data[1] = grayscale;
    data[2] = grayscale;
    data[3] = grayscale;

}

void Color::GetColor(PixelFormat format, uint8_t *outColor) const
{
}

void Color::SetColor(PixelFormat format, const uint8_t *colorData)
{
}

void Color::ConvertTo(PixelFormat targetFormat, uint8_t *outColor) const
{
}
