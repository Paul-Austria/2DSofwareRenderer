#include "Texture.h"
#include "PixelFormat/PixelFormatInfo.h"
using namespace Renderer2D;

Texture::Texture(uint16_t width, uint16_t height, PixelFormat format) : width(width), height(height), format(format)
{
    storedLocally = true;
    uint8_t bytesPerPixel = PixelFormatRegistry::GetInfo(format).bytesPerPixel;
    data = new uint8_t[bytesPerPixel*width*height*bytesPerPixel];
}

Texture::Texture(uint16_t width, uint16_t height, uint8_t *data, PixelFormat format): width(width), height(height), format(format), data(data)
{
    storedLocally = false;
}

Texture::~Texture()
{
    if(storedLocally) delete[] data;
}

uint8_t *Texture::GetData()
{
    return data;
}

PixelFormat Texture::GetFormat()
{
    return format;
}

uint16_t Texture::GetWidth()
{
    return width;
}

uint16_t Texture::GetHeight()
{
    return height;
}
